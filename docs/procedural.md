
My goal is to learn to generate some brushwork, so that I can easily mimick the theme of an existing map, but then rearrange the parts in many different ways.

Some maps I've generated with code. A cube based map where every hole is a portal to another cube, perfect for Rocket Arena style game play.

A maze map, using recursive division. Has 4 floors each with a different maze, and portals in between the floors. Guns and ammo are hidden randomly throughout.

A chutes and ladders map, based on Quake 1 Edge of Oblivion, has ramps between 2 pyramids and wind tunnels connecting it in between.

An F1 race track. Uses CEIC2000 as edge detection for a JPG image of a bird eye view of the Monaco race track to generate the proper brushwork to for a road.

Atlantis arena, will be a special kind of water based map where players must swim through a giant globe of water to reach power-ups. Markers within the water will allow you to jettison in different directions, with a big open arena in the center.



## Procedural Features

  * Procedurally generated game content and maps, sv_bsp procedurally generates a map of any size and transfers to the client. 
  * Use `+map \*memory0` to see a generated skybox, the source `.map` and generated bsp file are stored in the game home path. 
  

## TODOs

  * Connect virtual-fs for image loading
  * Create voxelized model on server aka "destructible model" and stream to client, replace using z-index?
  * Add checksum to skip download, also minimize download screen 
  * Remove dependency on client side changes by building a temporary pk3 on the server with zlib (or whatever) and allowing legacy clients to download from that
  * Fix multithreading so console can respond and cancel map generations like a process list/print queue, use this feature for faster map development.
