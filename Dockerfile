# try to make this more efficient
FROM debian:bullseye-slim AS build-tools

RUN \
  echo "# INSTALL BUILD DEPENDENCIES ##########################################" && \
  apt-get update && \
  apt upgrade -y && apt dist-upgrade && \
  apt-get install -y build-essential "linux-headers-*-common" libcurl4-gnutls-dev curl g++ gcc git make nodejs npm python3 python3-distutils vim && \
  mkdir -p /tmp

RUN \
  echo "# FETCH INSTALLATION FILES ######################################" && \
  cd /tmp && \
  git clone --recursive --progress https://github.com/briancullinan/planet_quake && \
  cd /tmp/planet_quake && \
  git submodule add -f git://github.com/emscripten-core/emsdk.git code/xquakejs/lib/emsdk && \
  git submodule update --init --recursive --progress

# copy the output from grabbing master and installing
RUN \
  echo "# INSTALL NODE AND EMSDK ######################################" && \
  cd /tmp/planet_quake && \
  npm install && \
  npm run install:emsdk && \
  echo "" > /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "BINARYEN_ROOT = '/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "LLVM_ROOT = '/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/bin'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "NODE_JS = '/usr/bin/node'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "EM_CACHE = '/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  mkdir -p /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache && \
  export EM_CACHE=/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache && \
  export EMSCRIPTEN_CACHE=/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache && \
  npm run install:libs

# update the copy from cache to latest from github
FROM build-tools AS build-latest

# TODO: checkout different branches for different experimental features
RUN \
  echo "# UPDATE SOURCE FILES ######################################" && \
  cd /tmp/planet_quake && \
  git status && \
  git pull && \
  cd /tmp/planet_quake/code/xquakejs/lib/emsdk && \
  git pull

RUN \
  echo "# UPDATE BUILD DEPS ######################################" && \
  cd /tmp/planet_quake && \
  echo "" > /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "BINARYEN_ROOT = '/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "LLVM_ROOT = '/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/bin'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "NODE_JS = '/usr/bin/node'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  echo "EM_CACHE = '/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache'" >> /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/.emscripten && \
  npm install && \
  npm run install:emsdk && \
  mkdir -p /tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache && \
  export EM_CACHE=/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache && \
  export EMSCRIPTEN_CACHE=/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache && \
  npm run install:libs

FROM build-latest AS build-ded

RUN \
  echo "# BUILD NATIVE SERVER ##########################################" && \
  cd /tmp/planet_quake && \
  make clean release BUILD_CLIENT=0 NOFPU=1

FROM build-latest AS build-js

RUN \
  echo "# BUILD JS CLIENT ##########################################" && \
  cd /tmp/planet_quake && \
  export STANDALONE=1 && \
  make clean release EMSCRIPTEN_CACHE=/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache PLATFORM=js

FROM node:15.12-slim as serve-tools

RUN \
  echo "# INSTALL CONTENT DEPENDENCIES #################################" && \
  apt-get update && \
  apt-get install -y git && \
  mkdir -p /home/baseq3 && \
  mkdir -p /tmp

RUN \
  echo "# FETCH RUN FILES #################################" && \
  cd /tmp && \
  git clone --progress https://github.com/briancullinan/planet_quake

FROM node:15.12-slim AS serve-content

RUN mkdir -p /tmp/planet_quake/code/xquakejs/bin
RUN mkdir -p /tmp/planet_quake/code/xquakejs/lib
RUN mkdir -p /tmp/planet_quake/build/release-js-js
RUN mkdir -p /tmp/planet_quake/build/release-linux-x86_64

COPY --from=serve-tools /tmp/planet_quake/package.json /tmp/planet_quake/package.json
COPY --from=serve-tools /tmp/planet_quake/code/xquakejs/bin /tmp/planet_quake/code/xquakejs/bin
COPY --from=serve-tools /tmp/planet_quake/code/xquakejs/lib /tmp/planet_quake/code/xquakejs/lib
COPY --from=build-js /tmp/planet_quake/build/release-js-js/quake3e* /tmp/planet_quake/build/release-js-js/
COPY --from=build-ded /tmp/planet_quake/build/release-linux-x86_64/quake3e* /tmp/planet_quake/build/release-linux-x86_64/

RUN \
  cd /tmp/planet_quake && \
  npm install --dev

EXPOSE 8080/tcp
EXPOSE 27960/udp
VOLUME [ "/home/baseq3" ]
ENV RCON=password123!
ENV GAME=baseq3
ENV BASEGAME=baseq3

CMD node /tmp/planet_quake/code/xquakejs/bin/web.js --temp /home

FROM serve-content AS serve-quake3e

CMD /tmp/planet_quake/build/release-linux-x86_64/quake3e.ded.x64 \
  +cvar_restart +set net_port 27960 +set fs_basepath /home \
  +set dedicated 2 +set fs_homepath /home \
  +set fs_basegame ${BASEGAME} +set fs_game ${GAME} \
  +set ttycon 0 +set rconpassword ${RCON} \
  +set logfile 2 +set com_hunkmegs 150 +set vm_rtChecks 0 \
  +set sv_maxclients 32 +set sv_pure 0 +exec server.cfg

FROM serve-content AS serve-both

CMD \
  (node /tmp/planet_quake/code/xquakejs/bin/web.js --temp /tmp &) && \
  /tmp/planet_quake/build/release-linux-x86_64/quake3e.ded.x64 \
    +cvar_restart +set net_port 27960 +set fs_basepath /home \
    +set dedicated 2 +set fs_homepath /home \
    +set fs_basegame ${BASEGAME} +set fs_game ${GAME} \
    +set ttycon 0 +set rconpassword ${RCON} \
    +set logfile 2 +set com_hunkmegs 150 +set vm_rtChecks 0 \
    +set sv_maxclients 32 +set sv_pure 0 +exec server.cfg

FROM serve-both AS repack

RUN \
  echo "# INSTALL REPACK DEPENDENCIES ##########################################" && \
  apt-get update && \
  apt-get install -y systemd imagemagick imagemagick-common vorbis-tools vim python && \
  cd /tmp/planet_quake && \
  npm install --dev

VOLUME [ "/home/baseq3" ]

CMD node /tmp/planet_quake/code/xquakejs/bin/repack.js --no-graph --no-overwrite --temp /home /home/baseq3

########### TODO REPACK DOCKER HERE ############
# needs a data source for baseq3 content, Github with demo data maybe?

FROM serve-both AS full

COPY --from=briancullinan/quake3e:baseq3 /home/baseq3-cc /home/baseq3
