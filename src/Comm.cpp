
// Comm.cpp

// Implements the Comm class that encapsulates the communication with the BotWarz server

#include "Globals.h"
#include "Comm.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "json/json.h"
#include "sha1.h"
#include "BotWarzApp.h"





////////////////////////////////////////////////////////////////////////////////
// Callbacks:

class Callbacks:
	public cNetwork::cConnectCallbacks,
	public cTCPLink::cCallbacks
{
public:
	Callbacks(Comm & a_Comm):
		m_Comm(a_Comm)
	{
	}

protected:
	Comm & m_Comm;

	virtual void OnLinkCreated(cTCPLinkPtr a_Link) override
	{
		// Save the link for later use:
		m_Comm.m_Link = a_Link;

		// Disable NAGLE:
		a_Link->EnableNoDelay();
	}

	virtual void OnReceivedData(const char * a_Data, size_t a_Length) override
	{
		m_Comm.onIncomingData(AString(a_Data, a_Length));
	}

	virtual void OnRemoteClosed(void) override
	{
		LOG("Server closed the connection, terminating.");
		m_Comm.abortConnection();
	}

	virtual void OnConnected(cTCPLink & a_Link) override
	{
		// Nothing needed, server talks first
		LOG("Connected to the server. Waiting for the handshake request.");
		m_Comm.m_Status = Comm::csConnected;
	}

	virtual void OnError(int a_ErrorCode, const AString & a_ErrorMsg) override
	{
		LOGERROR("Error while connecting to the BotWarz server: %d (%s)", a_ErrorCode, a_ErrorMsg.c_str());
		m_Comm.abortConnection();
	}
};





////////////////////////////////////////////////////////////////////////////////
// Comm:

Comm::Comm(BotWarzApp & a_App):
	m_App(a_App),
	m_Status(csConnecting),
	m_ShouldTerminate(false),
	m_LastSentCmdId(1),
	m_LastReceivedCmdId(1),
	m_CommandSenderThread(&Comm::commandSenderThread, this)
{
}





bool Comm::init(void)
{
	// Connect to the server:
	auto callbacks = std::make_shared<Callbacks>(*this);
	if (!cNetwork::Connect("botwarz.eset.com", 2000, callbacks, callbacks))
	{
		m_Status = csError;
		LOGERROR("Cannot connect to server");
		return false;
	}

	// Wait for the handshake to complete:
	m_evtHandshake.Wait();
	if (m_Status == csError)
	{
		LOGERROR("Server handshake failed");
		return false;
	}
	LOG("Server handshake completed. Waiting for a game to start.");

	return true;
}




void Comm::stop(void)
{
	// Close the connection to the server:
	auto Link = m_Link;
	if (Link != nullptr)
	{
		Link->Close();
		m_Link.reset();
	}

	// Wake up the update sender thread and wait for it to terminate:
	m_ShouldTerminate = true;
	m_evtGameStart.Set();
	m_evtCommandIdMatch.Set();
	m_CommandSenderThread.join();
}





void Comm::send(const AString & a_Data)
{
	// Log to file, if requested:
	m_App.commLog(false, a_Data);

	m_Link->Send(a_Data);
}





void Comm::send(const Json::Value & a_Data)
{
	Json::StreamWriterBuilder wr;
	wr.settings_["indentation"] = "";
	wr.settings_["commentStyle"] = "None";
	send(Json::writeString(wr, a_Data) + "\n");
}





void Comm::onIncomingData(const AString & a_Data)
{
	// Log to file / screen, if requested:
	m_App.commLog(true, a_Data);

	// Process the data, linewise:
	auto queuedEnd = m_QueuedData.size();
	m_QueuedData.append(a_Data);
	auto dataLen = m_QueuedData.size();
	size_t lineStart = 0;
	for (auto i = queuedEnd; i < dataLen; i++)
	{
		if (m_QueuedData[i] == '\n')
		{
			processLine(m_QueuedData.substr(lineStart, i - lineStart));
			lineStart = i + 1;
		}
	}  // for i - m_QueuedData[]
	if (lineStart > 0)
	{
		m_QueuedData.erase(0, lineStart);
	}
}





void Comm::processLine(const AString & a_Line)
{
	// Parse the line into Json:
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(a_Line, root, false))
	{
		LOGWARNING("%s: Cannot parse incoming Json: %s", __FUNCTION__, reader.getFormattedErrorMessages().c_str());
		return;
	}

	// Handle "status" replies:
	if (root.isMember("status"))
	{
		AString status = root["status"].asString();
		if (status == "socket_connected")
		{
			processSocketConnected(root);
		}
		else if (status == "login_ok")
		{
			processLoginOK(root);
		}
		else if (status == "login_failed")
		{
			processLoginFailed(root);
		}
		else
		{
			// Log an unknown message, but keep going
			LOGWARNING("%s: Received an unhandled \"status\" message: %s", __FUNCTION__, root["msg"].asCString());
		}
		return;
	}

	// Handle "game" replies:
	if (root.isMember("game"))
	{
		processGame(root);
		return;
	}

	// Handle "play" replies:
	if (root.isMember("play"))
	{
		processPlay(root);
		return;
	}

	// Handle "result" replies:
	if (root.isMember("result"))
	{
		processResult(root);
		return;
	}

	LOGWARNING("%s: Received an unknown message: %s", __FUNCTION__, a_Line.c_str());
}





void Comm::processSocketConnected(const Json::Value & a_Response)
{
	// Extract the random string (nonce):
	if (!a_Response.isMember("random"))
	{
		LOGERROR("%s: The server hasn't provided any random string.", __FUNCTION__);
		abortConnection();
		return;
	}
	AString random = a_Response["random"].asString();

	// Calculate the SHA1 checksum to send to the server:
	unsigned char shaChecksum[20];
	random += m_App.getLoginToken();
	sha1(reinterpret_cast<const unsigned char *>(random.data()), random.size(), shaChecksum);

	// Convert to lowercase hex:
	std::stringstream shaHash;
	shaHash << std::hex << std::setfill('0') << std::nouppercase;
	for (size_t i = 0; i < ARRAYCOUNT(shaChecksum); i++)
	{
		shaHash << std::setw(2) << static_cast<unsigned>(shaChecksum[i]);
	}

	// Send the login request:
	Json::Value root;
	AString hash = shaHash.str();
	ASSERT(hash.length() == 40);
	root["login"]["hash"] = hash;
	root["login"]["nickname"] = m_App.getLoginNick();
	m_Status = csWaitingForHandshake;
	send(root);
}





void Comm::processLoginOK(const Json::Value & a_Response)
{
	if (m_Status != csWaitingForHandshake)
	{
		LOGERROR("%s: login_ok status received while status not WaitingForHandshake (exp %d, got %d). Aborting",
			__FUNCTION__, csWaitingForHandshake, m_Status
		);
		abortConnection();
		return;
	}

	m_Status = csIdle;
	m_evtHandshake.Set();
}





void Comm::processLoginFailed(const Json::Value & a_Response)
{
	if (m_Status != csWaitingForHandshake)
	{
		LOGERROR("%s: login_failed status received while status not WaitingForHandshake (exp %d, got %d). Aborting",
			__FUNCTION__, csWaitingForHandshake, m_Status
		);
		abortConnection();
		return;
	}

	// Display the message presented by the server:
	AString msg;
	if (a_Response.isMember("msg"))
	{
		msg = a_Response["msg"].asString();
	}
	else
	{
		msg = "<No message given>";
	}
	LOGERROR("The server login failed: %s. Aborting.", msg.c_str());

	// Abort the connection:
	abortConnection();
}





void Comm::processGame(const Json::Value & a_Response)
{
	if (m_Status != csIdle)
	{
		LOGERROR("%s: game started while not expecting it (status %d). Aborting.", __FUNCTION__, m_Status);
		abortConnection();
		return;
	}

	m_Status = csGame;
	LOG("Starting game: %s against %s",
		a_Response["game"]["players"][0]["nickname"].asCString(),
		a_Response["game"]["players"][1]["nickname"].asCString()
	);
	m_App.startGame(a_Response["game"]);
	m_evtGameStart.Set();
}





void Comm::processPlay(const Json::Value & a_Response)
{
	if (m_Status != csGame)
	{
		LOGERROR("%s: Received a \"play\" response while not in a game (status %d). Aborting.",
			__FUNCTION__, m_Status
		);
		abortConnection();
		return;
	}

	m_App.updateBoard(a_Response["play"]);

	// If the received lastCmdId starts matching our cmdId, wake up the command sender:
	auto prevLastReceivedCmdId = m_LastReceivedCmdId;
	m_LastReceivedCmdId = a_Response["play"]["lastCmdId"].asInt();
	if ((m_LastReceivedCmdId == m_LastSentCmdId) && (m_LastReceivedCmdId != prevLastReceivedCmdId))
	{
		m_evtCommandIdMatch.Set();
	}
}





void Comm::processResult(const Json::Value & a_Response)
{
	if (m_Status != csGame)
	{
		LOGERROR("%s: Received a \"result\" response while not in a game (status %d). Aborting.",
			__FUNCTION__, m_Status
		);
		abortConnection();
		return;
	}
	
	LOG("Game finished. Winner: %s", a_Response["result"]["winner"]["nickname"].asCString());
	m_App.finishGame(a_Response["result"]);
	m_Status = csIdle;
}





void Comm::abortConnection(void)
{
	// Close the connection to server:
	auto Link = m_Link;
	if (Link != nullptr)
	{
		Link->Close();
		m_Link.reset();
	}

	// If we were waiting for a handshake, wake up the main thread:
	if (m_Status == csWaitingForHandshake)
	{
		m_Status = csError;
		m_evtHandshake.Set();
	}

	// Set the status to Error, so that the future operations fail:
	m_Status = csError;

	// Terminate the entire app:
	m_App.terminate();
}





void Comm::commandSenderThread(void)
{
	while (!m_ShouldTerminate)
	{
		// Wait for the game start:
		m_evtGameStart.Wait();

		while (m_Status == csGame)
		{
			// Send the commands:
			sendCommands();

			// Wait for the game update with the matching command ID:
			m_evtCommandIdMatch.Wait();

			// The command ID has just matched, wait for 200 msec before sending new commands:
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}  // while (csGame)
	}  // while (!m_ShouldTerminate)
}






void Comm::sendCommands(void)
{
	Json::Value cmds;
	cmds["cmdId"] = ++m_LastSentCmdId;
	cmds["bots"] = m_App.getBotCommands();
	send(cmds);
}




