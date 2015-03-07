
// Bot.h

// Declares the Bot class representing a single bot at a single moment in time

#pragma once

#include <memory>
#include <vector>





// fwd:
class QJsonObject;





class Bot
{
public:
	int m_Team;
	int m_ID;
	double m_X;
	double m_Y;
	double m_Angle;
	double m_Speed;

	/** Creates a new instance with the default values. */
	Bot(void);

	/** Creates a new instance and fills it with values from the json.
	a_Team is the number of the team that the bot belongs to (0 or 1). */
	Bot(int a_Team, const QJsonObject & a_Json);
};

typedef std::shared_ptr<Bot> BotPtr;
typedef std::vector<BotPtr> BotPtrs;





