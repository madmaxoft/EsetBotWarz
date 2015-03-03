
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include <fstream>
#include "lib/Network/NetworkSingleton.h"
#include "BotWarzApp.h"





/** Initializes and runs the entire app.
Returns the value that the process should return upon exit. */
int run(int argc, char ** argv)
{
	// Process the command line:
	bool shouldLogComm = false;
	bool shouldShowComm = false;
	for (int i = 0; i < argc; i++)
	{
		AString Arg(argv[i]);
		if (NoCaseCompare(Arg, "/logcomm") == 0)
		{
			shouldLogComm = true;
		}
		else if (NoCaseCompare(Arg, "/showcomm") == 0)
		{
			shouldShowComm = true;
		}
		else if (NoCaseCompare(Arg, "nooutbuf") == 0)
		{
			setvbuf(stdout, nullptr, _IONBF, 0);
		}
	}  // for i - argv[]

	// Read the login information from a file:
	AString loginToken;
	AString loginNick;
	{
		std::ifstream loginFile("login.txt");
		loginFile >> loginToken;
		loginFile >> loginNick;
	}
	if (loginToken.empty())
	{
		LOGERROR("The login token is empty, please provide a valid token in file login.txt");
		return 1;
	}
	if (loginNick.empty())
	{
		LOGERROR("The login nick is empty, please provide a valid nick in file login.txt");
		return 1;
	}

	// Run the app:
	BotWarzApp app(loginToken, loginNick);
	return app.run(shouldLogComm, shouldShowComm);
}





////////////////////////////////////////////////////////////////////////////////
// main:

int main(int argc, char ** argv)
{
	// Initialize LibEvent:
	cNetworkSingleton::Get();

	int res = run(argc, argv);

	// Shutdown all of LibEvent:
	cNetworkSingleton::Get().Terminate();

	return res;
}




