
My goal is to learn to generate some brushwork, so that I can easily mimick the theme of an existing map, but then rearrange the parts in many different ways.

Some maps I've generated with code. A cube based map where every hole is a portal to another cube, perfect for Rocket Arena style game play.

A maze map, using recursive division. Has 4 floors each with a different maze, and portals in between the floors. Guns and ammo are hidden randomly throughout.

A chutes and ladders map, based on Quake 1 Edge of Oblivion, has ramps between 2 pyramids and wind tunnels connecting it in between.

![F1](../docs/chutes.png?raw=true)

An F1 race track. Uses CEIC2000 as edge detection for a JPG image of a bird eye view of the Monaco race track to generate the proper brushwork to for a road.

![F1](../docs/f1.png?raw=true)

Atlantis arena, will be a special kind of water based map where players must swim through a giant globe of water to reach power-ups. Markers within the water will allow you to jettison in different directions, with a big open arena in the center.

![F1](../docs/aqua.png?raw=true)


## Procedural Features

  * Procedurally generated game content and maps, sv_bsp procedurally generates a map of any size and transfers to the client. 
  * Use `+map \*memory0` to see a generated skybox, the source `.map` and generated bsp file are stored in the game home path. 
  * Map splicing by min and max dimensions. 
    `+sv_bspRebuild 1 +set sv_bspMap 1 +set sv_bspSplice "0 -1000 -1000 3000 720 3000" +devmap q3dm1` to get all the vertexes leading up to the tounge in map 1
    `+set sv_bspSplice "-4032 -3264 -2048 -1664 -1024 -960" +devmap tig_ra3` for arena 0 loading map with water fall/missing fall
    `+set sv_bspSplice "-896 -1216 -64 1152 800 1056" +devmap tig_ra3` for arena 1 "Stolen Goods" aztec castle style
    `+set sv_bspSplice "-4096 -640 -1152 -1664 2192 2624" +devmap tig_ra3` for arena 2 "Succulent Subversion" small with tall trees and bouncepads
    `+set sv_bspSplice "368 904 -960 3584 4032 2304" +devmap tig_ra3` for arena 3 "Flat Ground" Shub Niggurath's shrine lava cave
    `+set sv_bspSplice "-256 -3968 -3648 3968 256 -2176" +devmap tig_ra3` for arena 4 "Quiet Times" big outdoor garden dojo

## TODOs

  * Cut all the maps in half, and find the color distance to make a white map, like ultra-brights for skins, but for maps and not bright. Save storage and memory by rendering the same model mirrored with a different shader palette. Perhaps the room in the middle merges the light better.
  * Cut up all maps and use as 3D geomtry assets to build new maps automatically or upgrade map content from prior campaigns
  * fix skybox detection for new splicing tool
  * use cut out tongue or slicing tool to make a new map with teleporters in between
  * Recompile every map with new lightmaps and higher res/upscaled images
  * Connect every map in one giant universe, after removing ceilings (for birds eye), simplifying geometry, and stretching walls slightly outwards (multiply every Z point/total Z as a factor of growth between ceiling and floor so all the walls slant outwards)
  * Recompile skyboxes with different colors (q3map_skylight) like based on time of day, to make it feel more realistic and melancholic like you've lost a whole day in the game even if you've only been playing an hour. Might need GauGAN
  * Create voxelized model on server aka "destructible model" and stream to client, replace using z-index?
  * Use destruction and voxelization on Quake 2 remake where player has to return to previous levels in ruins
  * Add checksum to skip download, also minimize download screen 
  * Remove dependency on client side changes by building a temporary pk3 on the server with zlib (or whatever) and allowing legacy clients to download from that
  * Fix multithreading so console can respond and cancel map generations like a process list/print queue, use this feature for faster map development.
  * Geometry scaling.
  * Change color of team models, bases, lights, fogs, flags, etc.
  * Save space and rendering by only storing half of the BSP file, then switching the shaders and rendering it twice for each side with a "warp zone" in between the colors.
  * Option to turn on and off carrying flags through portals.
  * Switch lava and acid or color of void for space maps, light all white maps.
  * Optional skybox per map setting to replace skybox and "look and feel".
  * Need a way to edit .map file in engine that feels more natural, possible using laser sight, ruler, sketchup style alignment, turn off mouse capture unless pressing shift or right click
