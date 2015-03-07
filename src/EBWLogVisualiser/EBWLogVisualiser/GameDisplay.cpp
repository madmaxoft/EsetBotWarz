
// GameDisplay.cpp

// Implements the GameDisplay class representing the widget displaying the game state

#include "GameDisplay.h"





GameDisplay::GameDisplay(QWidget * a_Parent):
	Super(a_Parent)
{
}




void GameDisplay::setGameState(GameStatePtr a_GameState)
{
	m_GameState = a_GameState;
	redraw();
}





void GameDisplay::redraw()
{
	// TODO
}





void GameDisplay::paintEvent(QPaintEvent * a_Event)
{
	// TODO
}




