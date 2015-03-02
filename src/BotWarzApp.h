
// BotWarzApp.h

// Declares the BotWarzApp class representing the entire application





#pragma once





class BotWarzApp
{
public:
	BotWarzApp(void);

	/** Runs the entire application.
	If a_ShouldLogComm is true, all the communication with the server is logged into a file.
	Returns the value that the process should return to the OS upon its exit. */
	int run(bool a_ShouldLogComm);
};




