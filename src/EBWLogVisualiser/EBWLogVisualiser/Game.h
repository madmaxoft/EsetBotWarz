
// Game.h

// Declares the Game class representing an entire single game

#pragma once

#include <memory>
#include <vector>
#include <QString>





// fwd:
class GameState;
typedef std::shared_ptr<GameState> GameStatePtr;
typedef std::vector<GameStatePtr> GameStatePtrs;
class BotCommands;
typedef std::shared_ptr<BotCommands> BotCommandsPtr;
typedef std::vector<BotCommandsPtr> BotCommandsPtrs;
class Comment;
typedef std::shared_ptr<Comment> CommentPtr;
typedef std::vector<CommentPtr> CommentPtrs;
class QJsonValue;





class Game
{
public:
	/** Creates a new game with default settings. */
	Game(void);

	/** Creates a new game and initializes it from the json ("game" server message). */
	Game(quint64 a_ClientTime, GameStatePtr a_InitialState, const QJsonValue & a_Json);

	/** Adds a new state to the game. To be used only when loading the log file.
	Adjusts m_TotalTime, if appropriate. */
	void addGameState(GameStatePtr a_GameState);

	/** Adds a new botcommands to the game. To be used only when loading the log file.
	Adjusts m_TotalTime, if appropriate. */
	void addBotCommands(BotCommandsPtr a_BotCommands);

	/** Adds a new comment to the game. To be used only when loading the log file.
	Adjusts m_TotalTime, if appropriate. */
	void addComment(CommentPtr a_Comment);

	/** Sorts all gamestates and comments by their time.
	Called when loading the log file, when the game has been fully parsed. */
	void finish(quint64 a_ClientTime, const QJsonValue & a_Json);

	/** Returns the state of the game at the specified relative client time.
	Returns the latest state earlier than or equal to the specified time. */
	GameStatePtr getGameStateAt(quint64 a_RelClientTime);

	/** Returns the bot commands at the specified relative client time.
	Returns the latest commands earlier than or equal to the specified time. */
	BotCommandsPtr getBotCommandsAt(quint64 a_RelClientTime);

	/** Returns the timestamps of all the gamestates. */
	void getGameStateTimestamps(std::vector<quint64> & a_Timestamps);

	/** Returns the timestamps of all the botcommands. */
	void getBotCommandTimestamps(std::vector<quint64> & a_Timestamps);

	const QString & getPlayer1Name(void) const { return m_Player1Name; }
	const QString & getPlayer2Name(void) const { return m_Player2Name; }
	quint64 getGameStartTime(void) const { return m_GameStartTime; }
	quint64 getTotalTime(void) const { return m_TotalTime; }

protected:
	/** Name of the first player. */
	QString m_Player1Name;

	/** Name of the second player. */
	QString m_Player2Name;

	/** Client time of the game start. */
	quint64 m_GameStartTime;

	/** Total game time (in client time). */
	quint64 m_TotalTime;

	/** The game states parsed from the game log. */
	GameStatePtrs m_GameStates;

	/** The bot commands parsed from the game log. */
	BotCommandsPtrs m_BotCommands;

	/** The comments parsed from the game log. */
	CommentPtrs m_Comments;
};

typedef std::shared_ptr<Game> GamePtr;
typedef std::vector<GamePtr> GamePtrs;




