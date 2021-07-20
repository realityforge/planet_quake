/*
===========================================================================
Copyright (C) 2020 Auriga

This file is part of AAS Bots, a modification of Tremulous

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

chat "ikunx"
{
	#include "teamplay.h"

	type "game_enter"
	{
		"Great, time to fight more inexperienced warriors.";
	}

	type "game_exit"
	{
		"Well it felt good to be in battle again.";
	}

	type "level_start"
	{
		4, "was a great place before they screwed it up.";
	}

	type "level_end"
	{
		"Could you take any longer.";
	}

	type "level_end_victory"
	{
		"Git gud scrub.";
	}

	type "level_end_lose"
	{
		"Well shi..";
	}

	type "hit_talking"
	{
		"Look asshole I was trying to hold a conversation!";
	}

	type "hit_nodeath"
	{
		TAUNT0;
		TAUNT1;
		TAUNT2;
	}

	type "hit_nokill"
	{
		HIT_NOKILL0;
		HIT_NOKILL1;
		HIT_NOKILL1;
	}

	type "death_telefrag"
	{
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED0;
	}

	type "death_cratered"
	{
		DEATH_FALLING0;
		DEATH_FALLING0;
		DEATH_FALLING0;
	}

	type "death_lava"
	{
		"Hey! I was just feeling cold.";
	}

	type "death_slime"
	{
		"Well it looked safer than that...";
	}

	type "death_drown"
	{
		"Forgot my ventilator.";
	}

	type "death_suicide"
	{
		DEATH_SUICIDE0;
		DEATH_SUICIDE1;
		DEATH_SUICIDE2;
	}

	type "death_lcannon"
	{
        "Can't you kill someone without that thing?";
        "Anyone can get a frag with that, you're not special.";
        DEATH_BFG1;
	}

	type "death_mdriver"
	{
		DEATH_RAIL1;
		DEATH_RAIL1;
		DEATH_RAIL0;
		 0, ", Those were standard rate in the war of 2046!";
	}

	type "death_insult"
	{
		"If I wasn't so old, you would've died first.";
		"Just great.";
		curse;
	}

	type "death_praise"
	{
		 "Maybe you are cut out for duty, ", 0, ".";
	}

	type "kill_rail"
	{
		DEATH_RAIL1;
		DEATH_RAIL0;
		DEATH_RAIL1;
	}

	type "kill_gauntlet"
	{
		KILL_GAUNTLET0;
		KILL_GAUNTLET1;
		KILL_GAUNTLET0;
	}

	type "kill_telefrag"
	{
		TELEFRAGGED0;
		TELEFRAGGED1;
		
	}

	type "kill_suicide"
	{
		
		TAUNT1;
		TAUNT0;
	}

	type "kill_insult"
	{
		"Back in my day scrubs like you wernt even chosen for the forces.";
		"Noob";
		KILL_EXTREME_INSULT;
		curse;
	}

	type "kill_praise"
	{
		D_PRAISE0;
		D_PRAISE1;
		D_PRAISE3;
	}

	type "random_insult"
	{
		 "We sacrifice weaklings like you to the sand worms, ", 0, ".";
	}

	type "random_misc"
	{
		"I miss my home planet.";
		"War is a terrible thing, unless its against untrained individuals like yourselves.";
	}
}