
// MainWindow.h

// Declares the MainWindow class representing the main app window

#pragma once

#include <memory>
#include <QMainWindow>





// fwd:
namespace Ui {
	class MainWindow;
}
class LogFile;
typedef std::shared_ptr<LogFile> LogFilePtr;
class Game;
typedef std::shared_ptr<Game> GamePtr;





class MainWindow:
	public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget * parent = nullptr);
	~MainWindow();

	/** Loads the game data from the specified file. */
	void loadFile(const QString & a_FileName);

private slots:
	void on_actFileOpen_triggered();

	void on_actExit_triggered();

	void on_gameList_itemSelectionChanged();

	void on_gameTimeline_currentTimeChanged(quint64 a_CurrentTime);

private:
	Ui::MainWindow * ui;

	LogFilePtr m_LogFile;
	GamePtr m_CurrentGame;
};




