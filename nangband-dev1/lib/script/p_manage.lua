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

-- Get the stat used for this power
function powers.stat(index)

	-- If "index" is a function, then return it's result
	if type(powers[index].level) == "function" then
		return powers[index].stat()
	end

	-- Otherwise, just return the stat
	return powers[index].stat
end

-- Get the requirement for the stat
function powers.require(index)

	-- If "minstat" is a function, then return it's result
	if type(powers[index].minstat) == "function" then
		return powers[index].stat()
	end

	-- Otherwise, just return the requirement
	return powers[index].minstat
end

-- Get minumum level for a power.
function powers.minlevel(index)

	-- If "level" is a function, then return it's result
	if type(powers[index].level) == "function" then
		return powers[index].level()
	end

	-- Otherwise, just return the energy
	return powers[index].level
end

-- Get amount of energy needed for a power.
function powers.energy(index)

	-- If "energy" is a function, then return it's result
	if type(powers[index].energy) == "function" then
		return powers[index].energy()
	end

	-- Otherwise, just return the energy
	return powers[index].energy
end

-- Get a power's name.
function powers.name(index)

	-- If "name" is a function, then return it's result
	if type(powers[index].name) == "function" then
		return powers[index].name()
	end

	-- Otherwise, just return the name
	return powers[index].name
end

-- Get extra info about a spell (damage, duration, ...)
function powers.description(index)
	local desc

	-- If the description is a name, call it
	if type(powers[index].desc) == "function" then
		desc = powers[index].desc()
	else
		-- Otherwise, just use it
		desc = powers[index].desc
	end

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
