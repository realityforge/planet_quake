
I have intentionally put off "game code" development because the possibilities are so endless. Game-code is separate from "engine-code". There is a hard line, a fixed set of API calls that the game code makes to the engine. Things like RegisterModel() is a call from the game to the engine to load a specific model file. The game doesn't care what format the model is in, just that it looks like the specific model it needs, the renderers job is to decode the model format. The same thing applies to every graphic, the game doesn't care if the graphic is PNG, JPEG, that is the engine's job to figure out.

The following features I've added to baseq3a without affecting the networking protocol.


## Game Features:

  * Automatically skip to multiplayer menu TODO: and that connects to first available command server that we can request a map command. TODO: Remove the auto-connect code from javascript.
  * Configurable vote options based on roles.
  * Opening a web page from a map trigger.
  * Freezing a player like freeze tag. TODO: referee only, add freezing to game dynamics. TODO: treat frozen player like spectator.
  * More configureable physics cvars.
  * Power-up item timers.
  * Damage plum for showing hit damage near players.
  * Armor piercing rails
  * Bouncing rockets
  * Infinite invisibility cloak with \cloak command
  * Ladders shader for mapping and loading UrT maps
  * Flame thrower, TODO: add model from web like Elon Musks invention
  * Vortex Grenades. TODO: add vortex visual like warping BFG from Quake 4
  * Working Grappling Hook. TODO: add bot support. TODO: add to character class like Major only. Anyone can pick up if she drops it.
  * Lightening discharge under water like Quake 1
  * Location Damage, hitting a player on specific body parts. Also, slows and breaks legs if you fall too far like UrT.
  * Advanced weapon switching order for clients to set which weapons upgrade when they pick up.
  * Vulnerable missiles, rockets can be shot down mid air.
  * Player classes, changing starting weapon based on character model. TODO: change speed, and how much ammo can be carried.
  * Weapon dropping, using the \drop command will eject current weapon. TODO: eject picked-up items, eject ammo, eject active power-up, eject runes, eject persistent power-ups like guard and returns to podium.
  * Anti-gravity boots with \boots command.
  * Flashlight and laser commands. TODO: add visual for laser sight like battlefield when you are being targeted right in the eye.
  * Centered weapon positioning option.
  * Progressive zooming, can stop at any zoom level.
  * Rotating doors in maps for UrT support.
  * Beheading with headshot detection for Railgun only.
  * Alt weapon fire, twice as fast POC.
  * Cluster grenades.
  * Homing rockets.
  * Spread-fire power-up.
  * Server-side insta-gib gameplay.
  * Bots can spawn-camp, where they use spawn points as objectives for moving around. This was necessary for insta-gib because items are removed from the map.
  * More modes of death - ring out takes a point away from the person who falls into the void and gives a point to the last person that did knock-back damage to the player that died. "Void death" detection if someone fall a distance and then was killed by a world trigger. "from the grave" mode of death - when a grenade goes off an kills another player, after the person was already killed.
  * 60 different runes with colors, icons, and abilities from the original Rune Quake. TODO: implement runes.
  * Portals with nice camera angles. Portal power-up can be placed anywhere, ground, mid-air, under water. Portal gun can replace the BFG with left and right click to place a portal on walls.

## TODO

  * Fix portal gun. Looking through a standing portal has a weird repetitive effect because of depth write or sorting or something. Turn off depth in shader or skip entity in tr_main.c? Measure/cache midpoint of portal model and use on floor and wall alignment. Fix corners by tracing in server for edges. Add NOPORTAL surfaceParm. Still take falling damage for landing on a portal. Projectiles through portals. Face wall portals like 5 degrees towards player away from original angle. No falling damage while holding the portal gun.
  * Infinite haste, how is this different than g_speed? Applies to only one player.
  * Boots that can climb steep slopes. 
  * Jump velocity as a part of anti-gravity boots. 
  * Power-ups cover entire body or just the gun setting from server and client setting. 
  * Add player state to UI QVM so multiple cursors can be shared, and players can see map and game configuration.
  * Add walk on walls, better movement than trem.
  * Live-action minimap
  * Ricochet mode where damage only applies after projectile bounces/splash damage
  * Showing enemy health and armor about their head like an RPG
  * Extra UI menus with multiQVM, for voting on maps and bitcoin setup, Instant replay, consolidate all VM UIs scoreboard/postgame/HUD/menus in to 1 UI system, replace the menu address with an API call.
  * Many mod support, compiling and playing lots of different game types, capture the flag with 3+ teams
  * Campaign mode, playing older engine content and playing as enemy characters, new AI for old enemies
