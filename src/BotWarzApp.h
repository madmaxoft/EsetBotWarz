
// BotWarzApp.h

// Declares the BotWarzApp class representing the entire application. Acts as the flow control hub, relaying messages from modules to all other modules.





#pragma once

#include "Comm.h"
#include "Board.h"
#include "lib/Network/Event.h"





// fwd:
class Controller;





class BotWarzApp
{
public:
	BotWarzApp(const AString a_LoginToken, const AString & a_LoginNick);

	/** Runs the entire application.
	If a_ShouldLogComm is true, all the communication with the server is logged into a file.
	If a_ShouldShowComm is true, all the communication with the server is output to stdout.
	a_ControllerFileName is the name of the Lua file to use for the controller.
	If a_ShouldDebugZBS is true, a ZBS debugger code is prepended to the Lua controller script, enabling debugging in ZeroBrane Studio.
	Returns the value that the process should return to the OS upon its exit. */
	int run(bool a_ShouldLogComm, bool a_ShouldShowComm, const AString & a_ControllerFileName, bool a_ShouldDebugZBS);

	/** Notifies the app that it should terminate.
	Wakes up the main thread to do the actual termination. */
	void terminate(void);

	/** Starts a new game.
	a_GameData is the contents of the "game" tag of the server message. */
	void startGame(const Json::Value & a_GameData);

	/** Updates the board based on the data received form the server.
	a_Board is the contents of the "play" tag in the game update message. */
	void updateBoard(const Json::Value & a_Board);

	/** Called when the current game is finished.
	a_ResultData is the contents of the "result" tag of the server message. */
	void finishGame(const Json::Value & a_ResultData);

	const AString & getLoginToken(void) const { return m_LoginToken; }
	const AString & getLoginNick(void) const { return m_LoginNick; }

protected:
	/** The representation of the game board. */
	Board m_Board;

	/** The AI controller to use for driving the bots. */
	SharedPtr<Controller> m_Controller;

	/** The communication interface to the server. */
	Comm m_Comm;

	/** Event that is signalled upon termination request. */
	cEvent m_evtTerminate;

	/** The token to be used for login. */
	AString m_LoginToken;

	/** The nick to be used for login. */
	AString m_LoginNick;

};




