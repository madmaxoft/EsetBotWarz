
// BotWarzApp.cpp

// Implements the BotWarzApp class representing the entire application

#include "Globals.h"
#include "BotWarzApp.h"
#include <fstream>
#include <iostream>
#include "Controller.h"
#include "LuaController.h"
#include "json/json.h"





BotWarzApp::BotWarzApp(const AString a_LoginToken, const AString & a_LoginNick):
	m_Board(*this),
	m_Comm(*this),
	m_LoginToken(a_LoginToken),
	m_LoginNick(a_LoginNick)
{
	LOG("Login nick: %s", a_LoginNick.c_str());
}





int BotWarzApp::run(bool a_ShouldLogComm, bool a_ShouldShowComm, const AString & a_ControllerFileName, bool a_ShouldDebugZBS)
{
	// Initialize the Lua controller:
	m_Controller = createLuaController(*this, a_ControllerFileName, a_ShouldDebugZBS);
	if (!m_Controller->isValid())
	{
		LOGERROR("Controller init failed, aborting.");
		return 2;
	}

	// DEBUG: test the controller by processing a dummy game:
	Json::Value gameData, updateData1, updateData2, finishData;
	std::ifstream("gameData.json") >> gameData;
	std::ifstream("updateData1.json") >> updateData1;
	std::ifstream("updateData2.json") >> updateData2;
	std::ifstream("finishData.json") >> finishData;
	startGame(gameData["game"]);
	updateBoard(updateData1["play"]);
	updateBoard(updateData2["play"]);
	finishGame(finishData["result"]);

	LOGERROR("Temporary termination.");
	return 1;

	// Initialize the server communication interface:
	if (!m_Comm.init(a_ShouldLogComm, a_ShouldShowComm))
	{
		LOGERROR("Comm::init() failed.");
		return 1;
	}

	// Wait for the termination request:
	m_evtTerminate.Wait();

	// Stop everything:
	m_Comm.stop();

	return 0;
}





void BotWarzApp::terminate(void)
{
	m_evtTerminate.Set();
}





void BotWarzApp::startGame(const Json::Value & a_GameData)
{
	m_Board.initialize(a_GameData);

	// Send the message to m_Controller, but take care of multithreading / reloading:
	auto controller = m_Controller;
	if (controller != nullptr)
	{
		controller->onGameStarted(m_Board);
	}
}





void BotWarzApp::updateBoard(const Json::Value & a_GameData)
{
	m_Board.updateFromJson(a_GameData);

	// Send the message to m_Controller, but take care of multithreading / reloading:
	auto controller = m_Controller;
	if (controller != nullptr)
	{
		controller->onGameUpdate();
	}
}





void BotWarzApp::finishGame(const Json::Value & a_ResultData)
{
	// Send the message to m_Controller, but take care of multithreading / reloading:
	auto controller = m_Controller;
	if (controller != nullptr)
	{
		controller->onGameFinished();
	}
}





void BotWarzApp::botDied(const Bot & a_Bot)
{
	// Send the message to m_Controller, but take care of multithreading / reloading:
	auto controller = m_Controller;
	if (controller != nullptr)
	{
		controller->onBotDied(a_Bot);
	}
}




