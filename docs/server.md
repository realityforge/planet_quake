## Server Features

This fork includes various experimental features. Most of them can be turned on and off during compiling because the feature is tagged with pre-compile `#if` statements to include the feature.
This is an incomplete list of features added.
Use `USE_FEATURE=1` with make to enable.

### USE_DEMO_CLIENTS
TheDoctors method of recording a demo file for every single client using `\set sv_autoRecord 1` cvar or the `\cl_record` command. This creates a separate `.dm_68` demo file for each client that connects and each match. Use `set sv_autoRecordThreshold .9` to set the accuracy threshold for saving the client recording, otherwise it will be automatically removed. Useful for admins to only record the most ub3r of players.

### USE_DEMO_SERVER
lrq3000 method of server side demos using `set sv_autoDemo 1` cvar or the `\demo_record` command. Works by sending entity states to every client while spectating. TODO: NPC demo clients instead of resizing max players. TODO: replay and record using the multiview format instead.

### USE_RECENT_EVENTS
Event streaming. When specific things happen on the server, rcon can query the list of events and get a JSON style response of what happened since the last time it was checked. E.g. `{"type":1,"value":"q3dm1"}` where type `1` is SV_EVENT_MAPCHANGE. This is also used for the discord integration, and rankings. TODO: piped output for "pushing" events (as opposed to polling). TODO: add location information capable of making a heatmap. TODO: add pipe output
It's also used for the Discord chat bot connector

### USE_PERSIST_CLIENT
Persistent client sessions, that is, saving and restoring client score/health/location after a full disconnect and reconnect, use `sv_clSession` to specify how many seconds a session is valid to be restored.
either giving clients 30 seconds to reconnect and keep their score, 
or saving client states for days/weeks as a part of a long adventure game or campaign

### USE_SERVER_ROLES
Roles based rcon access. Set up to 24 rcon passwords each with a specific role assigned to it. Set the names of the roles with `sv_roles` (default: admin referee moderator). Role names must be alphanumeric with spaces in between, up to 24 names can be specified. Roles are set with `sv_roleName` where Name one of the role names set in `sv_roles`. Role capabilities are set with the name of the console command, e.g. `ban kick map`. Authorized users can access the entire command, that is, if they have `exec` permissions they can execute ANY script. If you want to limit executions to specific commands or combinations an alias has to be used and the alias would be listed in the `sv_roleName` instead of the command.

### USE_REFEREE_CMDS
Referee commands include things like: 
`\freeze`, `\unfreeze` for server side pausing. 
disciplinary actions such as `\mute`, `\nofire`.
`\lock`, `\lockred`, `\lockblue` to lock team joining.

### USE_DYNAMIC_ZIP
Dynamically build zip files to transer to clients using `cl_lazyLoading`. Automatically repackages the most commonly used files out of a pk3 requested by each client. see [lazyloading.md](../docs/lazyloading.md) for more information.

### USE_ENGINE_TELE
Engine side teleporting option can convert voids to teleporters or
  allow players to teleport with key bindings, in any mod. You can also
  set your starting position using sv_teleStartPos, it can be named or xyz

### USE_MEMORY_MAPS
Build maps in memory with randomly or procedurally generated parts
make external lightmaps for lazy updating, see [procedural.md](../docs/procedural.md) for more information.


## Multigame

Multigame is a mod I've been working on that adds Runes and alternate fire modes and stuff, [games.md](../docs/games.md#game-features).


## TODO: Server-side GPU

I've been thinking about this concept to move VBO and any FPU calculation to the GPU using shaders and readPixels or PBOs to calculate the floating point values for the frame and return the results in the next frame when it arrives from the GPU. Somehow all the traces from a frame would have to be queued in another shader image, then the game would react to the results a frame later intead of procedurally. The GPU could also calculate trajectories automatically just from updating a variadic for time interpolation.

## TODOs

  * write displacement code like showing a black cat and shifting 32 units to the right and 
  gibbing like a telefrag if they end up inside a wall, "player was displaced."
  * Event history with demo streaming as a service in the browser for splicing all those sweet frags, SQS/Message Queue
  * Synchronized server/AI for offline and connection interruptions
  * Headless match streaming directly to OBS using multiview and fpipes where necessary. Streaming render commands?
