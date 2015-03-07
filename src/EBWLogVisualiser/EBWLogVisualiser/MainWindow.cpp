
// MainWindow.cpp

// Implements the MainWindow class representing the main app window

#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "LogFile.h"
#include "Game.h"
#include "GameState.h"





MainWindow::MainWindow(QWidget * parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->showMaximized();
}





MainWindow::~MainWindow()
{
	delete ui;
}





void MainWindow::loadFile(const QString & a_FileName)
{
	auto logFile = std::make_shared<LogFile>();
	auto err = logFile->readFile(a_FileName);
	if (!err.isEmpty())
	{
		QMessageBox::warning(this, tr("Error"), err);
		return;
	}
	m_LogFile = logFile;

	// Set the games to the games menu:
	ui->menu_Games->clear();
	auto actGroup = new QActionGroup(this);
	int idx = 0;
	for (auto & game: logFile->getGames())
	{
		auto act = new QAction(game->getLabel(), nullptr);
		connect(act, SIGNAL(triggered()), this, SLOT(onGameItemTriggered()));
		act->setCheckable(true);
		act->setActionGroup(actGroup);
		act->setProperty("gameIndex", idx++);
		if (idx <= 9)
		{
			act->setShortcut(QKeySequence(QString("Ctrl+%1").arg(idx)));
		}
		ui->menu_Games->addAction(act);
		if (idx == 1)
		{
			act->setChecked(true);
			act->trigger();
		}
	}
	ui->menu_Games->setEnabled(!logFile->getGames().empty());
}





void MainWindow::on_actFileOpen_triggered()
{
	QString fnam = QFileDialog::getOpenFileName(this, tr("Open log file"), QString(), tr("EBW log files (*.ebwlog)"));
	if (fnam.isEmpty())
	{
		return;
	}
	loadFile(fnam);
}





void MainWindow::on_actExit_triggered()
{
	close();
}





void MainWindow::on_gameTimeline_currentTimeChanged(quint64 a_CurrentTime)
{
	// Display the time information in the status bar:
	auto gameState = m_CurrentGame->getGameStateAt(a_CurrentTime);
	ui->gameDisplay->setGameState(gameState);
	auto botCommands = m_CurrentGame->getBotCommandsAt(a_CurrentTime);
	ui->gameDisplay->setBotCommands(botCommands);
	auto gameTime = gameState->m_ServerTime;
	ui->statusBar->showMessage(QString("Current time: %1, game time: %2")
		.arg(static_cast<double>(a_CurrentTime) / 1000)
		.arg(gameTime)
	);

	// Display the text logs:
	ui->textLog->clear();
	ui->textLog->addItems(gameState->m_AILogs);
}





void MainWindow::onGameItemTriggered(void)
{
	// Determine which game item it was:
	auto idx = sender()->property("gameIndex").toInt();
	m_CurrentGame = m_LogFile->getGames()[idx];
	ui->gameTimeline->setGame(m_CurrentGame);
}




