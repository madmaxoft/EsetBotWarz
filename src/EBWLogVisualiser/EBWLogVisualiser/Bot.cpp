
// Bot.cpp

// Implements the Bot class representing a single bot at a single moment in time

#include "Bot.h"
#include <QJsonObject>





Bot::Bot(void):
	m_Team(0),
	m_ID(0),
	m_X(0),
	m_Y(0),
	m_Angle(0),
	m_Speed(0)
{
}





Bot::Bot(int a_Team, const QJsonObject & a_Json):
	m_Team(a_Team)
{
	m_ID    = a_Json["id"].toInt();
	m_X     = a_Json["x"].toDouble();
	m_Y     = a_Json["y"].toDouble();
	m_Angle = a_Json["angle"].toDouble();
	m_Speed = a_Json["speed"].toDouble();
}




