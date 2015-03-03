
// BotWarzApp.h

// Declares the BotWarzApp class representing the entire application





#pragma once

#include "Comm.h"





class BotWarzApp
{
public:
	BotWarzApp(const AString a_LoginToken, const AString & a_LoginNick);

	/** Runs the entire application.
	If a_ShouldLogComm is true, all the communication with the server is logged into a file.
	If a_ShouldShowComm is true, all the communication with the server is output to stdout.
	Returns the value that the process should return to the OS upon its exit. */
	int run(bool a_ShouldLogComm, bool a_ShouldShowComm);

protected:
	/** The communication interface to the server. */
	Comm m_Comm;
};




