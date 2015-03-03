
// Board.cpp

// Implements the Board class representing the game board state

#include "Globals.h"
#include "Board.h"
#include "json/value.h"
#include "BotWarzApp.h"





Board::Board(BotWarzApp & a_App):
	m_App(a_App)
{
	// Nothing needed yet
}





void Board::initialize(const Json::Value & a_GameData)
{
	// Read the world parameters:
	m_Width = a_GameData["world"]["width"].asDouble();
	m_Height = a_GameData["world"]["height"].asDouble();
	m_BotRadius = a_GameData["botRadius"].asDouble();

	// Read the speed list:
	m_SpeedLevels.clear();
	auto speedLevels = a_GameData["SpeedLevels"];
	for (auto itr = speedLevels.begin(), end = speedLevels.end(); itr != end; ++itr)
	{
		auto & level = *itr;
		m_SpeedLevels.emplace_back(level["speed"].asDouble(), level["maxAngle"].asDouble());
	}  // for itr - speedLevels[]

	// Create the bots:
	m_MyBots.clear();
	m_EnemyBots.clear();
	auto & players = a_GameData["players"];
	for (auto itrP = players.begin(), endP = players.end(); itrP != endP; ++itrP)
	{
		auto & player = *itrP;
		bool isEnemy = (player["nickname"] != m_App.getLoginNick());
		if (isEnemy)
		{
			m_EnemyName = player["nickname"].asString();
		}
		auto & botArray = isEnemy ? m_EnemyBots : m_MyBots;
		auto & bots = player["bots"];
		for (auto itrB = bots.begin(), endB = bots.end(); itrB != endB; ++itrB)
		{
			auto & bot = *itrB;
			botArray.push_back(std::make_shared<Bot>(*this, bot["id"].asInt(), isEnemy, bot));
		}  // for itrB - bots[]
	}  // for itrP - players[]
	// TODO
}





void Board::updateFromJson(const Json::Value & a_GameData)
{
	// TODO
}




