# Docker (NEW)

Build the image from this repository:

`docker build .` or `docker build --target ...`

The Dockerfile has been updated to use https://docs.docker.com/develop/develop-images/multistage-build/

All images extend from either:

```
FROM node:12.15-slim as serve-tools
FROM debian:bullseye-slim AS build-tools
```

For some reason, some essential compile tool was missing from node-slim. This is a list of all the build targets and their reasoning:

* build-tools - install all the build tools needed for both builds.
* build-latest - update the copy from cache to latest from github.
* build-ded - dedicated server build.
* build-js - quake js based emscripten build.

* serve-tools - installs just the stuff needed to run and copied compiled output.
* serve-content - just serve the content in the build directory.
* serve-quake3e - start a quake3e dedicated server with run options.
* repack - all tools needed to repack game content for web.
* full - baseq3 testing content with everything in latest.

Might be easier to grab latest from dockerhub:

`docker run -ti -p 8080:8080 -p 27960:27960/udp briancullinan/quake3e:full`

Add files to the container by attaching a volume containing pk3s:

`docker run -ti -p 8080:8080 -p 27960:27960/udp -v quake3/baseq3:/home/baseq3 --name quake3e briancullinan/quake3e:full`

Then store the converted files for future runs:

`docker commit quake3e`

Visit to view:

http://127.0.0.1:8080/?connect%20127.0.0.1

To copy the built dedicated server out of the docker container, probably should just use cross-compiling with make:

`docker cp quake3e:/tmp/build/planet_quake/build/release-linux-x86_64/quake3e.ded.x64 ./build/`
