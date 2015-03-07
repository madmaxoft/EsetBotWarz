
// GameDisplay.cpp

// Implements the GameDisplay class representing the widget displaying the game state

#include "GameDisplay.h"
#include <QPainter>
#include "GameState.h"
#include "BotCommands.h"
#include "Bot.h"





GameDisplay::GameDisplay(QWidget * a_Parent):
	Super(a_Parent)
{
}




void GameDisplay::setGameState(GameStatePtr a_GameState)
{
	m_GameState = a_GameState;
	redraw();
}





void GameDisplay::setBotCommands(BotCommandsPtr a_Cmds)
{
	m_BotCommands = a_Cmds;
	redraw();
}





void GameDisplay::redraw()
{
	update();
}





void GameDisplay::paintEvent(QPaintEvent * a_Event)
{
	// Paint the arena:
	QPainter p(this);
	p.setBrush(QColor(255, 255, 255));
	p.fillRect(0, 0, 900, 600, Qt::SolidPattern);

	// Get the gamestate, multithread-safe:
	auto gameState = m_GameState;
	if (gameState == nullptr)
	{
		return;
	}

	// Paint the commands:
	paintBotCommands(p);

	// Paint the game state:
	static const QColor teamColors[2] =
	{
		QColor(192, 0, 0),
		QColor(0, 0, 255),
	};
	static const QPen teamEllipsePens[2] =
	{
		QPen(QBrush(teamColors[0]), 4),
		QPen(QBrush(teamColors[1]), 4),
	};
	static const QPen teamDetailPens[2] =
	{
		QPen(QBrush(teamColors[0]), 0),
		QPen(QBrush(teamColors[1]), 0),
	};
	static const QPen textPen = QPen(QColor(255, 255, 255));
	p.setBrush(QBrush(Qt::NoBrush));
	for (auto & b: gameState->m_Bots)
	{
		p.setPen(teamEllipsePens[b->m_Team]);
		p.drawEllipse(b->m_X - 20, b->m_Y - 20, 41, 41);
		p.setPen(teamDetailPens[b->m_Team]);
		double angle = (b->m_Angle + 45) / 180 * 3.1415926;
		p.drawLine(b->m_X, b->m_Y, b->m_X + 20 * cos(angle), b->m_Y + 20 * sin(angle));
		angle = (b->m_Angle - 45) / 180 * 3.1415926;
		p.drawLine(b->m_X, b->m_Y, b->m_X + 20 * cos(angle), b->m_Y + 20 * sin(angle));
		angle = b->m_Angle / 180 * 3.1415926;
		p.drawLine(b->m_X, b->m_Y, b->m_X + b->m_Speed * cos(angle), b->m_Y + b->m_Speed * sin(angle));
		p.setPen(textPen);
		p.drawText(b->m_X + 20, b->m_Y + 20, QString::number(b->m_ID));
	}  // for b - gameState->m_Bots[]
}





void GameDisplay::paintBotCommands(QPainter & a_Painter)
{
	// Get the bot commands and game state, multithread-safe:
	auto botCommands = m_BotCommands;
	if (botCommands == nullptr)
	{
		return;
	}
	auto gameState = m_GameState;
	if (gameState == nullptr)
	{
		return;
	}

	// Define brushes to use:
	static const QBrush brushAccel   = QBrush(QColor(0,   192, 0));
	static const QPen   penAccel     = QPen  (QColor(0,   192, 0));
	static const QBrush brushBrake   = QBrush(QColor(255,   0, 0));
	static const QPen   penBrake     = QPen  (QColor(255,   0, 0));
	static const QBrush brushSteer   = QBrush(QColor(192, 192, 0));
	static const QBrush brushUnknown = QBrush(QColor(0,     0, 0));
	static const QPen   penSteer     = QPen  (QColor(192, 192, 0));

	// Paint each command:
	for (auto & c: botCommands->m_Commands)
	{
		// Get the bot to paint:
		auto bot = gameState->getBotByID(c.m_BotID);
		if (bot == nullptr)
		{
			continue;
		}

		// Paint the command:
		switch (c.m_Kind)
		{
			case BotCommands::cmdAccelerate:
			{
				a_Painter.setBrush(brushAccel);
				a_Painter.setPen(penAccel);
				a_Painter.drawEllipse(bot->m_X - 10, bot->m_Y - 10, 20, 20);
				break;
			}
			case BotCommands::cmdBrake:
			{
				a_Painter.setBrush(brushBrake);
				a_Painter.setPen(penBrake);
				a_Painter.drawEllipse(bot->m_X - 10, bot->m_Y - 10, 20, 20);
				break;
			}
			case BotCommands::cmdSteer:
			{
				a_Painter.setPen(penSteer);
				double angle = (bot->m_Angle + c.m_Param) / 180 * 3.1415926;
				a_Painter.drawLine(bot->m_X, bot->m_Y, bot->m_X + 400 * cos(angle), bot->m_Y + 400 * sin(angle));
				break;
			}
			case BotCommands::cmdUnknown:
			{
				a_Painter.setBrush(brushUnknown);
				a_Painter.drawEllipse(bot->m_X - 10, bot->m_Y - 10, 20, 20);
			}
		}
	}  // for c - botCommands[]
}




