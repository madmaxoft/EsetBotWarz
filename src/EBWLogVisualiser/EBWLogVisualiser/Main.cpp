
// Main.cpp

// Implements the main app entrypoint





#include "MainWindow.h"
#include <QApplication>





int main(int argc, char * argv[])
{
	QApplication a(argc, argv);
	MainWindow w;

	if (argc == 2)
	{
		w.loadFile(argv[1]);
	}

	w.show();

	return a.exec();
}




