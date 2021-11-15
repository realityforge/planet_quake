
## Why is my brain backwards?

I came across a few major design decisions in my research where I would have done the opposite thing. I want to discuss them here to give people an idea of where this fork is going. I will try to list more design decisions here.

1) I've done tons of search for cool maps. I want to copy the geometry in maps, and use procedural programming to generate new, but similar, dungeons/puzzles/castles/team bases/etc. This episode from Quake 1 http://www.worch.com/category/quake/ took all the maps from the entire episode and put them all in 1 single file. The author said it took over 48 hours to compile lighting.

My opposite solution. Instead of combining all of the maps into 1 file, to me, it makes more sense to split the maps into smaller, more manage files, like a model file but for the entire room. Then make the transition between scenes and maps more seamless so the player never sees a loading screen. This would make it "feel" like it's all 1 map.

2) Browser Qfusion source code, there is a commit "Drop model based LOD support". https://github.com/Qfusion/qfusion/commit/3aa63147a4c14d6bc55f1b14072bbb541fcdd2c0  LOD stands for "level of detail". What this removed functionality did, it skips vertices (2x, 3x, 4x, etc) in a row so that models look less detailed. The textures on the model flatten out and they start to look kind of blocky (like Minecraft). This has the added bonus of speeding up rendering times.

Maybe there is some technical reason for this change, but my plan was to do the opposite. Instead of removing LODs, I'm expanding the functionality, where models can be compressed and simplified in the distance, and then when the player gets closer to an area, the level of detail is increased.

