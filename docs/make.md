
Make is just a programming language that reminds me of prolog.


## Make system

I've rewritten the make system. Looking at ioq3 and Q3e and the giant Makefile compared to the simplicity of [VitaQuake3](https://github.com/Rinnegatamante/vitaQuakeIII/blob/master/Makefile) make files, I decided to do something half way in between. I split all the make features up into seperate files and use common platform files for setting up libs and build configurations.

In the /make/ folder there are 4 main categories. build_*, game_*, lib_*, platform_*

### platform_* 
Configures LDFLAGS for client and building as a dynamic library for each of the following, win, unix, wasm, macos.

### libs_* 
Are inteded to build libraries that can be attached by the engine such as additional video codecs. It also includes all the source code and build tools for the entire modern development pipeline of Quake 3 assets.

### game_* 
Build game QVMs. I've included the source for a few modern total conversion mods in their current state at the time I copied them. The right thing would be to sub-module these repositories in the /games/ folder.

### build_* 
Build just the required components of the engine. This is to build engine parts, I was thinking to add here build modes for using the engine as a networking UDP protocol, or only using the QVM interface but allowing an extension of APIs and libraries that can be embedded into the QVM (i.e. anything that can be static compiled to C99 standard).


## Make examples

Build client:

make -f make/build_client.make debug

Build dedicated server:

make -f make/build_server.make release

Build portals:

make -f make/build_client.make USE_MULTIVM_CLIENT=1 USE_MULTIVM_CLIENT=1


