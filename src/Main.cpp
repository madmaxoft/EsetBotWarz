
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "lib/Network/NetworkSingleton.h"
#include "BotWarzApp.h"





////////////////////////////////////////////////////////////////////////////////
// main:

int main(int argc, char ** argv)
{
	// Initialize LibEvent:
	cNetworkSingleton::Get();

	// Process the command line:
	bool shouldLogComm = false;
	for (int i = 0; i < argc; i++)
	{
		AString Arg(argv[i]);
		if (NoCaseCompare(Arg, "/commlog") == 0)
		{
			shouldLogComm = true;
		}
		else if (NoCaseCompare(Arg, "nooutbuf") == 0)
		{
			setvbuf(stdout, nullptr, _IONBF, 0);
		}
	}  // for i - argv[]

	// Run the app:
	BotWarzApp app;
	int res = app.run(shouldLogComm);

	// Shutdown all of LibEvent:
	cNetworkSingleton::Get().Terminate();

	return res;
}




