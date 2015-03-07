#-------------------------------------------------
#
# Project created by QtCreator 2015-03-06T11:06:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EBWLogVisualiser
TEMPLATE = app


SOURCES +=\
	MainWindow.cpp \
	Main.cpp \
	GameDisplay.cpp \
	Bot.cpp \
	LogFile.cpp \
	TimelineControl.cpp \
    Game.cpp \
    GameState.cpp \
    BotCommands.cpp

HEADERS  +=\
	MainWindow.h \
	GameDisplay.h \
	GameState.h \
	Game.h \
	LogFile.h \
	Bot.h \
	TimelineControl.h \
    BotCommands.h

FORMS    += MainWindow.ui
