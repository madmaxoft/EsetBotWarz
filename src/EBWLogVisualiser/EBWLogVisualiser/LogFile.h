
// LogFile.h

// Declares the LogFile class representing a single log file containing possibly multiple games

#pragma once

#include <memory>
#include <vector>
#include <QString>





// fwd:
class Game;
typedef std::shared_ptr<Game> GamePtr;
typedef std::vector<GamePtr> GamePtrs;






class LogFile
{
public:
	LogFile(void);

	/** Reads the specified file, parsing the games in it.
	Returns empty string if successful, error string on failure. */
	QString readFile(const QString & a_FileName);

	/** Clears all the games stored in the class. */
	void clear(void);

	const GamePtrs & getGames(void) const { return m_Games; }

protected:
	/** The games contained in the log file. */
	GamePtrs m_Games;
};




