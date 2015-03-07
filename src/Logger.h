
// Logger.h

// Declares the Logger class representing the logging framework





#pragma once

#include "lib/Network/CriticalSection.h"





class Logger
{
public:
	Logger(void);

	bool init(bool a_ShouldLogComm, bool a_ShouldShowComm);

	/** Logs communication data. */
	void commLog(bool a_IsIncoming, const AString & a_Data);

	/** Logs custom data pertaining to a specific bot. */
	void aiLog(int a_BotID, const AString & a_Message);

	/** Output a generic comment into the log. */
	void commentLog(const AString & a_Message);

protected:
	/** If true, all the communication with the server is sent to stdout. */
	bool m_ShouldShowComm;

	/** File into which all the communication is logged, nullptr if none.
	Protected against multithreaded writes by m_CSCommLog. */
	FILE * m_CommLogFile;

	/** File into which all the communication is binary-logged. */
	FILE * m_BinCommLogFile;

	/** Mutex protecting m_CommLogFile against multithreaded writes. */
	cCriticalSection m_CSCommLog;

	/** The timestamp of the commlogfile creation. Used to output relative time offsets in the commlog file */
	std::chrono::high_resolution_clock::time_point m_CommLogBeginTime;


	/** Creates the filename base for log files (binary and text). */
	static AString getLogFileNameBase(void);
};




