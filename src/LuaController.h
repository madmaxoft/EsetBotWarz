
// LuaController.h

// Declares the createLuaController() function that returns a new LuaController instance





#pragma once





// fwd:
class BotWarzApp;
class Controller;





extern SharedPtr<Controller> createLuaController(BotWarzApp & a_App, const AString & a_FileName, bool a_ShouldDebugZBS);





