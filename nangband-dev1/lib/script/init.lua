-- Redirect error messages to Angband's msg_print()
_ALERT = function(text)
	msg_print(text)
end

-- Simple helper function
function loadfile(name)
	dofile(build_script_path(name))
end

-- Load the managing files

-- Load the "powers" manager
loadfile("p_manage.lua")

-- Load the "spells" manager
loadfile("s_manage.lua")


-- Load the magic spells
loadfile("s_magic.lua")
loadfile("s_pray.lua")

-- Load the object uses
loadfile("object.lua")

-- Load the (silly) example powers
loadfile("p_exampl.lua")

