
It's amazing how active development on idTech3 technology is in the right corners. I found a tutorial describing server-side-demos. More on my development experience here https://blog.briancullinan.com/article/server-side-demos.

I researched 3 main ways of recording demos. 1) Record a client demo for every individual client, but from the server's end (The Doctor's method). 2) Record the entire server state for every client (large files, lrq3000). 3) Record extra snapshot data from every client perspective (Cyrax' Multiview).

lrq3000 specifically discusses using Cyrax's multiview to record state, and combining that with their own server-side playback (currently, multiview only plays back on the client like a traditional demo). Achieving the best of both worlds. I haven't yet implemented this either, but it's a priority for story-based co-op play.

## Demo Features

  * Recording for every client, [TheDoctor's method](http://openarena.ws/board/index.php?topic=4437.0). 
  * Server-side demos, [lrq3000 implementation](https://github.com/lrq3000/ioq3-server-side-demos) recording entire server state and spectating playback
  * [Cyrax' multiview protocol](http://edawn-mod.org/forum/viewtopic.php?f=6&t=7) for viewing all clients from one demo file. 
  * MultiVM command `\load <ui,cgame,game>; \mvjoin`. `\mvtile` display views in a grid. [Using multiworld mod](../docs/multiworld.md)

## Demo TODO
  * Playing back dm_68/mv-demo files for all players
  * Fast forward, rewind 10 seconds using baseline indexes in demo files
  * Render a seek bar and controls on the client
  * Merge features with Movie Maker Edition, but using the client for some functions and not a QVM/Game code.
  
