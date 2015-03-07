
// GameState.cpp

// Declares the GameState class representing a state of a single game at a single moment in time

#include "GameState.h"
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include "Bot.h"





GameState::GameState(void):
	m_ClientTime(0),
	m_ServerTime(0),
	m_RequestedTime(0)
{
}





GameState::GameState(quint64 a_ClientTime, int a_ServerTime, const QJsonArray & a_JsonPlayers):
	m_ClientTime(a_ClientTime),
	m_ServerTime(a_ServerTime),
	m_RequestedTime(a_ClientTime)
{
	int idx = 0;
	for (auto & player: a_JsonPlayers)
	{
		auto bots = player.toObject()["bots"].toArray();
		for (auto & bot: bots)
		{
			m_Bots.push_back(std::make_shared<Bot>(idx, bot.toObject()));
		}
		++idx;
	}  // for player - players[]
}





BotPtr GameState::getBotByID(int a_BotID)
{
	for (auto & b: m_Bots)
	{
		if (b->m_ID == a_BotID)
		{
			return b;
		}
	}
	return nullptr;
}




