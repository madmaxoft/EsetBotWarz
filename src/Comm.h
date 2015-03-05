
// Comm.h

// Declares the Comm class that encapsulates the communication with the BotWarz server





#pragma once

#include <thread>
#include "lib/Network/Network.h"
#include "lib/Network/Event.h"
#include "lib/Network/CriticalSection.h"





// fwd:
class BotWarzApp;
namespace Json
{
	class Value;
};





class Comm
{
public:
	Comm(BotWarzApp & a_App);

	/** Initializes the subsystem.
	If a_ShouldLogComm is true, all the communication with the server is logged into a file.
	If a_ShouldShowComm is true, all the communication with the server is output to stdout.
	Returns true if successful, logs the reason and returns false to indicate failure. */
	bool init(bool a_ShouldLogComm, bool a_ShouldShowComm);

	/** Called from the app to stop everything. */
	void stop(void);

	/** Sends the data to the server, logging it to file if requested. */
	void send(const AString & a_Data);

	/** Sends the json data to the server, logging it to file if requested. */
	void send(const Json::Value & a_Data);

	/** Outputs the message to the commlog, if logging is enabled.
	Prefixes each message with a timestamp since the logfile creation.
	A newline is not included in the output message, caller needs to provide one. */
	void commLog(const AString & a_Msg);

protected:
	friend class Callbacks;

	/** Various states that the comm interface can be in.
	This indicates the connection status as well as what's currently going on. */
	enum Status
	{
		csConnecting,           ///< Waiting for the socket to connect
		csConnected,            ///< Connected, waiting for the server to send the initial message
		csWaitingForHandshake,  ///< Waiting for the server to acknowledge the login info
		csIdle,                 ///< Successfully logged in, waiting for a game to start
		csGame,                 ///< A game is in progress
		csError,                ///< A non-recoverable error has occurred, the connection has been closed
	};

	/** The owner App object. */
	BotWarzApp & m_App;

	/** If true, all the communication with the server is sent to stdout. */
	bool m_ShouldShowComm;

	/** If true, all the communication with the server is logged into m_CommLogFile. */
	bool m_ShouldLogComm;

	/** File into which all the communication is logged, if m_ShouldLogComm is true.
	Protected against multithreaded writes by m_CSCommLog. */
	FILE * m_CommLogFile;

	/** Mutex protecting m_CommLogFile against multithreaded writes. */
	cCriticalSection m_CSCommLog;

	/** The timestamp of the commlogfile creation. Used to output relative time offsets in the commlog file */
	std::chrono::high_resolution_clock::time_point m_CommLogBeginTime;

	/** The TCP link to the server. */
	cTCPLinkPtr m_Link;

	/** Partial data that has been received from the server but not yet processed (incomplete line). */
	AString m_QueuedData;

	/** Synchronization between the network thread and the main thread waiting for handshake completion. */
	cEvent m_evtHandshake;

	/** The current status of the connection. */
	Status m_Status;

	/** Flag that tells everything that it should terminate as soon as possible. */
	bool m_ShouldTerminate;

	/** Event that is set when a game is started. */
	cEvent m_evtGameStart;

	/** Event that is set when a  game update is received that has the matching lastCmdId with our last sent cmdId. */
	cEvent m_evtCommandIdMatch;

	/** The last cmdId sent to the server. */
	volatile int m_LastSentCmdId;

	/** The last cmdId received from the server. */
	volatile int m_LastReceivedCmdId;

	/** The thread that queries the controller for new commands and sends them to the server, timed apart. */
	std::thread m_CommandSenderThread;


	/** Creates and opens the comm log file. */
	void openCommLogFile(void);

	/** Waits until the handshake is completed in the network thread. */
	bool waitForHandshakeCompletion(void);

	/** Called by the network callbacks when there's data incoming from the server.
	Logs the data, if requested, and processes any full lines present. */
	void onIncomingData(const AString & a_Data);

	/** Processes one line of incoming data. */
	void processLine(const AString & a_Line);

	/** Processes the "status: socket_connected" response, sends the login info. */
	void processSocketConnected(const Json::Value & a_Response);

	/** Processes the "status: login_ok" response, marks handshake finished. */
	void processLoginOK(const Json::Value & a_Response);

	/** Processes the "status: login_failed" response, aborts the connection. */
	void processLoginFailed(const Json::Value & a_Response);

	/** Processes the "game" response, starting a new game. */
	void processGame(const Json::Value & a_Response);

	/** Processes the "play" response, updating the game board. */
	void processPlay(const Json::Value & a_Response);

	/** Processes the "result" response, terminating the game. */
	void processResult(const Json::Value & a_Reponse);

	/** Severs the connection to the server and shuts down the comm interface. */
	void abortConnection(void);

	/** Runs the thread that periodically sends commands to the server. */
	void commandSenderThread(void);

	/** Sends the current commands to the server. */
	void sendCommands(void);
};




