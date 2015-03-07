
// TimelineControl.cpp

// Implements the TimelineControl class representing a widget for displaying the game timeline

#include "TimelineControl.h"
#include <QPainter>
#include <QPaintEvent>
#include "Game.h"





TimelineControl::TimelineControl(QWidget * parent):
	Super(parent),
	m_CurrentTime(0),
	m_TotalTime(0),
	m_Width(0)
{
}





void TimelineControl::setGame(GamePtr a_Game)
{
	m_Game = a_Game;
	a_Game->getGameStateTimestamps(m_GameStateTimestamps);
	a_Game->getBotCommandTimestamps(m_BotCommandTimestamps);
	m_TotalTime = a_Game->getTotalTime();
	m_GameStartTime = a_Game->getGameStartTime();
	if (m_CurrentTime > m_TotalTime)
	{
		m_CurrentTime = m_TotalTime;
	}
	update();
}





void TimelineControl::paintEvent(QPaintEvent * a_Event)
{
	QPainter p(this);
	auto game = m_Game;
	if (game == nullptr)
	{
		p.drawText(a_Event->rect(), Qt::AlignCenter, "No game selected");
	}
	else
	{
		// Draw all events in the game:
		p.fillRect(a_Event->rect(), QColor(255, 255, 255));
		int hei = a_Event->rect().height();
		p.setPen(QColor(128, 128, 255));
		for (auto ts: m_GameStateTimestamps)
		{
			int pos = relClientTimeToPos(ts - m_GameStartTime);
			p.drawLine(pos, 0, pos, hei);
		}
		p.setPen(QColor(128, 255, 128));
		for (auto ts: m_BotCommandTimestamps)
		{
			int pos = relClientTimeToPos(ts - m_GameStartTime);
			p.drawLine(pos, 0, pos, hei);
		}

		// Draw the current selection:
		int pos = relClientTimeToPos(m_CurrentTime);
		p.setPen(QColor(255, 0, 0));
		p.drawLine(pos, 0, pos, hei);
	}
	p.end();
}





void TimelineControl::resizeEvent(QResizeEvent * a_Event)
{
	m_Width = a_Event->size().width();
}





int TimelineControl::relClientTimeToPos(quint64 a_RelClientTime)
{
	return static_cast<int>(m_Width * a_RelClientTime / m_TotalTime);
}




