-- Spell-casting code


-----------------------------------------------------------
-- Available "schools" of magic
magic_schools = {}


-----------------------------------------------------------
-- Helper functions for spell-casting

-- Create a new "school" of magic
function add_school(tval)
	local school = {}
	school.spells = {}
	school.books = {}
	magic_schools[tval] = school
	return school
end


-- Adds a spell to a "school" of magic and
-- returns the zero-based index of the spell
function add_spell(school, spell)
	tinsert(school.spells, spell)
	return getn(school.spells) - 1
end


-- Add a new book to a "school" of magic
-- sval is the sval of the book to be added
-- spells is a table of spell-indices for that book
function add_book(school, sval, spells)
	school.books[sval] = spells
end


-- Get the index of the spell on a specific page of a spellbook
-- return -1 when the page doesn't exist
function get_spell_index(book, position)
	local spell = book[position + 1]

	-- No spell on that page
	if not spell then return -1 end

	return spell
end


-- Cast a spell
-- return FALSE if the player canceled the casting
function cast_spell(school, index)
	return school.spells[index + 1].effect()
end


-- Get extra info about a spell (damage, duration, ...)
function get_spell_info(school, index)
	local info = school.spells[index + 1].info
	if not info then return "" end

	return info()
end


-- Get the name of a spell
function get_spell_name(school, index)
	return school.spells[index + 1].name
end

