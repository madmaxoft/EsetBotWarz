
// LuaState.cpp

// Implements the LuaState class representing the wrapper over lua_State *, provides associated helper functions

#include "Globals.h"
#include "LuaState.h"

extern "C"
{
	#include "lib/lua/src/lualib.h"
}

#undef TOLUA_TEMPLATE_BIND





const LuaState::Ret LuaState::Return = {};





////////////////////////////////////////////////////////////////////////////////
// LuaState:

LuaState::LuaState(const AString & a_SubsystemName) :
	m_LuaState(nullptr),
	m_IsOwned(false),
	m_SubsystemName(a_SubsystemName),
	m_NumCurrentFunctionArgs(-1)
{
}





LuaState::LuaState(lua_State * a_AttachState) :
	m_LuaState(a_AttachState),
	m_IsOwned(false),
	m_SubsystemName("<attached>"),
	m_NumCurrentFunctionArgs(-1)
{
}





LuaState::~LuaState()
{
	if (isValid())
	{
		if (m_IsOwned)
		{
			close();
		}
		else
		{
			detach();
		}
	}
}





void LuaState::create(void)
{
	if (m_LuaState != nullptr)
	{
		LOGWARNING("%s: Trying to create an already-existing LuaState, ignoring.", __FUNCTION__);
		return;
	}
	m_LuaState = lua_open();
	luaL_openlibs(m_LuaState);
	m_IsOwned = true;
}





void LuaState::close(void)
{
	if (m_LuaState == nullptr)
	{
		LOGWARNING("%s: Trying to close an invalid LuaState, ignoring.", __FUNCTION__);
		return;
	}
	if (!m_IsOwned)
	{
		LOGWARNING(
			"%s: Detected mis-use, calling close() on an attached state (0x%p). Detaching instead.",
			__FUNCTION__, m_LuaState
		);
		detach();
		return;
	}
	lua_close(m_LuaState);
	m_LuaState = nullptr;
	m_IsOwned = false;
}





void LuaState::attach(lua_State * a_State)
{
	if (m_LuaState != nullptr)
	{
		LOGINFO("%s: Already contains a LuaState (0x%p), will be closed / detached.", __FUNCTION__, m_LuaState);
		if (m_IsOwned)
		{
			close();
		}
		else
		{
			detach();
		}
	}
	m_LuaState = a_State;
	m_IsOwned = false;
}





void LuaState::detach(void)
{
	if (m_LuaState == nullptr)
	{
		return;
	}
	if (m_IsOwned)
	{
		LOGWARNING(
			"%s: Detected a mis-use, calling detach() when the state is owned. Closing the owned state (0x%p).",
			__FUNCTION__, m_LuaState
		);
		close();
		return;
	}
	m_LuaState = nullptr;
}





void LuaState::addPackagePath(const AString & a_PathVariable, const AString & a_Path)
{
	// Get the current path:
	lua_getfield(m_LuaState, LUA_GLOBALSINDEX, "package");                           // Stk: <package>
	lua_getfield(m_LuaState, -1, a_PathVariable.c_str());                            // Stk: <package> <package.path>
	size_t len = 0;
	const char * PackagePath = lua_tolstring(m_LuaState, -1, &len);
	
	// Append the new path:
	AString NewPackagePath(PackagePath, len);
	NewPackagePath.append(LUA_PATHSEP);
	NewPackagePath.append(a_Path);
	
	// Set the new path to the environment:
	lua_pop(m_LuaState, 1);                                                          // Stk: <package>
	lua_pushlstring(m_LuaState, NewPackagePath.c_str(), NewPackagePath.length());    // Stk: <package> <NewPackagePath>
	lua_setfield(m_LuaState, -2, a_PathVariable.c_str());                            // Stk: <package>
	lua_pop(m_LuaState, 1);
	lua_pop(m_LuaState, 1);                                                          // Stk: -
}





void LuaState::execCode(const char * a_LuaCode)
{
	ASSERT(isValid());

	luaL_dostring(m_LuaState, a_LuaCode);
}





bool LuaState::loadFile(const AString & a_FileName)
{
	ASSERT(isValid());
	
	// Load the file:
	int s = luaL_loadfile(m_LuaState, a_FileName.c_str());
	if (reportErrors(s))
	{
		LOGWARNING("Can't load %s because of an error in file %s", m_SubsystemName.c_str(), a_FileName.c_str());
		return false;
	}

	// Execute the globals:
	s = lua_pcall(m_LuaState, 0, LUA_MULTRET, 0);
	if (reportErrors(s))
	{
		LOGWARNING("Error in %s in file %s", m_SubsystemName.c_str(), a_FileName.c_str());
		return false;
	}
	
	return true;
}





bool LuaState::hasFunction(const char * a_FunctionName)
{
	if (!isValid())
	{
		// This happens if cPlugin::Initialize() fails with an error
		return false;
	}

	lua_getglobal(m_LuaState, a_FunctionName);
	bool res = (!lua_isnil(m_LuaState, -1) && lua_isfunction(m_LuaState, -1));
	lua_pop(m_LuaState, 1);
	return res;
}





bool LuaState::pushFunction(const char * a_FunctionName)
{
	ASSERT(m_NumCurrentFunctionArgs == -1);  // If not, there's already something pushed onto the stack

	if (!isValid())
	{
		// This happens if cPlugin::Initialize() fails with an error
		return false;
	}
	
	// push the error handler for lua_pcall()
	lua_pushcfunction(m_LuaState, &reportFnCallErrors);
	
	lua_getglobal(m_LuaState, a_FunctionName);
	if (!lua_isfunction(m_LuaState, -1))
	{
		LOGWARNING("Error in %s: Could not find function %s()", m_SubsystemName.c_str(), a_FunctionName);
		lua_pop(m_LuaState, 2);
		return false;
	}
	m_CurrentFunctionName.assign(a_FunctionName);
	m_NumCurrentFunctionArgs = 0;
	return true;
}





bool LuaState::pushFunction(int a_FnRef)
{
	ASSERT(isValid());
	ASSERT(m_NumCurrentFunctionArgs == -1);  // If not, there's already something pushed onto the stack
	
	// push the error handler for lua_pcall()
	lua_pushcfunction(m_LuaState, &reportFnCallErrors);
	
	lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, a_FnRef);  // same as lua_getref()
	if (!lua_isfunction(m_LuaState, -1))
	{
		lua_pop(m_LuaState, 2);
		return false;
	}
	m_CurrentFunctionName = "<callback>";
	m_NumCurrentFunctionArgs = 0;
	return true;
}





bool LuaState::pushFunction(const TableRef & a_TableRef)
{
	ASSERT(isValid());
	ASSERT(m_NumCurrentFunctionArgs == -1);  // If not, there's already something pushed onto the stack
	
	// push the error handler for lua_pcall()
	lua_pushcfunction(m_LuaState, &reportFnCallErrors);
	
	lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, a_TableRef.getTableRef());  // Get the table ref
	if (!lua_istable(m_LuaState, -1))
	{
		// Not a table, bail out
		lua_pop(m_LuaState, 2);
		return false;
	}
	lua_getfield(m_LuaState, -1, a_TableRef.getFnName());
	if (lua_isnil(m_LuaState, -1) || !lua_isfunction(m_LuaState, -1))
	{
		// Not a valid function, bail out
		lua_pop(m_LuaState, 3);
		return false;
	}
	
	// Pop the table off the stack:
	lua_remove(m_LuaState, -2);
	
	Printf(m_CurrentFunctionName, "<table-callback %s>", a_TableRef.getFnName());
	m_NumCurrentFunctionArgs = 0;
	return true;
}





void LuaState::push(const AString & a_String)
{
	ASSERT(isValid());

	lua_pushlstring(m_LuaState, a_String.data(), a_String.size());
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::push(const AStringVector & a_Vector)
{
	ASSERT(isValid());

	lua_createtable(m_LuaState, (int)a_Vector.size(), 0);
	int newTable = lua_gettop(m_LuaState);
	int index = 1;
	for (AStringVector::const_iterator itr = a_Vector.begin(), end = a_Vector.end(); itr != end; ++itr, ++index)
	{
		lua_pushlstring(m_LuaState, itr->c_str(), itr->size());
		lua_rawseti(m_LuaState, newTable, index);
	}
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::push(const char * a_Value)
{
	ASSERT(isValid());

	lua_pushstring(m_LuaState, a_Value);
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::push(bool a_Value)
{
	ASSERT(isValid());

	lua_pushboolean(m_LuaState, a_Value ? 1 : 0);
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::push(double a_Value)
{
	ASSERT(isValid());

	lua_pushnumber(m_LuaState, a_Value);
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::push(int a_Value)
{
	ASSERT(isValid());

	lua_pushnumber(m_LuaState, a_Value);
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::push(Ref * a_Value)
{
	ASSERT(isValid());
	ASSERT(a_Value->isValid());

	lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, a_Value->operator int());
	m_NumCurrentFunctionArgs += 1;
}





void LuaState::getStackValue(int a_StackPos, bool & a_ReturnedVal)
{
	if (lua_isboolean(m_LuaState, a_StackPos))
	{
		a_ReturnedVal = (lua_toboolean(m_LuaState, a_StackPos) > 0);
	}
}





void LuaState::getStackValue(int a_StackPos, AString & a_Value)
{
	size_t len = 0;
	const char * data = lua_tolstring(m_LuaState, a_StackPos, &len);
	if (data != nullptr)
	{
		a_Value.assign(data, len);
	}
}





void LuaState::getStackValue(int a_StackPos, int & a_ReturnedVal)
{
	if (lua_isnumber(m_LuaState, a_StackPos))
	{
		a_ReturnedVal = (int)lua_tonumber(m_LuaState, a_StackPos);
	}
}





void LuaState::getStackValue(int a_StackPos, double & a_ReturnedVal)
{
	if (lua_isnumber(m_LuaState, a_StackPos))
	{
		a_ReturnedVal = lua_tonumber(m_LuaState, a_StackPos);
	}
}





bool LuaState::callFunction(int a_NumResults)
{
	ASSERT (m_NumCurrentFunctionArgs >= 0);  // A function must be pushed to stack first
	ASSERT(lua_isfunction(m_LuaState, -m_NumCurrentFunctionArgs - 1));  // The function to call
	ASSERT(lua_isfunction(m_LuaState, -m_NumCurrentFunctionArgs - 2));  // The error handler
	
	// Save the current "stack" state and reset, in case the callback calls another function:
	AString CurrentFunctionName;
	std::swap(m_CurrentFunctionName, CurrentFunctionName);
	int NumArgs = m_NumCurrentFunctionArgs;
	m_NumCurrentFunctionArgs = -1;
	
	// Call the function:
	int s = lua_pcall(m_LuaState, NumArgs, a_NumResults, -NumArgs - 2);
	if (s != 0)
	{
		// The error has already been printed together with the stacktrace
		LOGWARNING("Error in %s calling function %s()", m_SubsystemName.c_str(), CurrentFunctionName.c_str());
		return false;
	}
	
	// Remove the error handler from the stack:
	lua_remove(m_LuaState, -a_NumResults - 1);
	
	return true;
}





bool LuaState::checkParamTable(int a_StartParam, int a_EndParam)
{
	ASSERT(isValid());
	
	if (a_EndParam < 0)
	{
		a_EndParam = a_StartParam;
	}
	
	for (int i = a_StartParam; i <= a_EndParam; i++)
	{
		if (lua_istable(m_LuaState, i))
		{
			continue;
		}
		// Not a table, report error:
		lua_Debug entry;
		VERIFY(lua_getstack(m_LuaState, 0,   &entry));
		VERIFY(lua_getinfo (m_LuaState, "n", &entry));
		LOG("Error in function '%s': expected a table as parameter #%d, got %s",
			(entry.name != nullptr) ? entry.name : "?", i, getTypeText(i).c_str()
		);
		return false;
	}  // for i - Param
	
	// All params checked ok
	return true;
}





bool LuaState::checkParamNumber(int a_StartParam, int a_EndParam)
{
	ASSERT(isValid());
	
	if (a_EndParam < 0)
	{
		a_EndParam = a_StartParam;
	}
	
	for (int i = a_StartParam; i <= a_EndParam; i++)
	{
		if (lua_isnumber(m_LuaState, i))
		{
			continue;
		}
		// Not a number, report error:
		lua_Debug entry;
		VERIFY(lua_getstack(m_LuaState, 0,   &entry));
		VERIFY(lua_getinfo (m_LuaState, "n", &entry));
		LOG("Error in function '%s': expected a number as parameter #%d, got %s",
			(entry.name != nullptr) ? entry.name : "?", i, getTypeText(i).c_str()
		);
		return false;
	}  // for i - Param
	
	// All params checked ok
	return true;
}





bool LuaState::checkParamString(int a_StartParam, int a_EndParam)
{
	ASSERT(isValid());
	
	if (a_EndParam < 0)
	{
		a_EndParam = a_StartParam;
	}
	
	for (int i = a_StartParam; i <= a_EndParam; i++)
	{
		if (lua_isstring(m_LuaState, i))
		{
			continue;
		}
		// Not a string, report error:
		lua_Debug entry;
		VERIFY(lua_getstack(m_LuaState, 0,   &entry));
		VERIFY(lua_getinfo (m_LuaState, "n", &entry));
		LOG("Error in function '%s': expected a string as parameter #%d, got %s",
			(entry.name != nullptr) ? entry.name : "?", i, getTypeText(i).c_str()
		);
		return false;
	}  // for i - Param
	
	// All params checked ok
	return true;
}





bool LuaState::checkParamFunction(int a_StartParam, int a_EndParam)
{
	ASSERT(isValid());
	
	if (a_EndParam < 0)
	{
		a_EndParam = a_StartParam;
	}
	
	for (int i = a_StartParam; i <= a_EndParam; i++)
	{
		if (lua_isfunction(m_LuaState, i))
		{
			continue;
		}
		// Not a function, report error:
		lua_Debug entry;
		VERIFY(lua_getstack(m_LuaState, 0,   &entry));
		VERIFY(lua_getinfo (m_LuaState, "n", &entry));
		LOG("Error in function '%s': expected a function as parameter #%d, got %s",
			(entry.name != nullptr) ? entry.name : "?", i, getTypeText(i).c_str()
		);
		return false;
	}  // for i - Param
	
	// All params checked ok
	return true;
}





bool LuaState::checkParamFunctionOrNil(int a_StartParam, int a_EndParam)
{
	ASSERT(isValid());
	
	if (a_EndParam < 0)
	{
		a_EndParam = a_StartParam;
	}
	
	for (int i = a_StartParam; i <= a_EndParam; i++)
	{
		if (lua_isfunction(m_LuaState, i) || lua_isnil(m_LuaState, i))
		{
			continue;
		}
		// Not a function / nil, report error:
		lua_Debug entry;
		VERIFY(lua_getstack(m_LuaState, 0,   &entry));
		VERIFY(lua_getinfo (m_LuaState, "n", &entry));
		LOG("Error in function '%s': expected a function or nil as parameter #%d, got %s",
			(entry.name != nullptr) ? entry.name : "?", i, getTypeText(i).c_str()
		);
		return false;
	}  // for i - Param
	
	// All params checked ok
	return true;
}





bool LuaState::checkParamEnd(int a_Param)
{
	if (lua_isnoneornil(m_LuaState, a_Param))
	{
		return true;
	}

	// Not an end, report error:
	lua_Debug entry;
	VERIFY(lua_getstack(m_LuaState, 0,   &entry));
	VERIFY(lua_getinfo (m_LuaState, "n", &entry));
	LOG("Error in function '%s': expected no more parameters at #%d, got %s",
		(entry.name != nullptr) ? entry.name : "?", a_Param, getTypeText(a_Param).c_str()
	);
	return false;
}





bool LuaState::reportErrors(int a_Status)
{
	return reportErrors(m_LuaState, a_Status);
}





bool LuaState::reportErrors(lua_State * a_LuaState, int a_Status)
{
	if (a_Status == 0)
	{
		// No error to report
		return false;
	}
	
	LOGWARNING("LUA: %d - %s", a_Status, lua_tostring(a_LuaState, -1));
	lua_pop(a_LuaState, 1);
	return true;
}





void LuaState::logStackTrace(int a_StartingDepth)
{
	logStackTrace(m_LuaState, a_StartingDepth);
}





void LuaState::logStackTrace(lua_State * a_LuaState, int a_StartingDepth)
{
	LOGWARNING("Stack trace:");
	lua_Debug entry;
	int depth = a_StartingDepth;
	while (lua_getstack(a_LuaState, depth, &entry))
	{
		lua_getinfo(a_LuaState, "Sln", &entry);
		LOGWARNING("  %s(%d): %s", entry.short_src, entry.currentline, entry.name ? entry.name : "(no name)");
		depth++;
	}
	LOGWARNING("Stack trace end");
}





AString LuaState::getTypeText(int a_StackPos)
{
	return lua_typename(m_LuaState, lua_type(m_LuaState, a_StackPos));
}





int LuaState::callFunctionWithForeignParams(
	const AString & a_FunctionName,
	LuaState & a_SrcLuaState,
	int a_SrcParamStart,
	int a_SrcParamEnd
)
{
	ASSERT(isValid());
	ASSERT(a_SrcLuaState.isValid());
	
	// Store the stack position before any changes
	int OldTop = lua_gettop(m_LuaState);
	
	// push the function to call, including the error handler:
	if (!pushFunction(a_FunctionName.c_str()))
	{
		LOGWARNING("Function '%s' not found", a_FunctionName.c_str());
		lua_pop(m_LuaState, 2);
		return -1;
	}

	// Copy the function parameters to the target state
	if (copyStackFrom(a_SrcLuaState, a_SrcParamStart, a_SrcParamEnd) < 0)
	{
		// Something went wrong, fix the stack and exit
		lua_pop(m_LuaState, 2);
		m_NumCurrentFunctionArgs = -1;
		m_CurrentFunctionName.clear();
		return -1;
	}
	
	// Call the function, with an error handler:
	int s = lua_pcall(m_LuaState, a_SrcParamEnd - a_SrcParamStart + 1, LUA_MULTRET, OldTop + 1);
	if (reportErrors(s))
	{
		LOGWARNING("Error while calling function '%s' in '%s'", a_FunctionName.c_str(), m_SubsystemName.c_str());
		// Fix the stack.
		// We don't know how many values have been pushed, so just get rid of any that weren't there initially
		int CurTop = lua_gettop(m_LuaState);
		if (CurTop > OldTop)
		{
			lua_pop(m_LuaState, CurTop - OldTop);
		}
		
		// Reset the internal checking mechanisms:
		m_NumCurrentFunctionArgs = -1;
		m_CurrentFunctionName.clear();
		
		// Make Lua think everything is okay and return 0 values, so that plugins continue executing.
		// The failure is indicated by the zero return values.
		return 0;
	}
	
	// Reset the internal checking mechanisms:
	m_NumCurrentFunctionArgs = -1;
	m_CurrentFunctionName.clear();
	
	// Remove the error handler from the stack:
	lua_remove(m_LuaState, OldTop + 1);
	
	// Return the number of return values:
	return lua_gettop(m_LuaState) - OldTop;
}





int LuaState::copyStackFrom(LuaState & a_SrcLuaState, int a_SrcStart, int a_SrcEnd)
{
	/*
	// DEBUG:
	LOGD("Copying stack values from %d to %d", a_SrcStart, a_SrcEnd);
	a_SrcLuaState.logStack("Src stack before copying:");
	logStack("Dst stack before copying:");
	*/
	for (int i = a_SrcStart; i <= a_SrcEnd; ++i)
	{
		int t = lua_type(a_SrcLuaState, i);
		switch (t)
		{
			case LUA_TNIL:
			{
				lua_pushnil(m_LuaState);
				break;
			}
			case LUA_TSTRING:
			{
				AString s;
				a_SrcLuaState.toString(i, s);
				push(s);
				break;
			}
			case LUA_TBOOLEAN:
			{
				bool b = (lua_toboolean(a_SrcLuaState, i) != 0);
				push(b);
				break;
			}
			case LUA_TNUMBER:
			{
				lua_Number d = lua_tonumber(a_SrcLuaState, i);
				push(d);
				break;
			}
			default:
			{
				LOGWARNING("%s: Unsupported value: '%s' at stack position %d. Can only copy numbers, strings, bools and classes!",
					__FUNCTION__, lua_typename(a_SrcLuaState, t), i
				);
				a_SrcLuaState.logStack("Stack where copying failed:");
				lua_pop(m_LuaState, i - a_SrcStart);
				return -1;
			}
		}
	}
	return a_SrcEnd - a_SrcStart + 1;
}





void LuaState::toString(int a_StackPos, AString & a_String)
{
	size_t len;
	const char * s = lua_tolstring(m_LuaState, a_StackPos, &len);
	if (s != nullptr)
	{
		a_String.assign(s, len);
	}
}





void LuaState::logStack(const char * a_Header)
{
	logStack(m_LuaState, a_Header);
}





void LuaState::logStack(lua_State * a_LuaState, const char * a_Header)
{
	// Format string consisting only of %s is used to appease the compiler
	LOG("%s", (a_Header != nullptr) ? a_Header : "Lua C API Stack contents:");
	for (int i = lua_gettop(a_LuaState); i > 0; i--)
	{
		AString Value;
		int Type = lua_type(a_LuaState, i);
		switch (Type)
		{
			case LUA_TBOOLEAN: Value.assign((lua_toboolean(a_LuaState, i) != 0) ? "true" : "false"); break;
			case LUA_TLIGHTUSERDATA: Printf(Value, "%p", lua_touserdata(a_LuaState, i)); break;
			case LUA_TNUMBER:        Printf(Value, "%f", (double)lua_tonumber(a_LuaState, i)); break;
			case LUA_TSTRING:        Printf(Value, "%s", lua_tostring(a_LuaState, i)); break;
			case LUA_TTABLE:         Printf(Value, "%p", lua_topointer(a_LuaState, i)); break;
			default: break;
		}
		LOGD("  Idx %d: type %d (%s) %s", i, Type, lua_typename(a_LuaState, Type), Value.c_str());
	}  // for i - stack idx
}





int LuaState::reportFnCallErrors(lua_State * a_LuaState)
{
	LOGWARNING("LUA: %s", lua_tostring(a_LuaState, -1));
	logStackTrace(a_LuaState, 1);
	return 1;  // We left the error message on the stack as the return value
}





////////////////////////////////////////////////////////////////////////////////
// LuaState::Ref:

LuaState::Ref::Ref(void) :
	m_LuaState(nullptr),
	m_Ref(LUA_REFNIL)
{
}





LuaState::Ref::Ref(LuaState & a_LuaState, int a_StackPos) :
	m_LuaState(nullptr),
	m_Ref(LUA_REFNIL)
{
	refStack(a_LuaState, a_StackPos);
}





LuaState::Ref::~Ref()
{
	if (m_LuaState != nullptr)
	{
		unRef();
	}
}





void LuaState::Ref::refStack(LuaState & a_LuaState, int a_StackPos)
{
	ASSERT(a_LuaState.isValid());
	if (m_LuaState != nullptr)
	{
		unRef();
	}
	m_LuaState = &a_LuaState;
	lua_pushvalue(a_LuaState, a_StackPos);  // push a copy of the value at a_StackPos onto the stack
	m_Ref = luaL_ref(a_LuaState, LUA_REGISTRYINDEX);
}





void LuaState::Ref::unRef(void)
{
	ASSERT(m_LuaState->isValid());  // The reference should be destroyed before destroying the LuaState
	
	if (isValid())
	{
		luaL_unref(*m_LuaState, LUA_REGISTRYINDEX, m_Ref);
	}
	m_LuaState = nullptr;
	m_Ref = LUA_REFNIL;
}




