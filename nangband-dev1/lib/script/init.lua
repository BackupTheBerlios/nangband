-- Redirect error messages to Angband's msg_print()
_ALERT = function(text)
	msg_print(text)
end

dofile(build_script_path("object.lua"))
dofile(build_script_path("spell.lua"))

-- Note to porters: only uncomment this line if you have defined
-- ALLOW_BORG *and* BORG_SCRIPTING.

-- dofile(build_script_patch("borg.lua"))
