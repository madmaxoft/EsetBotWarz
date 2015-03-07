
// GameDisplay.h

// Declares the GameDisplay class representing the widget displaying the game state

#pragma once

#include <memory>
#include <QWidget>





// fwd:
class GameState;
typedef std::shared_ptr<GameState> GameStatePtr;





class GameDisplay:
	public QWidget
{
	typedef QWidget Super;
	Q_OBJECT

public:
	explicit GameDisplay(QWidget * a_Parent = nullptr);

	/** Sets the game state that is displayed in the widget. */
	void setGameState(GameStatePtr a_Game);

public slots:
	/** Redraws the entire widget based on the current state. */
	void redraw();

protected:
	/** The game state being displayed. May be nullptr. */
	GameStatePtr m_GameState;


	/** Paints the entire widget. */
	virtual void paintEvent(QPaintEvent * a_Event) override;
};




