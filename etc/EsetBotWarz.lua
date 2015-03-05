
-- Implements EsetBotWarz interpreter description and interface for ZBStudio.

local function MakeEsetBotWarzInterpreter(a_InterpreterPostfix, a_ExePostfix)
	assert(type(a_InterpreterPostfix) == "string")
	assert(type(a_ExePostfix) == "string")

	return
	{
		name = "EsetBotWarz" .. a_InterpreterPostfix,
		description = "EsetBotWarz - the AI controller for JoinEset campaign BotWarz",
		api = {"baselib"},

		frun = function(self, wfilename, withdebug)
			-- The controllers are always in a "$/out/Controllers/<ControllerName>" subfolder
			-- Get to the executable by removing the last two dirs:
			local ExePath = wx.wxFileName(wfilename)
			ExePath:RemoveLastDir()
			ExePath:RemoveLastDir()
			ExePath:ClearExt()
			ExePath:SetName("")
			local ExeName = wx.wxFileName(ExePath)

			-- The executable name depends on the debug / non-debug build mode, it can have a postfix
			ExeName:SetName("EsetBotWarz" .. a_ExePostfix)

			-- Executable has a .exe ext on Windows
			if (ide.osname == 'Windows') then
				ExeName:SetExt("exe")
			end

			-- Start the debugger server:
			if withdebug then
				DebuggerAttachDefault({
					runstart = (ide.config.debugger.runonstart == true),
					basedir = ExePath:GetFullPath(),
				})
			end

			-- Add a "nooutbuf" cmdline param to the server, causing it to call setvbuf to disable output buffering:
			local Cmd = ExeName:GetFullPath() .. " " .. wfilename:GetFullPath() .. " /nooutbuf /pauseonexit /singlegame /logcomm"
			if (withdebug) then
				Cmd = Cmd .. " /zbsdebug"
			end

			-- Force ZBS not to hide MCS window, save and restore previous state:
			local SavedUnhideConsoleWindow = ide.config.unhidewindow.ConsoleWindowClass
			ide.config.unhidewindow.ConsoleWindowClass = 1  -- show if hidden
			
			-- Create the closure to call upon debugging finish:
			local OnFinished = function()
				-- Restore the Unhide status:
				ide.config.unhidewindow.ConsoleWindowClass = SavedUnhideConsoleWindow
			end

			-- Run the server:
			local pid = CommandLineRun(
				Cmd,                    -- Command to run
				ExePath:GetFullPath(),  -- Working directory for the debuggee
				false,                  -- Redirect debuggee output to Output pane? (NOTE: This force-hides the MCS window, not desirable!)
				true,                   -- Add a no-hide flag to WX
				nil,                    -- StringCallback, whatever that is
				nil,                    -- UID to identify this running program; nil to auto-assign
				OnFinished              -- Callback to call once the debuggee terminates
			)
		end,

		hasdebugger = true,
	}
end





return {
	name = "EsetBotWarz integration",
	description = "Integration with EsetBotWarz - the AI controller for JoinEset campaign BotWarz",
	author = "Mattes D (https://github.com/madmaxoft)",
	version = 0.1,

	onRegister = function(self)
		ide:AddInterpreter("esetbotwarz_debug", MakeEsetBotWarzInterpreter(" - debug mode", "_debug"))
		ide:AddInterpreter("esetbotwarz_release", MakeEsetBotWarzInterpreter(" - release mode", "_release"))
	end,

	onUnRegister = function(self)
		ide:RemoveInterpreter("esetbotwarz_debug")
		ide:RemoveInterpreter("esetbotwarz_release")
	end,
}




