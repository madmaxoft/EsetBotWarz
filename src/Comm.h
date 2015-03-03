
// Comm.h

// Declares the Comm class that encapsulates the communication with the BotWarz server





#pragma once

#include "lib/Network/Network.h"





// fwd:
namespace Json
{
	class Value;
};





class Comm
{
public:
	Comm(const AString a_LoginToken, const AString & a_LoginNick);

	/** Initializes the subsystem.
	If a_ShouldLogComm is true, all the communication with the server is logged into a file.
	If a_ShouldShowComm is true, all the communication with the server is output to stdout.
	Returns true if successful, logs the reason and returns false to indicate failure. */
	bool init(bool a_ShouldLogComm, bool a_ShouldShowComm);

	/** Sends the data to the server, logging it to file if requested. */
	void send(const AString & a_Data);

	/** Sends the json data to the server, logging it to file if requested. */
	void send(const Json::Value & a_Data);

protected:
	friend class Callbacks;

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

	/** Severs the connection to the server and shuts down the comm interface. */
	void abortConnection(void);
};




