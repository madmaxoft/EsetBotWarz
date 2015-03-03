
// BotWarzApp.cpp

// Implements the BotWarzApp class representing the entire application

#include "Globals.h"
#include "BotWarzApp.h"





BotWarzApp::BotWarzApp(const AString a_LoginToken, const AString & a_LoginNick):
	m_Board(*this),
	m_Comm(*this),
	m_LoginToken(a_LoginToken),
	m_LoginNick(a_LoginNick)
{
	LOG("Login nick: %s", a_LoginNick.c_str());
}





int BotWarzApp::run(bool a_ShouldLogComm, bool a_ShouldShowComm)
{
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
}





void BotWarzApp::updateBoard(const Json::Value & a_GameData)
{
	m_Board.updateFromJson(a_GameData);
}




