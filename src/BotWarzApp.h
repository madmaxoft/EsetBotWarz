
// BotWarzApp.h

// Declares the BotWarzApp class representing the entire application





#pragma once

#include "Comm.h"
#include "Board.h"
#include "lib/Network/Event.h"





class BotWarzApp
{
public:
	BotWarzApp(const AString a_LoginToken, const AString & a_LoginNick);

	/** Runs the entire application.
	If a_ShouldLogComm is true, all the communication with the server is logged into a file.
	If a_ShouldShowComm is true, all the communication with the server is output to stdout.
	Returns the value that the process should return to the OS upon its exit. */
	int run(bool a_ShouldLogComm, bool a_ShouldShowComm);

	/** Notifies the app that it should terminate.
	Wakes up the main thread to do the actual termination. */
	void terminate(void);

	/** Starts a new game.
	a_GameData is the contents of the "game" tag of the server message. */
	void startGame(const Json::Value & a_GameData);

	/** Updates the board based on the data received form the server.
	a_Board is the contents of the "play" tag in the game update message. */
	void updateBoard(const Json::Value & a_Board);

	const AString & getLoginToken(void) const { return m_LoginToken; }
	const AString & getLoginNick(void) const { return m_LoginNick; }

protected:
	/** The representation of the game board. */
	Board m_Board;

	/** The communication interface to the server. */
	Comm m_Comm;

	/** Event that is signalled upon termination request. */
	cEvent m_evtTerminate;

	/** The token to be used for login. */
	AString m_LoginToken;

	/** The nick to be used for login. */
	AString m_LoginNick;

};




