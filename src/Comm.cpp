
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

Comm::Comm(const AString a_LoginToken, const AString & a_LoginNick, BotWarzApp & a_App):
	m_App(a_App),
	m_LoginToken(a_LoginToken),
	m_LoginNick(a_LoginNick),
	m_ShouldShowComm(false),
	m_ShouldLogComm(false),
	m_CommLogFile(nullptr),
	m_Status(csConnecting)
{
}





bool Comm::init(bool a_ShouldLogComm, bool a_ShouldShowComm)
{
	// Open the comm log file, if requested:
	if (a_ShouldLogComm)
	{
		openCommLogFile();
	}
	m_ShouldShowComm = a_ShouldShowComm;

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
	// Nothing needed yet
	// TODO: wait for the update sender thread
}





void Comm::send(const AString & a_Data)
{
	// Log to file, if requested:
	if (m_ShouldLogComm)
	{
		fprintf(m_CommLogFile, "OUT: %s", a_Data.c_str());
		fflush(m_CommLogFile);
	}

	// Show on stdout, if requested:
	if (m_ShouldShowComm)
	{
		printf("OUT: %s", a_Data.c_str());
	}

	m_Link->Send(a_Data);
}





void Comm::send(const Json::Value & a_Data)
{
	Json::StreamWriterBuilder wr;
	wr.settings_["indentation"] = "";
	wr.settings_["commentStyle"] = "None";
	send(Json::writeString(wr, a_Data) + "\n");
}





void Comm::openCommLogFile(void)
{
	// Compose the log file name:
	time_t rawtime;
	time(&rawtime);
	struct tm * timeinfo;
	#ifdef _MSC_VER
		struct tm timeinforeal;
		timeinfo = &timeinforeal;
		localtime_s(timeinfo, &rawtime);
	#else
		timeinfo = localtime(&rawtime);
	#endif
	AString FileName = Printf("CommLogs/%02d-%02d-%02d-%02d-%02d-%02d.txt", 
		(timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1), timeinfo->tm_mday,
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec
	);

	// Create the folder for the logs, if not already present:
	#ifdef _WIN32
		CreateDirectoryA("CommLogs", nullptr);
	#else
		mkdir("CommLogs", S_IRWXU | S_IRWXG | S_IRWXO);
	#endif

	// Open the log file:
	#ifdef _MSC_VER
		fopen_s(&m_CommLogFile, FileName.c_str(), "w");
	#else
		m_CommLogFile = fopen(FileName.c_str(), "w");
	#endif
	m_ShouldLogComm = (m_CommLogFile != nullptr);
}




void Comm::onIncomingData(const AString & a_Data)
{
	// Log to file, if requested:
	if (m_ShouldLogComm)
	{
		fprintf(m_CommLogFile, "IN:  %s", a_Data.c_str());
		if (!a_Data.empty() && (a_Data.back() != '\n'))
		{
			fprintf(m_CommLogFile, "\n");
		}
		fflush(m_CommLogFile);
	}

	// Show on stdout, if requested:
	if (m_ShouldShowComm)
	{
		printf("IN:  %s", a_Data.c_str());
		if (!a_Data.empty() && (a_Data.back() != '\n'))
		{
			printf("\n");
		}
	}

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
		return;
	}
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
	random += m_LoginToken;
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
	root["login"]["nickname"] = m_LoginNick;
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




