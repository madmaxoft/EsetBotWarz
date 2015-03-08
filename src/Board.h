
// Board.h

// Declares the Board class representing the game board state





#pragma once

#include "lib/Network/CriticalSection.h"
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


	Board(BotWarzApp & a_App);

	/** (Re-)initializes the board from the game-start data.
	a_GameData is the contents of the "game" tag of the server's message. */
	void initialize(const Json::Value & a_GameData);

	/** Updates the board contents based on the json received from the server.
	a_Board is the contents of the "play" tag from the server's message. */
	void updateFromJson(const Json::Value & a_Board);

	const SpeedLevels & getSpeedLevels(void) const { return m_SpeedLevels; }
	double getWorldWidth(void) const { return m_Width; }
	double getWorldHeight(void) const { return m_Height; }
	double getBotRadius(void) const { return m_BotRadius; }

	/** Returns a copy of the m_MyBots field, in a thread-safe way. */
	BotPtrs getMyBotsCopy(void) const;

	/** Returns a copy of the m_AllBots field, in a thread-safe way. */
	BotIDMap getAllBotsCopy(void) const;

	/** Returns the local timestamp of the game start. */
	std::chrono::system_clock::time_point getLocalGameStartTime(void) const { return m_LocalGameStartTime; }

	/** Returns the server time of the last update. */
	int getServerTime(void) const { return m_ServerTime; }

protected:
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

	/** The mutex protecting m_MyBots, m_EnemyBots and m_AllBots against multithreaded access. */
	mutable cCriticalSection m_CSBots;

	/** The local timestamp of the game start. */
	std::chrono::system_clock::time_point m_LocalGameStartTime;

	/** The server time of the last update. */
	int m_ServerTime;
};




