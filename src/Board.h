
// Board.h

// Declares the Board class representing the game board state





#pragma once

#include "Bot.h"




// fwd:
class BotWarzApp;
namespace Json
{
	class Value;
};





class Board
{
public:
	Board(BotWarzApp & a_App);

	/** (Re-)initializes the board from the game-start data.
	a_GameData is the contents of the "game" tag of the server's message. */
	void initialize(const Json::Value & a_GameData);

	/** Updates the board contents based on the json received from the server.
	a_Board is the contents of the "play" tag from the server's message. */
	void updateFromJson(const Json::Value & a_Board);

protected:
	/** Defines a single level of speed that the bot can use. */
	struct SpeedLevel
	{
		/** The linear speed at which the bot moves in this level. */
		double m_LinearSpeed;

		/** The maximum angular rotation speed at which the bot can move in this level. */
		double m_MaxAngularSpeed;

		SpeedLevel(double a_LinearSpeed, double a_MaxAngularSpeed):
			m_LinearSpeed(a_LinearSpeed),
			m_MaxAngularSpeed(a_MaxAngularSpeed)
		{
		}
	};

	typedef std::vector<SpeedLevel> SpeedLevels;


	/** The parent App object. */
	BotWarzApp & m_App;

	/** World width (X coord). */
	double m_Width;

	/** World height (Y coord). */
	double m_Height;

	/** Radius of each individual bot. */
	double m_BotRadius;

	/** The available speed levels. */
	SpeedLevels m_SpeedLevels;

	/** Vector of my bots currently present on the board. */
	BotPtrs m_MyBots;

	/** Vector of enemy bots currently present on the board. */
	BotPtrs m_EnemyBots;

	/** Map of Bot ID -> Bot ptr for all bots currently present on the board. */
	BotIDMap m_AllBots;

	/** The nickname of the enemy. */
	AString m_EnemyName;
};




