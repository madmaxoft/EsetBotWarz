
// LuaController.cpp

// Implements the LuaController class representing the AI controller implemented in Lua

#include "Globals.h"
#include "json/value.h"
#include "lib/Network/CriticalSection.h"
#include "Controller.h"
#include "LuaState.h"
#include "Board.h"
#include "BotWarzApp.h"





static const char LUA_GLOBAL_LUACONTROLLER_FIELD_NAME[] = "__EsetBotWarz_luaController";





class LuaController :
	public Controller
{
	typedef Controller Super;

public:
	LuaController(BotWarzApp & a_App, const AString & a_FileName, bool a_ShouldDebugZBS):
		Super(a_App),
		m_LuaState(Printf("LuaController: %s", a_FileName.c_str()))
	{
		m_LuaState.create();
		lua_atpanic(m_LuaState, luaPanic);
		if (a_ShouldDebugZBS)
		{
			m_LuaState.execCode("require([[mobdebug]]).start()");
		}
		m_IsValid = m_LuaState.loadFile(a_FileName);
	}





	/** Called upon startup to query whether the controller has initialized properly; the app will terminate if not. */
	virtual bool isValid(void) const override
	{
		return m_IsValid;
	}





	/** Called when the game has just started.
	a_Board points to the game board that represents the game state.
	The board needs to stay valid until the game is finished via the onGameFinished() call. */
	virtual void onGameStarted(Board & a_Board) override
	{
		// Create a table representing the board in the Lua state
		cCSLock Lock(m_CSLuaState);
		if (m_LuaState == nullptr)
		{
			return;
		}
		lua_newtable(m_LuaState);  // Stack: [GBT]
		m_GameBoardTable.refStack(m_LuaState, -1);  // Stack: [GBT]
		if (!m_GameBoardTable.isValid())
		{
			LOGWARNING("%s: Cannot create gameboard table reference.", __FUNCTION__);
			return;
		}

		// Store the board:
		m_Board = &a_Board;

		// Fill the table with members:
		createSpeedLevelsTable();
		createWorldTable();
		createEmptySubTable("botCommands");
		createAllBotTable();
		createAPIFunctions();
		updateGameBoardTime();

		m_LuaState.call("onGameStarted", &m_GameBoardTable);
	}





	/** Called when a game update has been received. */
	virtual void onGameUpdate(void) override
	{
		// Update the bots in the board representation table:
		cCSLock Lock(m_CSLuaState);
		updateGameBoardTime();
		lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, m_GameBoardTable);  // Stack: [GBT]
		lua_getfield(m_LuaState, -1, "allBots");                       // Stack: [GBT] [allBots]
		auto bots = m_Board->getAllBotsCopy();
		for (auto & bot: bots)
		{
			auto & b = *(bot.second);
			lua_rawgeti(m_LuaState, -1, b.m_ID);    // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_X);      // Stack: [GBT] [allBots] [bot] [x]
			lua_setfield(m_LuaState, -2, "x");      // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_Y);      // Stack: [GBT] [allBots] [bot] [y]
			lua_setfield(m_LuaState, -2, "y");      // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_Angle);  // Stack: [GBT] [allBots] [bot] [angle]
			lua_setfield(m_LuaState, -2, "angle");  // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_Speed);  // Stack: [GBT] [allBots] [bot] [speed]
			lua_setfield(m_LuaState, -2, "speed");  // Stack: [GBT] [allBots] [bot]
			lua_pop(m_LuaState, 1);                 // Stack: [GBT] [allBots]
		}  // for bot - bots[]
		lua_pop(m_LuaState, 2);

		m_LuaState.call("onGameUpdate", &m_GameBoardTable);
	}






	/** Called when the current game has finished.
	The board that has represented this game can be released after this call returns. */
	virtual void onGameFinished(void) override
	{
		cCSLock Lock(m_CSLuaState);
		updateGameBoardTime();
		m_LuaState.call("onGameFinished", &m_GameBoardTable);
		m_GameBoardTable.unRef();
	}






	virtual void onBotDied(const Bot & a_Bot) override
	{
		// Call the callback:
		cCSLock Lock(m_CSLuaState);
		updateGameBoardTime();
		m_LuaState.call("onBotDied", &m_GameBoardTable, a_Bot.m_ID);

		// Remove the bot from the Lua tables:
		lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, m_GameBoardTable);  // Stack: [GBT]
		lua_getfield(m_LuaState, -1, "allBots");                       // Stack: [GBT] [allBots]
		lua_pushnil(m_LuaState);                                       // Stack: [GBT] [allBots] [nil]
		lua_rawseti(m_LuaState, -2, a_Bot.m_ID);                       // Stack: [GBT] [allBots]
		lua_pop(m_LuaState, 2);
	}





	/** Returns the current set of commands for the bots that should be sent to the server.
	Also clears the commands, so that they aren't sent the next time this is called. */
	virtual Json::Value getBotCommands(void) override
	{
		Json::Value res(Json::arrayValue);

		// Get the bots before locking the Lua State (to avoid deadlocks):
		auto myBots = m_Board->getMyBotsCopy();

		// Check that the Lua state is valid:
		cCSLock Lock(m_CSLuaState);
		if (!m_GameBoardTable.isValid())
		{
			return res;
		}
		updateGameBoardTime();

		// Call the pre-getCommands callback:
		m_LuaState.call("onSendingCommands", &m_GameBoardTable);

		// Get the botCommands table:
		lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, m_GameBoardTable);
		lua_getfield(m_LuaState, -1, "botCommands");
		if (lua_isnil(m_LuaState, -1))
		{
			LOGWARNING("The botCommands table is not present in the Lua game state. Returning no commands.");
			return res;
		}

		// For each of my currently alive bots, get its command:
		for (auto & bot : myBots)
		{
			lua_rawgeti(m_LuaState, -1, bot->m_ID);  // Stack: [GBT] [botCommands] [bot]
			if (!lua_istable(m_LuaState, -1))
			{
				// The entry isn't a table, nothing to query
				lua_pop(m_LuaState, 1);
				continue;
			}
			lua_getfield(m_LuaState, -1, "cmd");     // Stack: [GBT] [botCommands] [bot] [.cmd]
			AString cmd;
			m_LuaState.getStackValue(-1, cmd);
			Json::Value val;
			val["cmd"] = cmd;
			int toPop = 2;
			if (cmd == "steer")
			{
				lua_Number angle;
				lua_getfield(m_LuaState, -2, "angle");  // Stack: [GBT] [botCommands] [bot] [.cmd] [.angle]
				m_LuaState.getStackValue(-1, angle);
				val["angle"] = angle;
				toPop = 3;
			}
			lua_pop(m_LuaState, toPop);              // Stack: [GBT] [botCommands]
			val["id"] = bot->m_ID;
			res.append(val);

			// Clear the command:
			lua_pushnil(m_LuaState);                 // Stack: [GBT] [botCommands] [nil]
			lua_rawseti(m_LuaState, -2, bot->m_ID);  // Stack: [GBT] [botCommands]
		}  // for bot - myBots[]
		lua_pop(m_LuaState, 2);

		// Let the Lua script know that we've sent the commands:
		m_LuaState.call("onCommandsSent", &m_GameBoardTable);

		return res;
	}




protected:
	/** The Lua engine used for the AI.
	Protected against multithreaded access by m_CSLuaState. */
	LuaState m_LuaState;

	/** Set to true if the script file has loaded successfully. */
	bool m_IsValid;

	/** The reference to the game board table in the Lua state.
	Only accessible when m_LuaState is valid (and m_CSLuaState held). */
	LuaState::Ref m_GameBoardTable;

	/** The board that represents the game state. */
	Board * m_Board;

	/** Protects m_BotCommands against multithreaded access. */
	cCriticalSection m_CSLuaState;





	/** Creates the speedLevels table and stores it in the GameBoard table in m_LuaState.
	Assumes that the GBT is at the top of the Lua stack, and leaves it there. */
	void createSpeedLevelsTable(void)
	{
		ASSERT(m_CSLuaState.IsLockedByCurrentThread());

		// Create the SpeedLevels table:
		lua_newtable(m_LuaState);                     // Stack: [GBT] [SLT]

		// Fill it with the actual speed levels:
		auto speedLevels = m_Board->getSpeedLevels();
		int idx = 1;
		for (auto & sl: speedLevels)
		{
			lua_newtable(m_LuaState);                          // Stack: [GBT] [SLT] [SL]
			lua_pushnumber(m_LuaState, sl.m_LinearSpeed);      // Stack: [GBT] [SLT] [SL] [LinearSpeed]
			lua_setfield(m_LuaState, -2, "linearSpeed");       // Stack: [GBT] [SLT] [SL]
			lua_pushnumber(m_LuaState, sl.m_MaxAngularSpeed);  // Stack: [GBT] [SLT] [SL] [MaxAngularSpeed]
			lua_setfield(m_LuaState, -2, "maxAngularSpeed");   // Stack: [GBT] [SLT] [SL]
			lua_rawseti(m_LuaState, -2, idx++);                // Stack: [GBT] [SLT]
		}

		// Store the SpeedLevels table in the GameBoard table:
		lua_setfield(m_LuaState, -2, "speedLevels");  // Stack: [GBT]
	}





	/** Stores the world data - dimensions - in the GameBoard table in m_LuaState.
	Assumes that the GBT is at the top of the Lua stack, and leaves it there. */
	void createWorldTable(void)
	{
		ASSERT(m_CSLuaState.IsLockedByCurrentThread());

		// Create the World table:
		lua_newtable(m_LuaState);                     // Stack: [GBT] [WT]

		// Fill it with the actual dimensions:
		lua_pushnumber(m_LuaState, m_Board->getWorldWidth());   // Stack: [GBT] [WT] [Width]
		lua_setfield(m_LuaState, -2, "width");                  // Stack: [GBT] [WT]
		lua_pushnumber(m_LuaState, m_Board->getWorldHeight());  // Stack: [GBT] [WT] [Height]
		lua_setfield(m_LuaState, -2, "height");                 // Stack: [GBT] [WT]
		lua_pushnumber(m_LuaState, m_Board->getBotRadius());    // Stack: [GBT] [WT] [Radius]
		lua_setfield(m_LuaState, -2, "botRadius");              // Stack: [GBT] [WT]

		// Store the World table in GameBoard table:
		lua_setfield(m_LuaState, -2, "world");  // Stack: [GBT]
	}




	/** Creates an empty table and sets it as the named subtable of the GameBoard table in m_LuaState.
	Assumes that the GBT is at the top of the Lua stack, and leaves it there. */
	void createEmptySubTable(const char * a_SubTableName)
	{
		lua_newtable(m_LuaState);                      // Stack: [GBT] [new table]
		lua_setfield(m_LuaState, -2, a_SubTableName);  // Stack: [GBT]
	}




	/** Stores all the bots in an "allBots" table inside the GBT.
	Assumes that the GBT is at the top of the Lua stack, and leaves it there. */
	void createAllBotTable(void)
	{
		lua_newtable(m_LuaState);                    // Stack: [GBT] [allBots]
		auto allBots = m_Board->getAllBotsCopy();
		for (auto & bot: allBots)
		{
			auto & b = *(bot.second);
			lua_newtable(m_LuaState);                  // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_ID);        // Stack: [GBT] [allBots] [bot] [id]
			lua_setfield(m_LuaState, -2, "id");        // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_X);         // Stack: [GBT] [allBots] [bot] [x]
			lua_setfield(m_LuaState, -2, "x");         // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_Y);         // Stack: [GBT] [allBots] [bot] [y]
			lua_setfield(m_LuaState, -2, "y");         // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_Speed);     // Stack: [GBT] [allBots] [bot] [speed]
			lua_setfield(m_LuaState, -2, "speed");     // Stack: [GBT] [allBots] [bot]
			lua_pushnumber(m_LuaState, b.m_Angle);     // Stack: [GBT] [allBots] [bot] [angle]
			lua_setfield(m_LuaState, -2, "angle");     // Stack: [GBT] [allBots] [bot]
			lua_pushboolean(m_LuaState, b.m_IsEnemy);  // Stack: [GBT] [allBots] [bot] [isEnemy]
			lua_setfield(m_LuaState, -2, "isEnemy");   // Stack: [GBT] [allBots] [bot]
			lua_rawseti(m_LuaState, -2, b.m_ID);       // Stack: [GBT] [allBots]
		}  // for bot - allBots[]
		lua_setfield(m_LuaState, -2, "allBots");
	}




	static int luaPanic(lua_State * a_LuaState)
	{
		LOGERROR("*** LUA PANIC ***");
		LuaState L(a_LuaState);
		AString panicString;
		L.getStackValue(-1, panicString);
		LOGERROR("%s", panicString.c_str());
		L.logStackTrace();
		ASSERT(!"LUA PANIC");
		return 1;
	}





	/** OBSOLETE binding for the commLog() function. Convert into commentLog. */
	static int commLog(lua_State * a_LuaState)
	{
		// Warn about being obsolete:
		LuaState L(a_LuaState);
		LOGWARNING("%s: Function is obsolete, use commentLog instead", __FUNCTION__);
		L.logStackTrace();

		// Check the params:
		if (
			!L.checkParamString(1) ||
			!L.checkParamEnd(2)
		)
		{
			return 0;
		}

		// Get the luaController instance from the state:
		lua_getfield(a_LuaState, LUA_GLOBALSINDEX, LUA_GLOBAL_LUACONTROLLER_FIELD_NAME);
		if (!lua_islightuserdata(a_LuaState, -1))
		{
			LOGWARNING("%s: Cannot find my instance in the Lua state", __FUNCTION__);
			L.logStackTrace();
			return 0;
		}
		LuaController * luaController = reinterpret_cast<LuaController *>(lua_touserdata(a_LuaState, -1));
		lua_pop(a_LuaState, 1);

		// Log:
		AString msg;
		L.getStackValue(1, msg);
		luaController->m_App.commentLog(msg);
		return 0;
	}





	/** Binding for the commentLog() function. */
	static int commentLog(lua_State * a_LuaState)
	{
		// Check the params:
		LuaState L(a_LuaState);
		if (
			!L.checkParamString(1) ||
			!L.checkParamEnd(2)
		)
		{
			return 0;
		}

		// Get the luaController instance from the state:
		lua_getfield(a_LuaState, LUA_GLOBALSINDEX, LUA_GLOBAL_LUACONTROLLER_FIELD_NAME);
		if (!lua_islightuserdata(a_LuaState, -1))
		{
			LOGWARNING("%s: Cannot find my instance in the Lua state", __FUNCTION__);
			L.logStackTrace();
			return 0;
		}
		LuaController * luaController = reinterpret_cast<LuaController *>(lua_touserdata(a_LuaState, -1));
		lua_pop(a_LuaState, 1);

		// Log:
		AString msg;
		L.getStackValue(1, msg);
		luaController->m_App.commentLog(msg);
		return 0;
	}





	/** Binding for the aiLog() function. */
	static int aiLog(lua_State * a_LuaState)
	{
		// Check the params:
		LuaState L(a_LuaState);
		if (
			!L.checkParamNumber(1) ||
			!L.checkParamString(2) ||
			!L.checkParamEnd(3)
		)
		{
			return 0;
		}

		// Get the luaController instance from the state:
		lua_getfield(a_LuaState, LUA_GLOBALSINDEX, LUA_GLOBAL_LUACONTROLLER_FIELD_NAME);
		if (!lua_islightuserdata(a_LuaState, -1))
		{
			LOGWARNING("%s: Cannot find my instance in the Lua state", __FUNCTION__);
			L.logStackTrace();
			return 0;
		}
		LuaController * luaController = reinterpret_cast<LuaController *>(lua_touserdata(a_LuaState, -1));
		lua_pop(a_LuaState, 1);

		// Log:
		int botID = 0;
		AString msg;
		L.getStackValues(1, botID, msg);
		luaController->m_App.aiLog(botID, msg);
		return 0;
	}





	void createAPIFunctions(void)
	{
		// Push the luaController instance into the state:
		lua_pushlightuserdata(m_LuaState, this);
		lua_setfield(m_LuaState, LUA_GLOBALSINDEX, LUA_GLOBAL_LUACONTROLLER_FIELD_NAME);

		// Push the API functions:
		lua_pushcfunction(m_LuaState, &commentLog);
		lua_setfield(m_LuaState, LUA_GLOBALSINDEX, "commentLog");
		lua_pushcfunction(m_LuaState, &commLog);
		lua_setfield(m_LuaState, LUA_GLOBALSINDEX, "commLog");  // OBSOLETE, but still available in the API
		lua_pushcfunction(m_LuaState, &aiLog);
		lua_setfield(m_LuaState, LUA_GLOBALSINDEX, "aiLog");
	}





	/** Updates the local and server time stored in the GameBoard table. */
	void updateGameBoardTime(void)
	{
		ASSERT(m_CSLuaState.IsLockedByCurrentThread());

		lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, m_GameBoardTable);
		lua_pushnumber(m_LuaState, m_Board->getServerTime());
		lua_setfield(m_LuaState, -2, "serverTime");
		auto localTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_Board->getLocalGameStartTime()).count();
		lua_pushnumber(m_LuaState, static_cast<lua_Number>(localTime));
		lua_setfield(m_LuaState, -2, "localTime");
		lua_pop(m_LuaState, 1);
	}
};






SharedPtr<Controller> createLuaController(BotWarzApp & a_App, const AString & a_FileName, bool a_ShouldDebugZBS)
{
	return std::make_shared<LuaController>(a_App, a_FileName, a_ShouldDebugZBS);
}




