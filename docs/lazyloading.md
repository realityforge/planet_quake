## What is lazy loading?

I soon realized by down encoding graphics, then maximizing in game quality, I could get a similar "feel" from only 10-20% of an image. This meant, it would be faster to load the images into memory, but the quality on the GPU would be similar to the default, using 0-mips.

That means, I can load graphics quickly enough that it does it in-between frames. That also means, the graphics could be downloaded from a remote source, and loaded in-game whenever they arrive. Combine this feature with multiworld, and I can create an experience similar to MegaTexture where different quality graphics and models can be loaded based on the distance the player is from the content. Then the entire render will switch "worlds" as they get closer.


## Lazy Loading Features

  * Deferred (lazy) loading of all game content, entities, models, textures. 
  * New `cl_lazyLoad` cvar, 0 - turn off lazy loading, only load textures available on disk, 1 - load all textures available and fetch remotely as needed, 2 - set all to default and try to load from `sv_dlURL`, 3 - load textures only during INTERMISSION or dead or SPECTATING
  * Shader palettes for pre-rendering colors, that is, all graphics have an average color defined in a single palette `.shader` file, the alternate color is used while the true image is loading


## TODO

  * Fix check for files from renderer and return 0 if file isn't in index and return placeholder when file is queued for download
  * Distance based lazy loading, need to sort which graphics load first by the number of times it's displayed on screen
  * 4 - set all to default and load during intermission (this is specifically for subordinate VMs in multiVM/multi-render modes)
  * Leave files open for changing mip levels (especially on DDS)?
  * Lazy loading file streaming out of pk3 on server over UDP or cURL interface.
  * Changing the theme of maps at runtime using `if` in shaders
  * Download files using offsets out of pk3 files, like streaming a part of the zip file, add this to native dedicated server and UDP downloads, this won't work on Google CDN because there is no accept-ranges support with brotli compression, https://cloud.google.com/storage/docs/xml-api/get-object-download service in front of that needs to re-encode the individual file based on the offset provided with brotli.
