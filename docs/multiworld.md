
When I started on multiworld (Sep 30, 2019), I originally wanted to "split-up" Quake 2 maps, because they are large. As kind of a manual forced culling, and then stream the maps in mid game, where it would load 2 areas nearby, and as you move to the next area, a further area would load.

When I started researching Cyrax' multiview protocol, it dawned on me that this was the perfect match for loading multiple QVMs at the same time. It's just CPU cycles right? For my proof-of-concept, I changed about 3,000 lines of code, made all the replacements to turn every reference to a VM into an array of VMs. Since then, I've reversed all of my changes, and now this uses a few small pre-compiler templates to achieve the same goal, making it only about 300 lines of code changed from master. It is also mostly additions, so there should be no trouble merging future updates from master Q3e.

Multiworld serves multiple purposes. For example, now using multiview, you can view multiple players at the same time by loading multiple CGame QVMs, each with their own managed state information.  As kind of an administrative picture-in-picture. Or maybe there is a game play where you have a heads up display of your teammates, like some futuristic police body-camera.

Multiworld can also load multiple maps at the same time. Each load has it's own settings, and `g_gametype`. This isn't implemented, but I see in the future, maybe someone wants to play a death match, and someone else wants to play capture the flag, they can both be present and playing on the same server, the same map loaded multiple times, but they are playing 2 different games and interacting as obstacles for each other as if they were playing the same game.

Finally, I want to use multiworld for streaming game content to clients. Where the client would be completely unaware of content being loaded in the background. Then they could switch levels instantly, or pass through a door and be in another large area without being too hard on the graphics card or memory. Making idTech3 an "open-world" engine.


## Multiworld Features

  * Multiple map loader in parallel with teleport switch. Multiple QVM loading for supplemental UIs and multiview. Multiview for movie making, example `+spdevmap q3dm1 +activeAction "+wait 1000 +load cgame +wait +world 0 +tile -1 -1 +tile 0 +tile 1 +tile 0 0 0 +wait 100 +mvjoin"`  ![F1](../docs/multiworld.png?raw=true)
  * Multiple worlds at once `+devmap q3dm1 +wait +load game q3dm2 +set activeAction "+wait 500 +game 1 +wait 500 +game 0 +wait 100 +tile -1 -1 0 +tile 0 0 0 +tile 1 0 1 +tile 0 0 0"`. `dvr [clientnum] x y h w` command for setting a view in a specific location. ![F1](../docs/hall.png?raw=true)
  * Multiple demos at once. ![F1](../docs/demos.png?raw=true)
  * Multiworld working with bots and sounds, multi-world-demos. Drag and drop for multiQVM views https://www.youtube.com/watch?v=xvmdETvvBo8.
  * Multiworld portals https://www.youtube.com/watch?v=iGxOdTNv3uY
  * Multiworld entities rendering properly with `set sv_mvOmnipresent -1` for spectate mode or `set sv_mvOmnipresent 1` to occupy multiple worlds at the same time, useful for "mirror dimension" effects.


## Running Multiworld

  Server:
  ```
  ./build/debug-darwin-x86_64/quake3e +set fs_basepath /Applications/ioquake3 +set fs_game baseq3 +set sv_pure 0 +set net_enable 1 +set dedicated 2 +set bot_enable 1 +set activeAction \"load\ game\ q3dm2\" +spmap q3dm1
  ```

  then Client:

  ```
  ./build/debug-darwin-x86_64/quake3e +set fs_basepath /Applications/ioquake3 +set fs_game baseq3 +set sv_pure 0 +set net_enable 1 +set dedicated 0 +set cl_nodelta 1 +bind g \"game\;\ wait\ 10\;\ tile\ 0\ 0\ 0\;\ tile\ -1\ -1\ 1\;\ tile\ 0\ 0\ 0\" +bind h \"game\;\ world\ 1\;\" +bind j \"game\;\ wait\ 10\;\ tile\ 0\ 0\ 0\;\ tile\ 1\ 0\ 1\;\ tile\ 0\ 0\ 0\" +connect local.games
  ```

  Explanation: Server starts on map q3dm1, the activeAction is added and launches another single player VM on q3dm2. Dedicated is required with USE_LOCAL_DED to prevent automatically starting multiple processes.

  Client cl_nodelta is required for the time-being, deltas work but it causes server errors. Binds set the DVR for q3dm1 to `g` key, and q3dm2 to `h` key. Level only needs to load first time it's used.

### Portal Demo

  Launch options are much.
  ```
  ./build/debug-darwin-x86_64/quake3e_mw +set fs_basepath /Applications/ioquake3 +set fs_basegame baseq3 +set sv_pure 0 +set r_fullscreen 0 +set cl_nodelta 1 +set developer 1 +set cl_lazyLoad 0 +devmap tig_ra3_arena_0 +load game tig_ra3_arena_1 +load game tig_ra3_arena_2 +load game tig_ra3_arena_3 +load game tig_ra3_arena_4 +set sv_mvWorld 1 +set sv_memoryMaps 1 +set sv_bspRebuild 1 +set sv_bspLight 2 +set sv_mvOmnipresent -1
  ```


## TODO

  * Still something wrong with marks on walls in second world.
  * Use cm_entityString or something to communicate portal changes to multiple QVMs
  * Fix multiworld deltas... again... or make no-delta the requirement.
  * Fix scene rendering, skipping, and replaying entity cmds between frames. Not critical because I haven't seen a drop in FPS since I made the trWorld_t change.
  * Fix sv_mvOmnipresent 1, send all gamestates up-front or wait until network settles on client. When omnipresent changes, update all the clients on the server.
  * sv_mvOmnicient 1, allow Godmod enabled players to take a birds eye view of the arena and move people around like an all powerful being in the sky. So sac-relig
  * Multiworld CGame interaction, load all the cameras from entities by name using splines.c, and set the cameras in game using the camera API so that CGame is guaranteed to draw entities in place correctly. Automatically move cameras to load any map with the same lobby/rotating camera like that door in Doctor Strange.
  * Add rendering to a texture http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
  * Add `target_microphone` that speakers can target in different worlds, spatial and volume controls close to projected surfaces. Then go back and automatically add it for misc_portal_surface and misc_portal_cameras in an extra field.
  * Fix interpolation in portal using relative direction and distance from where the portal was triggered, i.e. enter top left, player and projectiles should come out top left of center.
  * Multiworld entities like rockets with trajectories.
  * Render UIs and cameras to a surface, UI needs absolute mouse input, play Q3 on a model computer inside Q3
  * Copy second weapon axis from GUNNM Oculus for even weirder multiworld interaction.
  * Additional perspectives in sv_snapshot for multiworld camera location
  * There is a bug in mw cgame API that interferes with Auth from mods like Urban Terror. 
  * There is a bug in multiworld renderer, trying to run normally and screen looks jittery with Urban Terror mod, but looks fine with baseq3a at 1000 FPS
  * Compatibility with Xonotic warp zones in maps which is just an automatically configured portal camera surface
  * Connect `scores` commands from all games so players show in the same game. Connect player scores/powerups/health/stats/etc from world to world using SV_RestoreClient?
  * Fix multiworld cvar system is severly broken.
  * Filesystem switching mask so multiple mods can be loaded at the same time
  * "demoMap" surface parm which renders demos to an arbitrary surface
  * Use scripting in kiosk mode multiple demo files at once
  * Add game options like switching maps at the end of round, moving around in other maps or moving to spectator or disconnecting. Options for transferring game stats between worlds.
  * Fix SAMEORIGIN spawn type location works but camera angles change.
  * Open multiple websocket ports connected to the same server, server accepts data from either socket but merges or ignores based on sequence just like normal UDP and deltas.
  * Stop crash after loading 10 maps with use_lazy_memory. Fix in cm_load, vm_create, renderer2, clear old bots? sound uses FindOldest? Multiworld hunking, so levels can be cleared individually?
  * Connect to multiple servers at the same time, closer with demo work
