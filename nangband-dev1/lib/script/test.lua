-- A very simple test-suite for Lua support in Angband

if nil then
function test_use_object_hook(test_tval, test_svals)
	for test_sval, _ in test_svals do
		local test_object = {tval = test_tval, sval = test_sval, pval = 1, k_idx = 1, ident = 0}
		use_object_hook(test_object)
	end
end

-- Test the food
test_use_object_hook(TV_FOOD, {
		SV_FOOD_POISON,
		SV_FOOD_BLINDNESS,
		SV_FOOD_PARANOIA,
		SV_FOOD_CONFUSION,
		SV_FOOD_HALLUCINATION,
		SV_FOOD_PARALYSIS,
		SV_FOOD_WEAKNESS,
		SV_FOOD_SICKNESS,
		SV_FOOD_STUPIDITY,
		SV_FOOD_NAIVETY,
		SV_FOOD_UNHEALTH,
		SV_FOOD_DISEASE,
		SV_FOOD_CURE_POISON,
		SV_FOOD_CURE_BLINDNESS,
		SV_FOOD_CURE_PARANOIA,
		SV_FOOD_CURE_CONFUSION,
		SV_FOOD_CURE_SERIOUS,
		SV_FOOD_RESTORE_STR,
		SV_FOOD_RESTORE_CON,
		SV_FOOD_RESTORING,
		SV_FOOD_BISCUIT,
		SV_FOOD_JERKY,
		SV_FOOD_RATION,
		SV_FOOD_SLIME_MOLD,
		SV_FOOD_WAYBREAD,
		SV_FOOD_PINT_OF_ALE,
		SV_FOOD_PINT_OF_WINE
		SV_FOOD_CAVIAR})

-- Test the potions
test_use_object_hook(TV_POTION, {
		SV_POTION_WATER,
		SV_POTION_APPLE_JUICE,
		SV_POTION_SLIME_MOLD,
		SV_POTION_SLOWNESS,
		SV_POTION_SALT_WATER,
		SV_POTION_POISON,
		SV_POTION_BLINDNESS,
		SV_POTION_CONFUSION,
		SV_POTION_SLEEP,
		SV_POTION_LOSE_MEMORIES,
		SV_POTION_RUINATION,
		SV_POTION_DEC_STR,
		SV_POTION_DEC_INT,
		SV_POTION_DEC_WIS,
		SV_POTION_DEC_DEX,
		SV_POTION_DEC_CON,
		SV_POTION_DEC_CHR,
		SV_POTION_DETONATIONS,
		SV_POTION_DEATH,
		SV_POTION_INFRAVISION,
		SV_POTION_DETECT_INVIS,
		SV_POTION_SLOW_POISON,
		SV_POTION_CURE_POISON,
		SV_POTION_BOLDNESS,
		SV_POTION_SPEED,
		SV_POTION_RESIST_HEAT,
		SV_POTION_RESIST_COLD,
		SV_POTION_HEROISM,
		SV_POTION_BERSERK_STRENGTH,
		SV_POTION_CURE_LIGHT,
		SV_POTION_CURE_SERIOUS,
		SV_POTION_CURE_CRITICAL,
		SV_POTION_HEALING,
		SV_POTION_STAR_HEALING,
		SV_POTION_LIFE,
		SV_POTION_RESTORE_MANA,
		SV_POTION_RESTORE_EXP,
		SV_POTION_RES_STR,
		SV_POTION_RES_INT,
		SV_POTION_RES_WIS,
		SV_POTION_RES_DEX,
		SV_POTION_RES_CON,
		SV_POTION_RES_CHR,
		SV_POTION_INC_STR,
		SV_POTION_INC_INT,
		SV_POTION_INC_WIS,
		SV_POTION_INC_DEX,
		SV_POTION_INC_CON,
		SV_POTION_INC_CHR,
		SV_POTION_AUGMENTATION,
		SV_POTION_ENLIGHTENMENT,
		SV_POTION_STAR_ENLIGHTENMENT,
		SV_POTION_SELF_KNOWLEDGE,
		SV_POTION_EXPERIENCE})

-- Test the scrolls
test_use_object_hook(TV_SCROLL, {
		SV_SCROLL_DARKNESS,
		SV_SCROLL_AGGRAVATE_MONSTER,
		SV_SCROLL_CURSE_ARMOR,
		SV_SCROLL_CURSE_WEAPON,
		SV_SCROLL_SUMMON_MONSTER,
		SV_SCROLL_SUMMON_UNDEAD,
		SV_SCROLL_TRAP_CREATION,
		SV_SCROLL_PHASE_DOOR,
		SV_SCROLL_TELEPORT,
		SV_SCROLL_TELEPORT_LEVEL,
		SV_SCROLL_WORD_OF_RECALL,
		SV_SCROLL_IDENTIFY,
		SV_SCROLL_STAR_IDENTIFY,
		SV_SCROLL_REMOVE_CURSE,
		SV_SCROLL_STAR_REMOVE_CURSE,
		SV_SCROLL_ENCHANT_ARMOR,
		SV_SCROLL_ENCHANT_WEAPON_TO_HIT,
		SV_SCROLL_ENCHANT_WEAPON_TO_DAM,
		SV_SCROLL_STAR_ENCHANT_ARMOR,
		SV_SCROLL_STAR_ENCHANT_WEAPON,
		SV_SCROLL_RECHARGING,
		SV_SCROLL_LIGHT,
		SV_SCROLL_MAPPING,
		SV_SCROLL_DETECT_GOLD,
		SV_SCROLL_DETECT_ITEM,
		SV_SCROLL_DETECT_TRAP,
		SV_SCROLL_DETECT_DOOR,
		SV_SCROLL_DETECT_INVIS,
		SV_SCROLL_SATISFY_HUNGER,
		SV_SCROLL_BLESSING,
		SV_SCROLL_HOLY_CHANT,
		SV_SCROLL_HOLY_PRAYER,
		SV_SCROLL_MONSTER_CONFUSION,
		SV_SCROLL_PROTECTION_FROM_EVIL,
		SV_SCROLL_RUNE_OF_PROTECTION,
		SV_SCROLL_TRAP_DOOR_DESTRUCTION,
		SV_SCROLL_STAR_DESTRUCTION,
		SV_SCROLL_DISPEL_UNDEAD,
		SV_SCROLL_GENOCIDE,
		SV_SCROLL_MASS_GENOCIDE,
		SV_SCROLL_ACQUIREMENT,
		SV_SCROLL_STAR_ACQUIREMENT})

-- Test the staves
test_use_object_hook(TV_STAFF, {
		SV_STAFF_DARKNESS,
		SV_STAFF_SLOWNESS,
		SV_STAFF_HASTE_MONSTERS,
		SV_STAFF_SUMMONING,
		SV_STAFF_TELEPORTATION,
		SV_STAFF_IDENTIFY,
		SV_STAFF_REMOVE_CURSE,
		SV_STAFF_STARLITE,
		SV_STAFF_LITE,
		SV_STAFF_MAPPING,
		SV_STAFF_DETECT_GOLD,
		SV_STAFF_DETECT_ITEM,
		SV_STAFF_DETECT_TRAP,
		SV_STAFF_DETECT_DOOR,
		SV_STAFF_DETECT_INVIS,
		SV_STAFF_DETECT_EVIL,
		SV_STAFF_CURE_LIGHT,
		SV_STAFF_CURING,
		SV_STAFF_HEALING,
		SV_STAFF_THE_MAGI,
		SV_STAFF_SLEEP_MONSTERS,
		SV_STAFF_SLOW_MONSTERS,
		SV_STAFF_SPEED,
		SV_STAFF_PROBING,
		SV_STAFF_DISPEL_EVIL,
		SV_STAFF_POWER,
		SV_STAFF_HOLINESS,
		SV_STAFF_GENOCIDE,
		SV_STAFF_EARTHQUAKES,
		SV_STAFF_DESTRUCTION})

-- Test the wands
test_use_object_hook(TV_WAND, {
		SV_WAND_HEAL_MONSTER,
		SV_WAND_HASTE_MONSTER,
		SV_WAND_CLONE_MONSTER,
		SV_WAND_TELEPORT_AWAY,
		SV_WAND_DISARMING,
		SV_WAND_TRAP_DOOR_DEST,
		SV_WAND_STONE_TO_MUD,
		SV_WAND_LITE,
		SV_WAND_SLEEP_MONSTER,
		SV_WAND_SLOW_MONSTER,
		SV_WAND_CONFUSE_MONSTER,
		SV_WAND_FEAR_MONSTER,
		SV_WAND_DRAIN_LIFE,
		SV_WAND_POLYMORPH,
		SV_WAND_STINKING_CLOUD,
		SV_WAND_MAGIC_MISSILE,
		SV_WAND_ACID_BOLT,
		SV_WAND_ELEC_BOLT,
		SV_WAND_FIRE_BOLT,
		SV_WAND_COLD_BOLT,
		SV_WAND_ACID_BALL,
		SV_WAND_ELEC_BALL,
		SV_WAND_FIRE_BALL,
		SV_WAND_COLD_BALL,
		SV_WAND_WONDER,
		SV_WAND_ANNIHILATION,
		SV_WAND_DRAGON_FIRE,
		SV_WAND_DRAGON_COLD,
		SV_WAND_DRAGON_BREATH})

-- Test the rods
test_use_object_hook(TV_ROD, {
		SV_ROD_DETECT_TRAP,
		SV_ROD_DETECT_DOOR,
		SV_ROD_IDENTIFY,
		SV_ROD_RECALL,
		SV_ROD_ILLUMINATION,
		SV_ROD_MAPPING,
		SV_ROD_DETECTION,
		SV_ROD_PROBING,
		SV_ROD_CURING,
		SV_ROD_HEALING,
		SV_ROD_RESTORATION,
		SV_ROD_SPEED,
		SV_ROD_TELEPORT_AWAY,
		SV_ROD_DISARMING,
		SV_ROD_LITE,
		SV_ROD_SLEEP_MONSTER,
		SV_ROD_SLOW_MONSTER,
		SV_ROD_DRAIN_LIFE,
		SV_ROD_POLYMORPH,
		SV_ROD_ACID_BOLT,
		SV_ROD_ELEC_BOLT,
		SV_ROD_FIRE_BOLT,
		SV_ROD_COLD_BOLT,
		SV_ROD_ACID_BALL,
		SV_ROD_ELEC_BALL,
		SV_ROD_FIRE_BALL,
		SV_ROD_COLD_BALL})
end

for i = 0, 64 do
	cast_spell_hook(TV_PRAYER_BOOK, i)
end
