-- Warrior Powers
powers.class_power(CLASS_WARRIOR,
{
	name = "Silly Message",
	desc = "Prints a silly message to the screen.  Very useless.",
	level = 5,
	stat = A_STR,
	minstat = 14,
	energy = function()
		if player.plev < 10 then return 15 end
		return 10
	end,
	effect = function() msg_print("Silly Message") end,
})

-- Human power
powers.race_power(RACE_HUMAN,
{
	name = "Testing (human power)",
	desc = "Prints a silly message to the screen.  Very useless.",
	level = 5,
	stat = A_STR,
	minstat = 14,
	energy = function() return 15 end,
	effect = function() msg_print("Testing ... 1 2 3") end,
})
