
// Controller.h

// Declares the Controller class representing an interface that AI controllers need to implement





#pragma once





// fwd:
class BotWarzApp;
class Board;
namespace Json
{
	class Value;
}





class Controller
{
public:
	Controller(BotWarzApp & a_App) :
		m_App(a_App)
	{
	}

	/** Called when the game has just started.
	a_Board points to the game board that represents the game state.
	The board needs to stay valid until the game is finished via the onGameFinished() call. */
	virtual void onGameStarted(Board & a_Board) = 0;

	/** Called when a game update has been received. */
	virtual void onGameUpdate(void) = 0;

	/** Called when the current game has finished.
	The board that has represented this game can be released after this call returns. */
	virtual void onGameFinished(void) = 0;

	/** Returns the current set of commands for the bots that should be sent to the server.
	Also clears the commands, so that they aren't sent the next time this is called. */
	virtual Json::Value getBotCommands(void) = 0;

protected:
	BotWarzApp & m_App;
};




