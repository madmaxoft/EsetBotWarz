
// MainWindow.cpp

// Implements the MainWindow class representing the main app window

#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "LogFile.h"
#include "Game.h"





MainWindow::MainWindow(QWidget * parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
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

	// Set the games to the game list:
	ui->gameList->clear();
	int idx = 0;
	for (auto & game: logFile->getGames())
	{
		auto item = new QListWidgetItem(game->getPlayer1Name() + " vs. " + game->getPlayer2Name());
		item->setData(Qt::UserRole, idx++);
		ui->gameList->addItem(item);
	}

	m_LogFile = logFile;

	// Select the first game:
	if (ui->gameList->count() > 0)
	{
		ui->gameList->item(0)->setSelected(true);
	}
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





void MainWindow::on_gameList_itemSelectionChanged()
{
	auto sel = ui->gameList->selectedItems();
	if (sel.isEmpty())
	{
		return;
	}
	m_CurrentGame = m_LogFile->getGames()[sel.front()->data(Qt::UserRole).toInt()];
	ui->gameTimeline->setGame(m_CurrentGame);
}





void MainWindow::on_gameTimeline_currentTimeChanged(quint64 a_CurrentTime)
{
	auto gameState = m_CurrentGame->getGameStateAt(a_CurrentTime);
	ui->gameDisplay->setGameState(gameState);
	auto botCommands = m_CurrentGame->getBotCommandsAt(a_CurrentTime);
	ui->gameDisplay->setBotCommands(botCommands);
}




