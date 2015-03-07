
// GameDisplay.h

// Declares the GameDisplay class representing the widget displaying the game state

#pragma once

#include <memory>
#include <QWidget>





// fwd:
class GameState;
typedef std::shared_ptr<GameState> GameStatePtr;
class BotCommands;
typedef std::shared_ptr<BotCommands> BotCommandsPtr;





class GameDisplay:
	public QWidget
{
	typedef QWidget Super;
	Q_OBJECT

public:
	explicit GameDisplay(QWidget * a_Parent = nullptr);

	/** Sets the game state that is displayed in the widget. */
	void setGameState(GameStatePtr a_State);

	/** Sets the bot commands that are displayed in the widget. */
	void setBotCommands(BotCommandsPtr a_Cmds);

public slots:
	/** Redraws the entire widget based on the current state. */
	void redraw();

protected:
	/** The game state being displayed. May be nullptr. */
	GameStatePtr m_GameState;

	/** The bot commands being displayed. May be nullptr. */
	BotCommandsPtr m_BotCommands;


	/** Paints the entire widget. */
	virtual void paintEvent(QPaintEvent * a_Event) override;

	/** Paints the bot commands using the specified painter. */
	void paintBotCommands(QPainter & a_Painter);

	QSize minimumSizeHint() const
	{
		return QSize(900, 600);
	}

	QSize maximumSizeHint() const
	{
		return QSize(900, 600);
	}

};




