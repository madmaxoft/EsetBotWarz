
// BotCommands.cpp

// Implements the BotCommands class representing the commands sent to bots by the client.

#include "BotCommands.h"
#include <QJsonObject>





BotCommands::BotCommands(void):
	m_ClientTime(0)
{
}





BotCommands::BotCommands(quint64 a_ClientTime, const QJsonArray & a_JsonCmds):
	m_ClientTime(a_ClientTime)
{
	for (auto & bot: a_JsonCmds)
	{
		auto b = bot.toObject();
		auto cmd = b["cmd"].toString();
		CommandKind kind = cmdUnknown;
		double param = 0;
		if (cmd == "accelerate")
		{
			kind = cmdAccelerate;
		}
		else if (cmd == "brake")
		{
			kind = cmdBrake;
		}
		else if (cmd == "steer")
		{
			kind = cmdSteer;
			param = b["angle"].toDouble();
		}
		m_Commands.emplace_back(b["id"].toInt(), kind, param);
	}
}




