
// Bot.h

// Declares the Bot class representing a single bot in the game





#pragma once





// fwd:
class Board;
namespace Json
{
	class Value;
}





class Bot
{
public:
	Board & m_Board;
	int m_ID;
	bool m_IsEnemy;
	double m_X;
	double m_Y;
	double m_Speed;
	double m_Angle;


	/** Creates a new bot instance tied to the specified board.
	a_Values is the contents of the "bots" array item of the server response, it is used to initialize the coords and angle. */
	Bot(Board & a_Board, int a_ID, bool a_IsEnemy, const Json::Value & a_Values);

	/** Updates the bot data from the specified json value.
	a_Value is the contents of the "bots" array item of the server response. */
	void updateFromJson(const Json::Value & a_Value);
};

typedef SharedPtr<Bot> BotPtr;
typedef std::vector<BotPtr> BotPtrs;
typedef std::map<int, BotPtr> BotIDMap;




