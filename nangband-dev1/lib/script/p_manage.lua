-- Available player powers
powers = {}

-- Add a class power.
function powers.class_power(class, power)

	-- Check for player class
	if player.pclass == class then
		tinsert(powers, power)
	end
end

-- Add a racial power.
function powers.race_power(race, power)

	-- Check for player race
	if player.prace == race then
		tinsert(powers, power)
	end
end

-- Invoke a power; return false if the power was cancelled.
function powers.invoke(index)
	return powers[index].effect()
end

-- Get minumum level for a power.
function powers.minlevel(index)
	return powers[index].minlevel
end

-- Get amount of energy needed for a power.
function powers.energy(index)
	return powers[index].energy()
end

-- Get a power's name.
function powers.name(index)
	return powers[index].name
end

-- Get extra info about a spell (damage, duration, ...)
function powers.description(index)
	local desc = powers[index].desc
	if not desc then return "" end

	return desc
end

-- Do the powers
function do_cmd_power()
	local i
	local power_size = getn(powers)

	-- Paranoia
	if power_size < 1 then

		-- Inform the user
		msg_print("You have no powers to use!")

		-- We are done
		return
	end

	-- Save the screen
	screen_save()

	-- Print a list of powers
	for i = 1, power_size do
		prt(i .. ") " .. powers.name(i), i+1, 0)
	end

	-- Get input
	i = inkey() - 48

	-- Load the screen
	screen_load()

	-- Trap user error
	if i > power_size or i < 0 then

		-- Tell the user
		msg_print("That power doesn't exist!")

		-- Load the screen
		screen_load()

		-- We are done
		return
	end

	-- Invoke the power
	powers.invoke(i)

	-- We are done
	return
end
