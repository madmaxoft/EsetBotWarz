
// Comm.h

// Declares the Comm class that encapsulates the communication with the BotWarz server





#pragma once

#include "lib/Network/Network.h"
#include "lib/Network/Event.h"





// fwd:
class BotWarzApp;
namespace Json
{
	class Value;
};





class Comm
{
public:
	Comm(const AString a_LoginToken, const AString & a_LoginNick, BotWarzApp & a_App);

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

	/** The token to be used for login. */
	AString m_LoginToken;

	/** The nick to be used for login. */
	AString m_LoginNick;

	/** If true, all the communication with the server is sent to stdout. */
	bool m_ShouldShowComm;

	/** If true, all the communication with the server is logged into m_CommLogFile. */
	bool m_ShouldLogComm;

	/** File into which all the communication is logged, if m_ShouldLogComm is true. */
	FILE * m_CommLogFile;

	/** The TCP link to the server. */
	cTCPLinkPtr m_Link;

	/** Partial data that has been received from the server but not yet processed (incomplete line). */
	AString m_QueuedData;

	/** Synchronization between the network thread and the main thread waiting for handshake completion. */
	cEvent m_evtHandshake;

	/** The current status of the connection. */
	Status m_Status;


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

	/** Severs the connection to the server and shuts down the comm interface. */
	void abortConnection(void);
};




