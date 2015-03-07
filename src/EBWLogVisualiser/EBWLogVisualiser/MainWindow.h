
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





class MainWindow:
	public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget * parent = nullptr);
	~MainWindow();

private slots:
	void on_actFileOpen_triggered();

	void on_actExit_triggered();

	void on_gameList_itemSelectionChanged();

private:
	Ui::MainWindow * ui;

	LogFilePtr m_LogFile;
};




