
// Bot.cpp

// Implements the Bot class representing a single bot in the game

#include "Globals.h"
#include "Bot.h"
#include "json/value.h"





Bot::Bot(Board & a_Board, int a_ID, bool a_IsEnemy, const Json::Value & a_Values):
	m_Board(a_Board),
	m_ID(a_ID),
	m_IsEnemy(a_IsEnemy)
{
	updateFromJson(a_Values);
}





void Bot::updateFromJson(const Json::Value & a_Value)
{
	m_X = a_Value["x"].asDouble();
	m_Y = a_Value["y"].asDouble();
	m_Speed = a_Value["speed"].asDouble();
	m_Angle = a_Value["angle"].asDouble();
}




