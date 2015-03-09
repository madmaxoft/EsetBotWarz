
// Logger.cpp

// Declares the Logger class representing the logging framework

#include "Globals.h"
#include "Logger.h"





// Various binary log data kinds:
static const char ldkDataIn  = 4;
static const char ldkDataOut = 5;
static const char ldkAILog   = 6;
static const char ldkComment = 7;

// Header of the binary log file:
char g_VersionHeader[] = "EBWLog\x00\x02";





Logger::Logger(void):
	m_ShouldShowComm(false),
	m_CommLogFile(nullptr),
	m_BinCommLogFile(nullptr)
{
}





bool Logger::init(bool a_ShouldLogComm, bool a_ShouldShowComm)
{
	// Create the folder for the logs, if not already present:
	#ifdef _WIN32
		CreateDirectoryA("CommLogs", nullptr);
	#else
		mkdir("CommLogs", S_IRWXU | S_IRWXG | S_IRWXO);
	#endif

	// Open the comm log file, if requested:
	m_CommLogBeginTime = std::chrono::high_resolution_clock::now();
	AString fileNameBase = getLogFileNameBase();
	if (a_ShouldLogComm)
	{
		AString logName = fileNameBase + ".txt";
		#ifdef _MSC_VER
			m_CommLogFile = _fsopen(logName.c_str(), "w", _SH_DENYWR);
		#else
			m_CommLogFile = fopen(logName.c_str(), "w");
		#endif
	}

	// Always open the binary log file:
	fileNameBase.append(".ebwlog");
	#ifdef _WIN32
		m_BinCommLogFile = _fsopen(fileNameBase.c_str(), "wb", _SH_DENYWR);
	#else
		m_BinCommLogFile = fopen(fileNameBase.c_str(), "wb");
	#endif
	fwrite(g_VersionHeader, sizeof(g_VersionHeader) - 1, 1, m_BinCommLogFile);  // g_VersionHeader variable is zero-terminated, don't output the terminator

	return true;
}





void Logger::commLog(bool a_IsIncoming, const AString & a_Data)
{
	UInt64 microSecOffset = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_CommLogBeginTime).count());
	double timeOffset = static_cast<double>(microSecOffset) / 1000;
	AString msg = Printf("%9.3f %s: %s", timeOffset, a_IsIncoming ? " IN" : "OUT", a_Data.c_str());

	// Show on stdout, if requested:
	if (m_ShouldShowComm)
	{
		printf("%s", msg.c_str());
	}

	// Output to file, if requested:
	cCSLock Lock(m_CSCommLog);
	if (m_CommLogFile != nullptr)
	{
		fprintf(m_CommLogFile, "%s", msg.c_str());
		fflush(m_CommLogFile);
	}

	// Always write a binary log:
	if (m_BinCommLogFile != nullptr)
	{
		UInt32 timeLow  = htonl(static_cast<UInt32>(microSecOffset));
		UInt32 timeHigh = htonl(static_cast<UInt32>(microSecOffset >> 32));
		fwrite(&timeHigh, 4, 1, m_BinCommLogFile);
		fwrite(&timeLow, 4, 1, m_BinCommLogFile);
		char kind = a_IsIncoming ? ldkDataIn : ldkDataOut;
		fwrite(&kind, 1, 1, m_BinCommLogFile);
		UInt32 len = htonl(static_cast<UInt32>(a_Data.size()));
		fwrite(&len, 4, 1, m_BinCommLogFile);
		fwrite(a_Data.data(), a_Data.size(), 1, m_BinCommLogFile);
	}
}





void Logger::aiLog(int a_BotID, const AString & a_Msg)
{
	UInt64 microSecOffset = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_CommLogBeginTime).count());
	double timeOffset = static_cast<double>(microSecOffset) / 1000;
	AString msg = Printf("%9.3f B#%d: %s\n", timeOffset, a_BotID, a_Msg.c_str());

	// Show on stdout, if requested:
	if (m_ShouldShowComm)
	{
		printf("%s", msg.c_str());
	}

	// Output to file, if requested:
	cCSLock Lock(m_CSCommLog);
	if (m_CommLogFile != nullptr)
	{
		fprintf(m_CommLogFile, "%s", msg.c_str());
		fflush(m_CommLogFile);
	}

	// Always write a binary log:
	if (m_BinCommLogFile != nullptr)
	{
		UInt32 timeLow  = htonl(static_cast<UInt32>(microSecOffset));
		UInt32 timeHigh = htonl(static_cast<UInt32>(microSecOffset >> 32));
		fwrite(&timeHigh, 4, 1, m_BinCommLogFile);
		fwrite(&timeLow, 4, 1, m_BinCommLogFile);
		char kind = ldkAILog;
		fwrite(&kind, 1, 1, m_BinCommLogFile);
		UInt32 len = htonl(static_cast<UInt32>(a_Msg.size() + 1));
		fwrite(&len, 4, 1, m_BinCommLogFile);
		char id = static_cast<char>(a_BotID);
		fwrite(&id, 1, 1, m_BinCommLogFile);
		fwrite(a_Msg.data(), a_Msg.size(), 1, m_BinCommLogFile);
	}
}





void Logger::commentLog(const AString & a_Msg)
{
	UInt64 microSecOffset = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_CommLogBeginTime).count());
	double timeOffset = static_cast<double>(microSecOffset) / 1000;
	AString msg = Printf("%9.3f   // %s\n", timeOffset, a_Msg.c_str());

	// Show on stdout, if requested:
	if (m_ShouldShowComm)
	{
		printf("%s", msg.c_str());
	}

	// Output to file, if requested:
	cCSLock Lock(m_CSCommLog);
	if (m_CommLogFile != nullptr)
	{
		fprintf(m_CommLogFile, "%s", msg.c_str());
		fflush(m_CommLogFile);
	}

	// Always write a binary log:
	if (m_BinCommLogFile != nullptr)
	{
		UInt32 timeLow  = htonl(static_cast<UInt32>(microSecOffset));
		UInt32 timeHigh = htonl(static_cast<UInt32>(microSecOffset >> 32));
		fwrite(&timeHigh, 4, 1, m_BinCommLogFile);
		fwrite(&timeLow, 4, 1, m_BinCommLogFile);
		char kind = ldkComment;
		fwrite(&kind, 1, 1, m_BinCommLogFile);
		UInt32 len = htonl(static_cast<UInt32>(a_Msg.size()));
		fwrite(&len, 4, 1, m_BinCommLogFile);
		fwrite(a_Msg.data(), a_Msg.size(), 1, m_BinCommLogFile);
	}
}





AString Logger::getLogFileNameBase(void)
{
	// Compose the log file name from the current time:
	time_t rawtime;
	time(&rawtime);
	struct tm * timeinfo;
	#ifdef _MSC_VER
		struct tm timeinforeal;
		timeinfo = &timeinforeal;
		localtime_s(timeinfo, &rawtime);
	#else
		timeinfo = localtime(&rawtime);
	#endif
	return Printf("CommLogs/%02d-%02d-%02d-%02d-%02d-%02d", 
		(timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1), timeinfo->tm_mday,
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec
	);
}




