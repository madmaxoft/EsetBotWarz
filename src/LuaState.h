
// LuaState.h

// Declares the LuaState class representing the wrapper over lua_State *, provides associated helper functions

/*
The contained lua_State can be either owned or attached.
Owned lua_State is created by calling create() and the LuaState automatically closes the state
Or, lua_State can be attached by calling attach(), the LuaState doesn't close such a state
Attaching a state will automatically close an owned state.

Calling a Lua function is done internally by pushing the function using pushFunction(), then pushing the
arguments and finally executing callFunction(). LuaState automatically keeps track of the number of
arguments and the name of the function (for logging purposes). After the call the return values are read from
the stack using getStackValue(). All of this is wrapped in a templated function overloads LuaState::call().

Reference management is provided by the LuaState::Ref class. This is used when you need to hold a reference to
any Lua object across several function calls; usually this is used for callbacks. The class is RAII-like, with
automatic resource management.
*/




#pragma once

extern "C"
{
	#include "lib/lua/src/lauxlib.h"
}





/** Encapsulates a Lua state and provides some syntactic sugar for common operations */
class LuaState
{
public:

	/** Used for storing references to object in the global registry.
	Can be bound (contains a reference) or unbound (doesn't contain reference).
	The reference can also be reset by calling RefStack(). */
	class Ref
	{
	public:
		/** Creates an unbound reference object. */
		Ref(void);
		
		/** Creates a reference in the specified LuaState for object at the specified StackPos */
		Ref(LuaState & a_LuaState, int a_StackPos);
		
		~Ref();
		
		/** Creates a reference to Lua object at the specified stack pos, binds this object to it.
		Calls UnRef() first if previously bound to another reference. */
		void refStack(LuaState & a_LuaState, int a_StackPos);
		
		/** Removes the bound reference, resets the object to Unbound state. */
		void unRef(void);
		
		/** Returns true if the reference is valid */
		bool isValid(void) const {return (m_Ref != LUA_REFNIL); }
		
		/** Allows to use this class wherever an int (i. e. ref) is to be used */
		operator int(void) const { return m_Ref; }
		
	protected:
		LuaState * m_LuaState;
		int m_Ref;
	} ;
	
	
	/** Used for calling functions stored in a reference-stored table */
	class TableRef
	{
		int m_TableRef;
		const char * m_FnName;
	public:
		TableRef(int a_TableRef, const char * a_FnName) :
			m_TableRef(a_TableRef),
			m_FnName(a_FnName)
		{
		}
		
		int getTableRef(void) const { return m_TableRef; }
		const char * getFnName(void) const { return m_FnName; }
	} ;
	
	
	/** A dummy class that's used only to delimit function args from return values for LuaState::call() */
	class Ret
	{
	} ;
	
	static const Ret Return;  // Use this constant to delimit function args from return values for LuaState::call()


	/** Creates a new instance. The LuaState is not initialized.
	a_SubsystemName is used for reporting problems in the console, it is "plugin %s" for plugins,
	or "LuaScript" for the cLuaScript template
	*/
	LuaState(const AString & a_SubsystemName);
	
	/** Creates a new instance. The a_AttachState is attached.
	Subsystem name is set to "<attached>".
	*/
	explicit LuaState(lua_State * a_AttachState);
	
	~LuaState();
	
	/** Allows this object to be used in the same way as a lua_State *, for example in the LuaLib functions */
	operator lua_State * (void) { return m_LuaState; }
	
	/** Creates the m_LuaState, if not closed already. This state will be automatically closed in the destructor.
	The regular Lua libs are registered, but the MCS API is not registered (so that Lua can be used as
	lite-config as well), use RegisterAPILibs() to do that. */
	void create(void);
	
	/** Registers all the API libraries that MCS provides into m_LuaState. */
	void registerAPILibs(void);
	
	/** Closes the m_LuaState, if not closed already */
	void close(void);
	
	/** Attaches the specified state. Operations will be carried out on this state, but it will not be closed in the destructor */
	void attach(lua_State * a_State);
	
	/** Detaches a previously attached state. */
	void detach(void);
	
	/** Returns true if the m_LuaState is valid */
	bool isValid(void) const { return (m_LuaState != nullptr); }
	
	/** Adds the specified path to package.<a_PathVariable> */
	void addPackagePath(const AString & a_PathVariable, const AString & a_Path);
	
	/** Loads the specified file
	Returns false and logs a warning to the console if not successful (but the LuaState is kept open).
	m_SubsystemName is displayed in the warning log message.
	*/
	bool loadFile(const AString & a_FileName);
	
	/** Returns true if a_FunctionName is a valid Lua function that can be called */
	bool hasFunction(const char * a_FunctionName);
	
	// Push a const value onto the stack (keep alpha-sorted):
	void push(const AString & a_String);
	void push(const AStringVector & a_Vector);
	void push(const char * a_Value);

	// Push a value onto the stack (keep alpha-sorted):
	void push(bool a_Value);
	void push(double a_Value);
	void push(int a_Value);
	
	/** Retrieve value at a_StackPos, if it is a valid bool. If not, a_Value is unchanged */
	void getStackValue(int a_StackPos, bool & a_Value);
	
	/** Retrieve value at a_StackPos, if it is a valid string. If not, a_Value is unchanged */
	void getStackValue(int a_StackPos, AString & a_Value);
	
	/** Retrieve value at a_StackPos, if it is a valid number. If not, a_Value is unchanged */
	void getStackValue(int a_StackPos, int & a_Value);
	
	/** Retrieve value at a_StackPos, if it is a valid number. If not, a_Value is unchanged */
	void getStackValue(int a_StackPos, double & a_Value);
	
	/** Call the specified Lua function.
	Returns true if call succeeded, false if there was an error.
	A special param of Ret & signifies the end of param list and the start of return values.
	Example call: Call(Fn, Param1, Param2, Param3, LuaState::Return, Ret1, Ret2) */
	template <typename FnT, typename... Args>
	bool call(const FnT & a_Function, Args &&... args)
	{
		if (!pushFunction(a_Function))
		{
			// Pushing the function failed
			return false;
		}
		return pushCallPop(args...);
	}

	/** Retrieves a list of values from the Lua stack, starting at the specified index. */
	template <typename T, typename... Args>
	inline void getStackValues(int a_StartStackPos, T & a_Ret, Args &&... args)
	{
		getStackValue(a_StartStackPos, a_Ret);
		getStackValues(a_StartStackPos + 1, args...);
	}

	/** Returns true if the specified parameters on the stack are of the specified usertable type; also logs warning if not. Used for static functions */
	bool checkParamUserTable(int a_StartParam, const char * a_UserTable, int a_EndParam = -1);
	
	/** Returns true if the specified parameters on the stack are of the specified usertype; also logs warning if not. Used for regular functions */
	bool checkParamUserType(int a_StartParam, const char * a_UserType, int a_EndParam = -1);
	
	/** Returns true if the specified parameters on the stack are tables; also logs warning if not */
	bool checkParamTable(int a_StartParam, int a_EndParam = -1);
	
	/** Returns true if the specified parameters on the stack are numbers; also logs warning if not */
	bool checkParamNumber(int a_StartParam, int a_EndParam = -1);
	
	/** Returns true if the specified parameters on the stack are strings; also logs warning if not */
	bool checkParamString(int a_StartParam, int a_EndParam = -1);
	
	/** Returns true if the specified parameters on the stack are functions; also logs warning if not */
	bool checkParamFunction(int a_StartParam, int a_EndParam = -1);
	
	/** Returns true if the specified parameters on the stack are functions or nils; also logs warning if not */
	bool checkParamFunctionOrNil(int a_StartParam, int a_EndParam = -1);
	
	/** Returns true if the specified parameter on the stack is nil (indicating an end-of-parameters) */
	bool checkParamEnd(int a_Param);
	
	/** If the status is nonzero, prints the text on the top of Lua stack and returns true */
	bool reportErrors(int status);
	
	/** If the status is nonzero, prints the text on the top of Lua stack and returns true */
	static bool reportErrors(lua_State * a_LuaState, int status);
	
	/** Logs all items in the current stack trace to the server console */
	void logStackTrace(int a_StartingDepth = 0);
	
	/** Logs all items in the current stack trace to the server console */
	static void logStackTrace(lua_State * a_LuaState, int a_StartingDepth = 0);
	
	/** Returns the type of the item on the specified position in the stack */
	AString getTypeText(int a_StackPos);
	
	/** Calls the function specified by its name, with arguments copied off the foreign state.
	If successful, keeps the return values on the stack and returns their number.
	If unsuccessful, returns a negative number and keeps the stack position unchanged. */
	int callFunctionWithForeignParams(
		const AString & a_FunctionName,
		LuaState & a_SrcLuaState,
		int a_SrcParamStart,
		int a_SrcParamEnd
	);
	
	/** Copies objects on the stack from the specified state.
	Only numbers, bools, strings and userdatas are copied.
	If successful, returns the number of objects copied.
	If failed, returns a negative number and rewinds the stack position. */
	int copyStackFrom(LuaState & a_SrcLuaState, int a_SrcStart, int a_SrcEnd);
	
	/** Reads the value at the specified stack position as a string and sets it to a_String. */
	void toString(int a_StackPos, AString & a_String);

	/** Logs all the elements' types on the API stack, with an optional header for the listing. */
	void logStack(const char * a_Header = nullptr);
	
	/** Logs all the elements' types on the API stack, with an optional header for the listing. */
	static void logStack(lua_State * a_LuaState, const char * a_Header = nullptr);
	
protected:

	lua_State * m_LuaState;
	
	/** If true, the state is owned by this object and will be auto-Closed. False => attached state */
	bool m_IsOwned;
	
	/** The subsystem name is used for reporting errors to the console, it is either "plugin %s" or "LuaScript"
	whatever is given to the constructor
	*/
	AString m_SubsystemName;
	
	/** Name of the currently pushed function (for the Push / Call chain) */
	AString m_CurrentFunctionName;
	
	/** Number of arguments currently pushed (for the Push / Call chain) */
	int m_NumCurrentFunctionArgs;


	/** Variadic template terminator: If there's nothing more to push / pop, just call the function.
	Note that there are no return values either, because those are prefixed by a Ret value, so the arg list is never empty. */
	bool pushCallPop(void)
	{
		return callFunction(0);
	}

	/** Variadic template recursor: More params to push. Push them and recurse. */
	template <typename T, typename... Args>
	inline bool pushCallPop(T a_Param, Args &&... args)
	{
		Push(a_Param);
		return PushCallPop(args...);
	}

	/** Variadic template terminator: If there's nothing more to push, but return values to collect, call the function and collect the returns. */
	template <typename... Args>
	bool pushCallPop(LuaState::Ret, Args &&... args)
	{
		// Calculate the number of return values (number of args left):
		int numReturns = sizeof...(args);

		// Call the function:
		if (!callFunction(numReturns))
		{
			return false;
		}

		// Collect the return values:
		getStackValues(-numReturns, args...);
		lua_pop(m_LuaState, numReturns);

		// All successful:
		return true;
	}

	/** Variadic template terminator: If there are no more values to get, bail out.
	This function is not available in the public API, because it's an error to request no values directly; only internal functions can do that.
	If you get a compile error saying this function is not accessible, check your calling code, you aren't reading any stack values. */
	void getStackValues(int a_StartingStackPos)
	{
		// Do nothing
	}

	/** Pushes the function of the specified name onto the stack.
	Returns true if successful. Logs a warning on failure (incl. m_SubsystemName)
	*/
	bool pushFunction(const char * a_FunctionName);
	
	/** Pushes a function that has been saved into the global registry, identified by a_FnRef.
	Returns true if successful. Logs a warning on failure
	*/
	bool pushFunction(int a_FnRef);
	
	/** Pushes a function that has been saved as a reference.
	Returns true if successful. Logs a warning on failure
	*/
	bool pushFunction(const Ref & a_FnRef)
	{
		return pushFunction(a_FnRef.operator int());
	}
	
	/** Pushes a function that is stored in a referenced table by name
	Returns true if successful. Logs a warning on failure
	*/
	bool pushFunction(const TableRef & a_TableRef);
	
	/** Pushes a usertype of the specified class type onto the stack */
	void pushUserType(void * a_Object, const char * a_Type);

	/**
	Calls the function that has been pushed onto the stack by PushFunction(),
	with arguments pushed by PushXXX().
	Returns true if successful, logs a warning on failure.
	*/
	bool callFunction(int a_NumReturnValues);
	
	/** Used as the error reporting function for function calls */
	static int reportFnCallErrors(lua_State * a_LuaState);
} ;




