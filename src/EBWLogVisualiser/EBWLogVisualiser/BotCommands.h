
// BotCommands.h

// Declares the BotCommands class representing the commands sent to bots by the client.

#pragma once

#include <memory>
#include <vector>
#include <QJsonArray>






class BotCommands
{
public:
	/** The kind of the command being sent.
	cmdUnknown is used when reporting the aggregate commands for bots that haven't had any command sent to them yet. */
	enum CommandKind
	{
		cmdSteer,
		cmdAccelerate,
		cmdBrake,
		cmdUnknown,
	};

	/** Container for a single command being sent. */
	struct BotCmd
	{
		/** The ID of the bot to whom the command is sent. */
		int m_BotID;

		/** The kind of the command being sent. */
		BotCommands::CommandKind m_Kind;

		/** The optional parameter for the command. */
		double m_Param;

		BotCmd(int a_BotID, BotCommands::CommandKind a_Kind, double a_Param):
			m_BotID(a_BotID),
			m_Kind(a_Kind),
			m_Param(a_Param)
		{
		}
	};
	typedef std::vector<BotCmd> BotCmds;


	/** The client's time of this batch. */
	quint64 m_ClientTime;

	/** The commands sent in this batch. */
	BotCmds m_Commands;


	/** Creates a new instance with default values. */
	BotCommands(void);

	/** Creates a new instance based on the json commands sent to the server. */
	BotCommands(quint64 a_ClientTime, const QJsonArray & a_JsonCmds);
};




