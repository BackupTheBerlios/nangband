/* File: cmd1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"



/*
 * Determine if the player "hits" a monster.
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Penalize invisible targets */
	if (!vis) chance = chance / 2;

	/* Power competes against armor */
	if ((chance > 0) && (rand_int(chance) >= (ac * 3 / 4))) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
sint critical_shot(int weight, int plus, int dam)
{
	int i, k;

	/* Extract "shot" power */
	i = (weight + ((p_ptr->to_h + plus) * 4) + (p_ptr->lev * 2));

	/* Critical hit */
	if (randint(5000) <= i)
	{
		k = weight + randint(500);

		if (k < 500)
		{
			msg_print("It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 1000)
		{
			msg_print("It was a great hit!");
			dam = 2 * dam + 10;
		}
		else
		{
			msg_print("It was a superb hit!");
			dam = 3 * dam + 15;
		}
	}

	return (dam);
}



/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
sint critical_norm(int weight, int plus, int dam)
{
	int i, k;

	/* Extract "blow" power */
	i = (weight + ((p_ptr->to_h + plus) * 5) + (p_ptr->lev * 3));

	/* Chance */
	if (randint(5000) <= i)
	{
		k = weight + randint(650);

		if (k < 400)
		{
			msg_print("It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 700)
		{
			msg_print("It was a great hit!");
			dam = 2 * dam + 10;
		}
		else if (k < 900)
		{
			msg_print("It was a superb hit!");
			dam = 3 * dam + 15;
		}
		else if (k < 1300)
		{
			msg_print("It was a *GREAT* hit!");
			dam = 3 * dam + 20;
		}
		else
		{
			msg_print("It was a *SUPERB* hit!");
			dam = ((7 * dam) / 2) + 25;
		}
	}
	else
	{
		k = weight + randint(650);

		if (k > 400)
		{
			if (k < 700)
			{
				msg_print("It was a weak hit.");
				dam = .6 * dam;
			}
			else if (k < 1000)
			{
				msg_print("It was a feeble hit.");
				dam = .3 * dam;
			}
			else
			{
				msg_print("It was an effeminate hit.");
				dam = 1;
			}
		}
	}	

	return (dam);
}



/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 *
 * Note that most brands and slays are x3, except Slay Animal (x2),
 * Slay Evil (x2), and Kill dragon (x5).
 */
sint tot_dam_aux(const object_type *o_ptr, s32b tdam, const monster_type *m_ptr)
{
	int mult = 10, bmult = 10;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/*
	 * Some weapons and ammo do extra damage.
	 *
	 * Slays now do 1.5x/2x/3x total damage, not base.
	 * Brands do 1.5x total. Slays are culmultive with brand; the
	 * best of each takes effect. 
	 */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			/* Slay Animal */
			if ((f1 & (TR1_SLAY_ANIMAL)) &&
			    (r_ptr->flags3 & (RF3_ANIMAL)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_ANIMAL);
				}

				if (mult < 15) mult = 15;
			}

			/* Slay Evil */
			if ((f1 & (TR1_SLAY_EVIL)) &&
			    (r_ptr->flags3 & (RF3_EVIL)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_EVIL);
				}

				if (mult < 15) mult = 15;
			}

			/* Slay Undead */
			if ((f1 & (TR1_SLAY_UNDEAD)) &&
			    (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_UNDEAD);
				}

				if (mult < 20) mult = 20;
			}

			/* Slay Demon */
			if ((f1 & (TR1_SLAY_DEMON)) &&
			    (r_ptr->flags3 & (RF3_DEMON)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_DEMON);
				}

				if (mult < 20) mult = 20;
			}

			/* Slay Orc */
			if ((f1 & (TR1_SLAY_ORC)) &&
			    (r_ptr->flags3 & (RF3_ORC)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_ORC);
				}

				if (mult < 20) mult = 20;
			}

			/* Slay Troll */
			if ((f1 & (TR1_SLAY_TROLL)) &&
			    (r_ptr->flags3 & (RF3_TROLL)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_TROLL);
				}

				if (mult < 20) mult = 20;
			}

			/* Slay Giant */
			if ((f1 & (TR1_SLAY_GIANT)) &&
			    (r_ptr->flags3 & (RF3_GIANT)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_GIANT);
				}

				if (mult < 20) mult = 20;
			}

			/* Slay Dragon */
			if ((f1 & (TR1_SLAY_DRAGON)) &&
			    (r_ptr->flags3 & (RF3_DRAGON)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_DRAGON);
				}

				if (mult < 20) mult = 20;
			}

			/* Execute Dragon */
			if ((f1 & (TR1_KILL_DRAGON)) &&
			    (r_ptr->flags3 & (RF3_DRAGON)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_DRAGON);
				}

				if (mult < 30) mult = 30;
			}

			/* Execute demon */
			if ((f1 & (TR1_KILL_DEMON)) &&
			    (r_ptr->flags3 & (RF3_DEMON)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_DEMON);
				}

				if (mult < 5) mult = 5;
			}

			/* Execute undead */
			if ((f1 & (TR1_KILL_UNDEAD)) &&
			    (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_UNDEAD);
				}

				if (mult < 5) mult = 5;
			}

                       /* Brand (Nether) */
                       if (f2 & (TR2_BRAND_NETHER))
                       {
                               /* Notice immunity */
                               if ((r_ptr->flags4 & (RF4_BR_NETH)) ||
 (r_ptr->flags3 && (RF3_UNDEAD)))
                               {
                                       /* Oh, never mind */
                               }

                               /* Otherwise, take the damage */
                               else
                               {
                                       if (bmult < 15) bmult = 15;
                               }
                       }

                       /* Brand (Nexus) */
                       if (f2 & (TR2_BRAND_NEXUS))
                       {
                               /* Notice immunity */
                               if (r_ptr->flags4 & (RF4_BR_NEXU))
                               {
                                       /* Oh, never mind */
                               }

                               /* Otherwise, take the damage */
                               else
                               {
                                       if (bmult < 15) bmult = 15;
                               }
                       }

                       /* Brand (Chaos) */
                       if (f2 & (TR2_BRAND_CHAOS))
                       {
                               /* Notice immunity */
                               if (r_ptr->flags4 & (RF4_BR_CHAO))
                               {
                                       /* Oh, never mind */
                               }

                               /* Otherwise, take the damage */
                               else
                               {
                                       if (bmult < 15) bmult = 15;
                               }
                       }


			/* Brand (Acid) */
			if (f2 & (TR2_BRAND_ACID))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_ACID))
				{
					if (m_ptr->ml)
					{
						l_ptr->flags3 |= (RF3_IM_ACID);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if (bmult < 15) bmult = 15;
				}
			}

			/* Brand (Elec) */
			if (f2 & (TR2_BRAND_ELEC))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_ELEC))
				{
					if (m_ptr->ml)
					{
						l_ptr->flags3 |= (RF3_IM_ELEC);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if (bmult < 15) bmult = 15;
				}
			}

			/* Brand (Fire) */
			if (f2 & (TR2_BRAND_FIRE))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_FIRE))
				{
					if (m_ptr->ml)
					{
						l_ptr->flags3 |= (RF3_IM_FIRE);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if (bmult < 15) bmult = 15;
				}
			}

			/* Brand (Cold) */
			if (f2 & (TR2_BRAND_COLD))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_COLD))
				{
					if (m_ptr->ml)
					{
						l_ptr->flags3 |= (RF3_IM_COLD);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if (bmult < 15) bmult = 15;
				}
			}

			/* Brand (Poison) */
			if (f2 & (TR2_BRAND_POIS))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_POIS))
				{
					if (m_ptr->ml)
					{
						l_ptr->flags3 |= (RF3_IM_POIS);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if (bmult < 15) bmult = 15;
				}
			}

			break;
		}
	}


	/* Return the total damage */
	return (tdam * (mult + bmult - 10) / 10);
}


/*
 * Search for hidden things
 */
void search(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, chance;

	object_type *o_ptr;


	/* Start with base search ability */
	chance = p_ptr->skill_srh;

	/* Penalize various conditions */
	if (p_ptr->blind || no_light()) chance = chance / 10;
	if (p_ptr->confused || p_ptr->image) chance = chance / 10;

	/* Search the nearby grids, which are always in bounds */
	for (y = (py - 1); y <= (py + 1); y++)
	{
		for (x = (px - 1); x <= (px + 1); x++)
		{
			/* Sometimes, notice things */
			if (rand_int(100) < chance)
			{
				/* Invisible trap */
				if (cave_feat[y][x] == FEAT_INVIS)
				{
					/* Pick a trap */
					pick_trap(y, x);

					/* Message */
					msg_print("You have found a trap.");

					/* Disturb */
					disturb(0, 0);
				}

				/* Secret door */
				if (cave_feat[y][x] == FEAT_SECRET)
				{
					/* Message */
					msg_print("You have found a secret door.");

					/* Pick a door */
					place_closed_door(y, x);

					/* Disturb */
					disturb(0, 0);
				}

				/* Scan all objects in the grid */
				for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
				{
					/* Skip non-chests */
					if (o_ptr->tval != TV_CHEST) continue;

					/* Skip disarmed chests */
					if (o_ptr->pval <= 0) continue;

					/* Skip non-trapped chests */
					if (!chest_traps[o_ptr->pval]) continue;

					/* Identify once */
					if (!object_known_p(o_ptr))
					{
						/* Message */
						msg_print("You have discovered a trap on the chest!");

						/* Know the trap */
						object_known(o_ptr);

						/* Notice it */
						disturb(0, 0);
					}
				}
			}
		}
	}
}


/*
 * Determine if the object can be picked up, and has "=g" in its inscription.
 */
static bool auto_pickup_okay(const object_type *o_ptr)
{
	cptr s;

	/* It can't be carried */
	if (!inven_carry_okay(o_ptr)) return (FALSE);

	/* No inscription */
	if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = strchr(quark_str(o_ptr->note), '=');

	/* Process inscription */
	while (s)
	{
		/* Auto-pickup on "=g" */
		if (s[1] == 'g') return (TRUE);

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Don't auto pickup */
	return (FALSE);
}


/*
 * Helper routine for py_pickup() and py_pickup_floor().
 *
 * Add the given dungeon object to the character's inventory.
 *
 * Delete the object afterwards.
 */
static void py_pickup_aux(int o_idx)
{
	int slot;

	char o_name[80];
	object_type *o_ptr;

	o_ptr = &o_list[o_idx];

	/* Carry the object */
	slot = inven_carry(o_ptr);

	/* Get the object again */
	o_ptr = &inventory[slot];

	/* Describe the object */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("You have %s (%c).", o_name, index_to_label(slot));

	/* Delete the object */
	delete_object_idx(o_idx);
}


/*
 * Make the player carry everything in a grid.
 *
 * If "pickup" is FALSE then only gold will be picked up.
 */
void py_pickup(int pickup)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	s16b this_o_idx, next_o_idx = 0;

	object_type *o_ptr;

	char o_name[80];

#ifdef ALLOW_EASY_FLOOR

	int last_o_idx = 0;

	int can_pickup = 0;
	int not_pickup = 0;

#endif /* ALLOW_EASY_FLOOR */


	/* Scan the pile of objects */
	for (this_o_idx = cave_o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Hack -- disturb */
		disturb(0, 0);

		/* Pick up gold */
		if (o_ptr->tval == TV_GOLD)
		{
			/* Message */
			msg_format("You have found %ld gold pieces worth of %s.",
			           (long)o_ptr->pval, o_name);

			/* Collect the gold */
			p_ptr->au += o_ptr->pval;

			/* Redraw gold */
			p_ptr->redraw |= (PR_GOLD);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

			/* Delete the gold */
			delete_object_idx(this_o_idx);

			/* Check the next object */
			continue;
		}

		/* Test for auto-pickup */
		if (auto_pickup_okay(o_ptr))
		{
			/* Pick up the object */
			py_pickup_aux(this_o_idx);

			/* Check the next object */
			continue;
		}

#ifdef ALLOW_EASY_FLOOR

		/* Easy Floor */
		if (easy_floor)
		{
			/* Pickup if possible */
			if (pickup && inven_carry_okay(o_ptr))
			{
				/* Pick up if allowed */
				if (!carry_query_flag)
				{
					/* Pick up the object */
					py_pickup_aux(this_o_idx);
				}

				/* Else count */
				else
				{
					/* Remember */
					last_o_idx = this_o_idx;

					/* Count */
					++can_pickup;
				}
			}

			/* Else count */
			else
			{
				/* Remember */
				last_o_idx = this_o_idx;

				/* Count */
				++not_pickup;
			}

			/* Check the next object */
			continue;
		}

#endif /* ALLOW_EASY_FLOOR */

		/* Describe the object */
		if (!pickup)
		{
			msg_format("You see %s.", o_name);

			/* Check the next object */
			continue;
		}

		/* Note that the pack is too full */
		if (!inven_carry_okay(o_ptr))
		{
			msg_format("You have no room for %s.", o_name);

			/* Check the next object */
			continue;
		}

		/* Query before picking up */
		if (carry_query_flag)
		{
			char out_val[160];
			sprintf(out_val, "Pick up %s? ", o_name);
			if (!get_check(out_val)) continue;
		}

		/* Pick up the object */
		py_pickup_aux(this_o_idx);
	}

#ifdef ALLOW_EASY_FLOOR

	/* Easy floor, objects left */
	if (easy_floor && (can_pickup + not_pickup > 0))
	{
		/* Not picking up */
		if (!pickup)
		{
			/* One object */
			if (not_pickup == 1)
			{
				/* Get the object */
				o_ptr = &o_list[last_o_idx];

				/* Describe the object */
				object_desc(o_name, o_ptr, TRUE, 3);

				/* Message */
				msg_format("You see %s.", o_name);
			}

			/* Multiple objects */
			else
			{
				/* Message */
				msg_format("You see a pile of %d objects.", not_pickup);
			}

			/* Done */
			return;
		}

		/* No room */
		if (!can_pickup)
		{
			/* One object */
			if (not_pickup == 1)
			{
				/* Get the object */
				o_ptr = &o_list[last_o_idx];

				/* Describe the object */
				object_desc(o_name, o_ptr, TRUE, 3);

				/* Message */
				msg_format("You have no room for %s.", o_name);
			}

			/* Multiple objects */
			else
			{
				/* Message */
				msg_print("You have no room for any of the objects on the floor.");
			}

			/* Done */
			return;
		}

		/* Pick up objects */
		while (1)
		{
			cptr q, s;

			int item;

			/* Restrict the choices */
			item_tester_hook = inven_carry_okay;

			/* Get an object*/
			q = "Get which item? ";
			s = NULL;
			if (!get_item(&item, q, s, (USE_FLOOR))) break;

			/* Pick up the object */
			py_pickup_aux(0 - item);
		}
	}

#endif /* ALLOW_EASY_FLOOR */

}


/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static bool check_hit(int power)
{
	return test_hit(power, p_ptr->ac + p_ptr->to_a, TRUE);
}


/*
 * Handle player hitting a real trap
 */
void hit_trap(int y, int x)
{
	int i, num, dam;

	cptr name = "a trap";


	/* Disturb the player */
	disturb(0, 0);

	/* Analyze XXX XXX XXX */
	switch (cave_feat[y][x])
	{
		case FEAT_TRAP_HEAD + 0x00:
		{
			msg_print("You fall through a trap door!");
			if (p_ptr->ffall)
			{
				msg_print("You float gently down to the next level.");
			}
			else
			{
				dam = damroll(2, 8);
				take_hit(dam, name);
			}

			/* New depth */
			p_ptr->depth++;

			/* Leaving */
			p_ptr->leaving = TRUE;

			break;
		}

		case FEAT_TRAP_HEAD + 0x01:
		{
			msg_print("You fall into a pit!");
			if (p_ptr->ffall)
			{
				msg_print("You float gently to the bottom of the pit.");
			}
			else
			{
				dam = damroll(2, 6);
				take_hit(dam, name);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x02:
		{
			msg_print("You fall into a spiked pit!");

			if (p_ptr->ffall)
			{
				msg_print("You float gently to the floor of the pit.");
				msg_print("You carefully avoid touching the spikes.");
			}

			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (rand_int(100) < 50)
				{
					msg_print("You are impaled!");

					dam = dam * 2;
					(void)set_cut(p_ptr->cut + randint(dam));
				}

				/* Take the damage */
				take_hit(dam, name);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x03:
		{
			msg_print("You fall into a spiked pit!");

			if (p_ptr->ffall)
			{
				msg_print("You float gently to the floor of the pit.");
				msg_print("You carefully avoid touching the spikes.");
			}

			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (rand_int(100) < 50)
				{
					msg_print("You are impaled on poisonous spikes!");

					dam = dam * 2;
					(void)set_cut(p_ptr->cut + randint(dam));

					if (p_ptr->resist_cur[RES_ACID] > 20)
					{
						msg_print("The poison does not affect you!");
					}
					else
					{
						dam = dam * 2;
						(void)set_poisoned(p_ptr->poisoned + randint(dam));
					}
				}

				/* Take the damage */
				take_hit(dam, name);
			}

			break;
		}

		case FEAT_TRAP_HEAD + 0x04:
		{
			msg_print("You are enveloped in a cloud of smoke!");
			cave_info[y][x] &= ~(CAVE_MARK);
			cave_set_feat(y, x, FEAT_FLOOR);
			num = 2 + randint(3);
			for (i = 0; i < num; i++)
			{
				(void)summon_specific(y, x, p_ptr->depth, 0);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x05:
		{
			msg_print("You hit a teleport trap!");
			teleport_player(100);
			break;
		}

		case FEAT_TRAP_HEAD + 0x06:
		{
			msg_print("You are enveloped in flames!");
			dam = damroll(4, 6);
			fire_dam(dam, "a fire trap");
			break;
		}

		case FEAT_TRAP_HEAD + 0x07:
		{
			msg_print("You are splashed with acid!");
			dam = damroll(4, 6);
			acid_dam(dam, "an acid trap");
			break;
		}

		case FEAT_TRAP_HEAD + 0x08:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)set_slow(p_ptr->slow + rand_int(20) + 20);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x09:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_STR);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0A:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_DEX);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0B:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_CON);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0C:
		{
			msg_print("You are surrounded by a black gas!");
			if (!p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr->blind + rand_int(50) + 25);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0D:
		{
			msg_print("You are surrounded by a gas of scintillating colors!");
			if (!resist_check(RES_CONF))
			{
				(void)set_confused(p_ptr->confused + rand_int(20) + 10);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0E:
		{
			msg_print("You are surrounded by a pungent green gas!");
			if (p_ptr->resist_cur[RES_POIS] > 15)
			{
				(void)set_poisoned(p_ptr->poisoned + rand_int(20) + 10);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0F:
		{
			msg_print("You are surrounded by a strange white mist!");
			if (!p_ptr->free_act)
			{
				(void)set_paralyzed(p_ptr->paralyzed + rand_int(10) + 5);
			}
			break;
		}
	}
}



/*
 * Attack the monster at the given location
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
int py_attack(int y, int x)
{
	int num = 0, k, bonus, chance;

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;

	object_type *o_ptr;

	char m_name[80];

	bool fear = FALSE;

	bool do_quake = FALSE;

	bool sneak_attack = FALSE;

	/* Get the monster */
	m_ptr = &m_list[cave_m_idx[y][x]];
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];


	/* Disturb the player */
	disturb(0, 0);

	/* Check for sneak attack */
	if (m_ptr->csleep)
		sneak_attack = TRUE;

	/* Disturb the monster */
	m_ptr->csleep = 0;


	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);


	/* Auto-Recall if possible and visible */
	if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

	/* Track a new monster */
	if (m_ptr->ml) health_track(cave_m_idx[y][x]);


	/* Handle player fear */
	if (p_ptr->afraid)
	{
		/* Message */
		msg_format("You are too afraid to attack %s!", m_name);

		/* Done */
		return (p_ptr->num_blow);
	}


	/* Get the weapon */
	o_ptr = &inventory[INVEN_WIELD];

	/* Calculate the "attack quality" */
	bonus = p_ptr->to_h + o_ptr->to_h;
	chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));


	/* Attack once for each legal blow */
	while (num++ < p_ptr->num_blow)
	{
		/* Test for hit */
		if (test_hit(chance, r_ptr->ac, m_ptr->ml))
		{
			/* Message */
			message_format(MSG_HIT, m_ptr->r_idx, "You hit %s.", m_name);

			/* Hack -- bare hands do one damage */
			k = 1;

			/* Handle normal weapon */
			if (o_ptr->k_idx)
			{
				k = damroll(o_ptr->dd, o_ptr->ds);
				if (p_ptr->impact && (k > 50)) do_quake = TRUE;
				k = critical_norm(o_ptr->weight, o_ptr->to_h, k);
				k += o_ptr->to_d;

				/* Slays are applied after plus to dam from weapon */
				k = tot_dam_aux(o_ptr, k, m_ptr);

				/* Sneak attacks are cool */
				if (sneak_attack)
				{
					sneak_attack = FALSE;
					k = k * 6 / (2 + rand_int(3)); 
				}
			}

			/* Apply the player damage bonuses */
			k += p_ptr->to_d;

			/* No negative damage */
			if (k < 0) k = 0;

                       /* Polymorph attack -- monsters get a saving throw (chaos
 breathers are immune) */
                       if (p_ptr->chaos_brand && !(r_ptr->flags4 & RF4_BR_CHAO)
 &&
                               rand_int(49) == 0 && randint(90) > r_ptr->level)
                       {
                               int tmp;

                               /* Pick a "new" monster race */
                               tmp = poly_r_idx(m_ptr->r_idx);

                               /* Handle polymorh */
                               if (tmp != m_ptr->r_idx)
                               {
                                       /* Monster polymorphs */
                                       msg_format("%^s changes!", m_name);

                                       /* "Kill" the "old" monster */
                                       delete_monster_idx(cave_m_idx[y][x]);

                                       /* Create a new monster (no groups) */
                                       (void)place_monster_aux(y, x, tmp, FALSE,
 FALSE);

                                       /* Hack -- Assume success XXX XXX XXX */

                                       /* Hack -- Get new monster */
                                       m_ptr = &m_list[cave_m_idx[y][x]];

                                       /* Hack -- Get new race */
                                       r_ptr = &r_info[m_ptr->r_idx];

                                       /* Hack -- Get new description */
                                       monster_desc(m_name, m_ptr, 0);
                               }

                               break;
                       }

			/* Complex message */
			if (p_ptr->wizard)
			{
				msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
			}

                       /* Nether heals the player some (nether breathers, demons and undead are immune) */
                       if (p_ptr->nethr_brand && k > 0 && !(r_ptr->flags3 & RF3_UNDEAD) &&
                               !(r_ptr->flags3 & RF3_DEMON) && !(r_ptr->flags4 & RF4_BR_NETH))
                       {
                               if (k > m_ptr->hp)
                                       hp_player(m_ptr->hp / 3);
                               else
                                       hp_player(k / 3);
                       }

			/* Damage, check for fear and death */
			if (mon_take_hit(cave_m_idx[y][x], k, &fear, NULL)) break;

			/* Confusion attack */
			if (p_ptr->confusing || (p_ptr->chaos_brand && rand_int(7) == 0))
			{
				/* Cancel glowing hands */
                               if (p_ptr->confusing && rand_int(3) == 0)
                               {
                                       /* Cancel glowing hands */
                                       p_ptr->confusing = FALSE;

					/* Message */
					msg_print("Your hands stop glowing.");
				}

				/* Confuse the monster */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (m_ptr->ml)
					{
						l_ptr->flags3 |= (RF3_NO_CONF);
					}

					msg_format("%^s is unaffected.", m_name);
				}
				else if (rand_int(100) < r_ptr->level)
				{
					msg_format("%^s is unaffected.", m_name);
				}
				else
				{
					msg_format("%^s appears confused.", m_name);
					m_ptr->confused += 10 + rand_int(p_ptr->lev) / 5;
				}
			}

                       /* Fear attack (nether breathers & undead are immune) */
                       if (p_ptr->nethr_brand && !(r_ptr->flags4 & RF4_BR_NETH) &&
                               !(r_ptr->flags3 & RF3_UNDEAD) && !(r_ptr->flags3 & (RF3_NO_FEAR)) &&
                               rand_int(13) == 0)
                       {
                               int tmp;

                               if (m_ptr->monfear == 0)
                                       msg_format("%^s flees in terror!", m_name);

                               /* Increase fear */
                               tmp = m_ptr->monfear + 13 + randint(13);

                               /* Set fear */
                               m_ptr->monfear = (tmp < 200) ? tmp : 200;
                       }

                       /* Teleport attack (nexus breathers are immune) */
                       if (p_ptr->nexus_brand && !(r_ptr->flags4 & RF4_BR_NEXU) && rand_int(9) == 0)
                       {
                               msg_format("%^s disappears!", m_name);
                               if (rand_int(9) < 2)
                                       teleport_away(cave_m_idx[y][x], 200);
                               else
                                       teleport_away(cave_m_idx[y][x], 10);

                               /* Can't do anything after it's teleported */
                               break;
                       }


		}

		/* Player misses */
		else
		{
			/* Message */
			message_format(MSG_MISS, m_ptr->r_idx, "You miss %s.", m_name);
		}
	}


	/* Hack -- delay fear messages */
	if (fear && m_ptr->ml)
	{
		/* Message */
		message_format(MSG_FLEE, m_ptr->r_idx, "%^s flees in terror!", m_name);
	}


	/* Mega-Hack -- apply earthquake brand */
	if (do_quake) earthquake(p_ptr->py, p_ptr->px, 10);

	/* Return number of blows taken */
	return (num);
}





/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should only be called when energy has been expended.
 *
 * Note that this routine handles monsters in the destination grid,
 * and also handles attempting to move into walls/doors/rubble/etc.
 */
void move_player(int dir, int jumping)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y = 0, x = 0;

	bool pass_walls = p_ptr->pass_walls;

	/* Find the result of moving */
	y = py + ddy[dir];
	x = px + ddx[dir];

	/* Make sure the player can't move out of bounds */
	if ((cave_feat[y][x] >= FEAT_PERM_EXTRA) &&
	    (cave_feat[y][x] <= FEAT_PERM_SOLID))
	{
		pass_walls = FALSE;
	}

	/* Attack monsters in the way */
	if (cave_m_idx[y][x] > 0)
	{
		int attacks;

		/* Attack */
		attacks = py_attack(y, x);

		/* Split blows */
		p_ptr->energy_use = 100 * attacks / p_ptr->num_blow;
	}

#ifdef ALLOW_EASY_ALTER

	/* Optionally alter known traps/doors on (non-jumping) movement */
	else if (easy_alter && !jumping &&
	         (cave_info[y][x] & (CAVE_MARK)) &&
	         (cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
	         (cave_feat[y][x] <= FEAT_DOOR_TAIL))
	{
		/* Not already repeating */
		if (!p_ptr->command_rep)
		{
			/* Hack -- Optional auto-repeat */
			if (always_repeat && (p_ptr->command_arg <= 0))
			{
				/* Repeat 99 times */
				p_ptr->command_rep = 99;

				/* Reset the command count */
				p_ptr->command_arg = 0;
			}
		}

		/* Alter */
		do_cmd_alter();
	}

#endif /* ALLOW_EASY_ALTER */

	/* Player can not walk through "walls" */
	else if (!cave_floor_bold(y, x) && !pass_walls)
	{
		/* Disturb the player */
		disturb(0, 0);

		/* Notice unknown obstacles */
		if (!(cave_info[y][x] & (CAVE_MARK)))
		{
			int i = rand_int(3);
			cptr mess = NULL;

			/* Introduce some randomness */
			if (i == 0) mess = "You stumble into something in the dark.";
			else if (i == 1) mess = "You feel something blocking your way.";
			else if (i == 0) mess = "Something is blocking your way.";

			/* Message the player */
			message(MSG_HITWALL, 0, mess);
		}

		/* Mention known obstacles */
		else
		{
			/* Rubble */
			if (cave_feat[y][x] == FEAT_RUBBLE)
			{
				message(MSG_HITWALL, 0, "There is a pile of rubble blocking your way.");
			}

			/* Closed door */
			else if (cave_feat[y][x] < FEAT_SECRET)
			{
				message(MSG_HITWALL, 0, "There is a door blocking your way.");
			}

			/* Wall (or secret door) */
			else
			{
				message(MSG_HITWALL, 0, "There is a wall blocking your way.");
			}
		}
	}

	/* Normal movement */
	else
	{
		/* Move player */
		monster_swap(py, px, y, x);

		/* New location */
		y = py = p_ptr->py;
		x = px = p_ptr->px;

		/* Searching */
		if ((p_ptr->skill_fos >= 50) ||
		    (0 == rand_int(50 - p_ptr->skill_fos)) ||
			(p_ptr->searching))
		{
			search();
		}

		/* Handle "objects" */
		py_pickup(jumping != always_pickup);

		/* Handle "store doors" */
		if ((cave_feat[y][x] >= FEAT_SHOP_HEAD) &&
		    (cave_feat[y][x] <= FEAT_SHOP_TAIL))
		{
			/* Disturb */
			disturb(0, 0);

			/* Hack -- Enter store */
			p_ptr->command_new = '_';

			/* Free turn XXX XXX XXX */
			p_ptr->energy_use = 0;
		}

		/* Discover invisible traps */
		else if (cave_feat[y][x] == FEAT_INVIS)
		{
			/* Disturb */
			disturb(0, 0);

			/* Message */
			msg_print("You found a trap!");

			/* Pick a trap */
			pick_trap(y, x);

			/* Hit the trap */
			hit_trap(y, x);
		}

		/* Set off an visible trap */
		else if ((cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
		         (cave_feat[y][x] <= FEAT_TRAP_TAIL))
		{
			/* Disturb */
			disturb(0, 0);

			/* Hit the trap */
			hit_trap(y, x);
		}
	}

	/* We are done. */
	return;
}
