
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
	auto speedLevels = a_GameData["speedLevels"];
	for (auto itr = speedLevels.begin(), end = speedLevels.end(); itr != end; ++itr)
	{
		auto & level = *itr;
		m_SpeedLevels.emplace_back(level["speed"].asDouble(), level["maxAngle"].asDouble());
	}  // for itr - speedLevels[]

	// Create the bots:
	cCSLock Lock(m_CSBots);
	m_MyBots.clear();
	m_EnemyBots.clear();
	auto & players = a_GameData["players"];
	for (auto itrP = players.begin(), endP = players.end(); itrP != endP; ++itrP)
	{
		auto & player = *itrP;
		bool isEnemy = (NoCaseCompare(player["nickname"].asString(), m_App.getLoginNick()) != 0);
		if (isEnemy)
		{
			m_EnemyName = player["nickname"].asString();
		}
		auto & botArray = isEnemy ? m_EnemyBots : m_MyBots;
		auto & bots = player["bots"];
		for (auto itrB = bots.begin(), endB = bots.end(); itrB != endB; ++itrB)
		{
			auto & bot = *itrB;
			auto botItem = std::make_shared<Bot>(*this, bot["id"].asInt(), isEnemy, bot);
			botArray.push_back(botItem);
			m_AllBots[botItem->m_ID] = botItem;
		}  // for itrB - bots[]
	}  // for itrP - players[]

	// Set the local game start time:
	m_LocalGameStartTime = std::chrono::system_clock::now();
}





void Board::updateFromJson(const Json::Value & a_GameData)
{
	// Update the server time:
	m_ServerTime = a_GameData["time"].asInt();

	// Update the bot arrays
	cCSLock Lock(m_CSBots);
	std::vector<int> presentIDs;
	for (int i = 0; i < 2; i++)
	{
		auto & bots = a_GameData["players"][i]["bots"];
		for (auto itrB = bots.begin(), endB = bots.end(); itrB != endB; ++itrB)
		{
			auto & bot = *itrB;
			int id = bot["id"].asInt();
			presentIDs.push_back(id);
			m_AllBots[id]->updateFromJson(bot);
		}  // for itrB - bots[]
	}  // for i - two players

	// Remove bots that haven't been reported:
	for (auto itr = m_AllBots.begin(), end = m_AllBots.end(); itr != end;)
	{
		if (std::find(presentIDs.begin(), presentIDs.end(), itr->first) == presentIDs.end())
		{
			m_App.botDied(*(itr->second));
			itr = m_AllBots.erase(itr);
		}
		else
		{
			++itr;
		}
	}  // for itr - m_AllBots[]
	for (auto itr = m_MyBots.begin(); itr != m_MyBots.end();)
	{
		if (std::find(presentIDs.begin(), presentIDs.end(), (*itr)->m_ID) == presentIDs.end())
		{
			itr = m_MyBots.erase(itr);
		}
		else
		{
			++itr;
		}
	}  // for itr - m_MyBots[]
	for (auto itr = m_EnemyBots.begin(); itr != m_EnemyBots.end();)
	{
		if (std::find(presentIDs.begin(), presentIDs.end(), (*itr)->m_ID) == presentIDs.end())
		{
			itr = m_EnemyBots.erase(itr);
		}
		else
		{
			++itr;
		}
	}  // for itr - m_EnemyBots[]
}





BotPtrs Board::getMyBotsCopy(void) const
{
	cCSLock Lock(m_CSBots);
	return m_MyBots;
}





BotIDMap Board::getAllBotsCopy(void) const
{
	cCSLock Lock(m_CSBots);
	return m_AllBots;
}




