
// GameState.h

// Declares the GameState class representing a state of a single game at a single moment in time

#pragma once

#include <memory>
#include <vector>
#include <qglobal.h>





// fwd:
class Bot;
typedef std::shared_ptr<Bot> BotPtr;
typedef std::vector<BotPtr> BotPtrs;
class QJsonArray;





class GameState
{
public:
	/** The time at which the game state has occurred. */
	quint64 m_ClientTime;

	/** The server game time at which the game state has occurred. */
	int m_ServerTime;

	/** The client time at which the game state has been requested.
	Is greater or equal to m_ClientTime.
	Can be used to guess the bot positions for in between two states.
	Only used in gamestates retrieved by Game::getGameStateAt(). */
	quint64 m_RequestedTime;

	/** States of each bot in the game. */
	BotPtrs m_Bots;


	/** Creates a new instance with default values. */
	GameState(void);

	/** Creates a new instance and initializes it from the specified json.
	a_Json is the contents of the "players" object in the server message. */
	GameState(quint64 a_ClientTime, int a_ServerTime, const QJsonArray & a_JsonPlayers);

	/** Returns the bot out of m_Bots that has the specified ID, or nullptr if no such bot. */
	BotPtr getBotByID(int a_BotID);
};

typedef std::shared_ptr<GameState> GameStatePtr;




