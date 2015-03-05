
-- Main.lua

-- Implements the main entrypoint to the controller





local function listBots(a_Game)
	for k, bot in pairs(a_Game.allBots) do
		print("  bot " .. k .. ":")
		for k2, val in pairs(bot) do
			print("    " .. k2 .. ": " .. tostring(val))
		end
	end
end





function onGameStarted(a_Game)
	print("LUA: onGameStarted")
	
	-- Accellerate all my bots:
	for idx, bot in pairs(a_Game.allBots) do
		if not(bot.isEnemy) then
			a_Game.botCommands[idx] = {cmd = "accelerate"}
		end
	end
end





function onGameUpdate(a_Game)
	print("LUA: onGameUpdate")
	-- listBots(a_Game)
	
	local cmd = {cmd = "steer", angle = a_Game.speedLevels[2].maxAngularSpeed}
	if not(a_Game.botCommands[1]) then
		-- The previous command has been sent, send another one:
		a_Game.botCommands[1] = cmd
	end
end





function onGameFinished(a_Game)
	print("LUA: onGameFinished")
end





function onBotDied(a_Game, a_BotID)
	local friendliness
	if (a_Game.allBots[a_BotID].isEnemy) then
		friendliness = "(enemy)"
	else
		friendliness = "(my)"
	end
	print("LUA: onBotDied: bot #" .. a_BotID .. friendliness)
end