
// LogFile.cpp

// Implements the LogFile class representing a single log file containing possibly multiple games

#include "LogFile.h"
#include <QFile>
#include <QtEndian>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "Game.h"
#include "GameState.h"
#include "BotCommands.h"





static const char leDataIn  = 1;
static const char leDataOut = 2;
static const char leComment = 32;




LogFile::LogFile(void)
{
	// Nothing needed yet
}





QString LogFile::readFile(const QString & a_FileName)
{
	QFile f(a_FileName);
	if (!f.open(QIODevice::ReadOnly))
	{
		return "Cannot open file";
	}

	// Check the version header:
	static const char versionHeader[] = "EBWLog\x00\x01";
	QByteArray hdr = f.read(8);
	if (memcmp(hdr.constData(), versionHeader, 8) != 0)
	{
		return "File is from a different version";
	}

	// Load individual events:
	GamePtr curGame;
	GameStatePtr curGameState;
	QByteArray incomingDataBuffer;
	while (!f.atEnd())
	{
		quint64 timeStamp = qFromBigEndian<quint64>(reinterpret_cast<const uchar *>(f.read(8).constData()));
		char kind;
		f.read(&kind, 1);
		qint32 len;
		f.read(reinterpret_cast<char *>(&len), 4);
		len = qFromBigEndian(len);
		QByteArray ba = f.read(len);
		switch (kind)
		{
			case leDataIn:
			{
				// Add the incoming data to a buffer, parse each full line received:
				incomingDataBuffer.append(ba);
				while (true)
				{
					int lineEnd = incomingDataBuffer.indexOf('\n');
					if (lineEnd < 0)
					{
						break;
					}

					// Parse the data received from the server:
					auto json = QJsonDocument::fromJson(incomingDataBuffer.left(lineEnd));
					if (!json.isObject())
					{
						return "Parse error";
					}
					auto jsonObj = json.object();
					incomingDataBuffer.remove(0, lineEnd + 1);

					// Process the "game" response (start a new game):
					auto itrGame = jsonObj.find("game");
					if (itrGame != jsonObj.end())
					{
						// Create a new game object:
						curGameState = std::make_shared<GameState>(
							timeStamp, (*itrGame).toObject()["time"].toInt(),
							(*itrGame).toObject()["players"].toArray()
						);
						curGame = std::make_shared<Game>(timeStamp, curGameState, *itrGame);
						m_Games.push_back(curGame);
						continue;
					}

					// Process the "play" response (add a new gamestate):
					auto itrPlay = jsonObj.find("play");
					if ((itrPlay != jsonObj.end()) && (curGame != nullptr))
					{
						// Create a new game state object:
						auto play = (*itrPlay).toObject();
						curGameState = std::make_shared<GameState>(
							timeStamp, play["time"].toInt(), play["players"].toArray()
						);
						curGame->addGameState(curGameState);
						continue;
					}

					// Process the "result" response (finalize current game):
					auto itrResult = jsonObj.find("result");
					if ((itrResult != jsonObj.end()) && (curGame != nullptr))
					{
						curGame->finish(timeStamp, *itrResult);
						curGame.reset();
						curGameState.reset();
						continue;
					}
				}  // while (true)
				break;
			}
			case leDataOut:
			{
				// Parse the data sent to the server:
				auto json = QJsonDocument::fromJson(ba);
				if (!json.isObject())
				{
					return "Parse error";
				}
				auto jsonObj = json.object();

				if (curGame != nullptr)
				{
					curGame->addBotCommands(std::make_shared<BotCommands>(
						timeStamp, jsonObj["bots"].toArray()
					));
				}
				break;
			}
			case leComment:
			{
				// Add the data as a comment
				// TODO
				break;
			}
		}
	}
	// TODO
	return "";
}




