
// Game.cpp

// Implements the Game class representing an entire single game

#include "Game.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include "GameState.h"
#include "BotCommands.h"





Game::Game(void):
	m_TotalTime(0)
{
}





Game::Game(quint64 a_ClientTime, GameStatePtr a_InitialState, const QJsonValue & a_Json):
	m_GameStartTime(a_ClientTime),
	m_TotalTime(0)
{
	auto game = a_Json.toObject();
	auto players = game["players"];
	m_Player1Name = players.toArray()[0].toObject()["nickname"].toString();
	m_Player2Name = players.toArray()[1].toObject()["nickname"].toString();
	m_GameStates.push_back(a_InitialState);
}





void Game::addGameState(GameStatePtr a_GameState)
{
	// Add the state:
	m_GameStates.push_back(a_GameState);

	// Update the total time, if needed:
	if (m_GameStartTime + a_GameState->m_ClientTime > m_TotalTime)
	{
		m_TotalTime = a_GameState->m_ClientTime - m_GameStartTime;
	}
}





void Game::addBotCommands(BotCommandsPtr a_BotCommands)
{
	// Add the commands:
	m_BotCommands.push_back(a_BotCommands);

	// Update the total time, if needed:
	if (m_GameStartTime + a_BotCommands->m_ClientTime > m_TotalTime)
	{
		m_TotalTime = a_BotCommands->m_ClientTime - m_GameStartTime;
	}
}





void Game::finish(quint64 a_ClientTime, const QJsonValue & a_Json)
{
	// Update the total time, if needed:
	if (m_GameStartTime + a_ClientTime > m_TotalTime)
	{
		m_TotalTime = a_ClientTime - m_GameStartTime;
	}

	// Sort the game states by their time:
	std::sort(m_GameStates.begin(), m_GameStates.end(),
		[=](GameStatePtr & a_First, GameStatePtr & a_Second)
		{
			return (a_First->m_ClientTime < a_Second->m_ClientTime);
		}
	);

	// TODO: Sort the botcommands by their time:
	// TODO: Sort the comments by their time
	// TODO: Set the winner name and the score
}





void Game::getGameStateTimestamps(std::vector<quint64> & a_Timestamps)
{
	a_Timestamps.reserve(m_GameStates.size());
	for (auto & state: m_GameStates)
	{
		a_Timestamps.push_back(state->m_ClientTime);
	}  // for state - m_GameStates[]
}





void Game::getBotCommandTimestamps(std::vector<quint64> & a_Timestamps)
{
	a_Timestamps.reserve(m_BotCommands.size());
	for (auto & cmd: m_BotCommands)
	{
		a_Timestamps.push_back(cmd->m_ClientTime);
	}  // for cmd - m_BotCommands[]
}




