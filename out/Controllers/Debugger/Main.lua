
-- Main.lua

-- Implements the main entrypoint to the controller





function onGameStarted(a_Game)
	print("LUA: onGameStarted")
	a_Game.botCommands[1] = {cmd = "accelerate"}
end





function onGameUpdate(a_Game)
	print("LUA: onGameUpdate")
	print("a_Game = " .. tostring(a_Game))
	print("a_Game.speedLevels = " .. tostring(a_Game.speedLevels))
	print("a_Game.speedLevels[2] = " .. tostring(a_Game.speedLevels[2]))
	print("a_Game.speedLevels[2].maxAngularSpeed = " .. tostring(a_Game.speedLevels[2].maxAngularSpeed))
	
	local cmd = {cmd = "turn", angle = a_Game.speedLevels[2].maxAngularSpeed}
	a_Game.botCommands[1] = cmd
end





function onGameFinished(a_Game)
	print("LUA: onGameFinished")
end




