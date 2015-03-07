
// TimelineControl.h

// Declares the TimelineControl class representing a widget for displaying the game timeline

#pragma once

#include <memory>
#include <QWidget>





// fwd:
class Game;
typedef std::shared_ptr<Game> GamePtr;





class TimelineControl:
	public QWidget
{
	typedef QWidget Super;
	Q_OBJECT
public:
	explicit TimelineControl(QWidget * parent = nullptr);

	/** Sets a new game to display. */
	void setGame(GamePtr a_Game);

signals:
	/** The current time has been changed. */
	void currentTimeChanged(quint64 a_CurrentTime);

public slots:

protected:
	/** The game being displayed. */
	GamePtr m_Game;

	/** Absolute timestamps of the gamestates on the game timeline. */
	std::vector<quint64> m_GameStateTimestamps;

	/** Absolute timestamps of the botcommands on the game timeline. */
	std::vector<quint64> m_BotCommandTimestamps;

	/** Current position on the timeline (relative client time). */
	quint64 m_CurrentTime;

	/** Client timestamp of the game start event. */
	quint64 m_GameStartTime;

	/** Total game time (client). */
	quint64 m_TotalTime;

	/** Width of the control. */
	int m_Width;


	/** Paints the entire widget */
	virtual void paintEvent(QPaintEvent *) override;

	/** Called when the widget is resized */
	virtual void resizeEvent(QResizeEvent *) override;

	/** Called when the user presses any mouse button. */
	virtual void mousePressEvent(QMouseEvent * a_Event) override;

	/** Called when the user moves the mouse. */
	virtual void mouseMoveEvent(QMouseEvent * a_Event) override;

	QSize minimumSizeHint() const
	{
		return QSize(100, 10);
	}

	QSize maximumSizeHint() const
	{
		return QSize(100000, 30);
	}

	/** Converts relative client time to the horizontal position on the timeline. */
	int relClientTimeToPos(quint64 a_RelClientTime);

	/** Converts horizontal position on the timeline to relative client time. */
	quint64 posToRelClientTime(int a_Pos);

	/** Sets the specified current time and calls the signals. */
	void setCurrentTime(quint64 a_CurrentTime);
};





