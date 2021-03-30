FROM node:12.15-slim as serve-tools
FROM debian:bullseye-slim AS build-tools

RUN \
  echo "# INSTALL BUILD DEPENDENCIES ##########################################" && \
  apt-get update && \
  apt upgrade -y && apt dist-upgrade && \
  apt-get install -y build-essential "linux-headers-*-common" libcurl4-gnutls-dev curl g++ gcc git make nodejs npm python3 python3-distutils vim && \
  mkdir -p /tmp

# copy the output from grabbing master and installing
# This is to speed up building
FROM briancullinan/quake3e:build-tools AS build-cache

RUN \
  echo "# FETCH INSTALLATION FILES ######################################" && \
  cd /tmp && \
  git clone --recursive --progress https://github.com/briancullinan/planet_quake && \
  cd /tmp/planet_quake && \
  git submodule add -f git://github.com/emscripten-core/emsdk.git code/xquakejs/lib/emsdk && \
  git submodule update --init --recursive --progress
RUN \
  echo "# INSTALL NODE AND EMSDK ######################################" && \
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

# update the copy from cache to latest from github
FROM briancullinan/quake3e:build-tools AS build-latest

COPY --from=briancullinan/quake3e:build-cache /tmp/planet_quake /tmp/planet_quake
COPY --from=briancullinan/quake3e:build-cache /tmp/planet_quake/.git /tmp/planet_quake/.git

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

FROM briancullinan/quake3e:build-latest AS build-ded

RUN \
  echo "# BUILD NATIVE SERVER ##########################################" && \
  cd /tmp/planet_quake && \
  make clean release BUILD_CLIENT=0 NOFPU=1

FROM briancullinan/quake3e:build-js AS build-both

COPY --from=briancullinan/quake3e:build-ded /tmp/planet_quake/build/release-linux-x86_64 /tmp/planet_quake/build/release-linux-x86_64

FROM briancullinan/quake3e:build-latest AS build-js

RUN \
  echo "# BUILD JS CLIENT ##########################################" && \
  cd /tmp/planet_quake && \
  export STANDALONE=1 && \
  make clean release EMSCRIPTEN_CACHE=/tmp/planet_quake/code/xquakejs/lib/emsdk/upstream/emscripten/cache PLATFORM=js

FROM briancullinan/quake3e:serve-tools AS serve-content

COPY --from=briancullinan/quake3e:build-js /tmp/planet_quake /tmp/planet_quake

EXPOSE 8080/tcp
VOLUME [ "/tmp/baseq3" ]

CMD node /tmp/planet_quake/code/xquakejs/bin/web.js /assets/baseq3-cc /tmp/baseq3

FROM briancullinan/quake3e:serve-tools AS serve-quake3e

COPY --from=briancullinan/quake3e:build-ded /tmp/planet_quake /tmp/planet_quake

EXPOSE 27960/udp
VOLUME [ "/tmp/baseq3" ]
ENV RCON=password123!
ENV GAME=baseq3-cc
ENV BASEGAME=baseq3-cc

CMD /home/ioq3srv/planet_quake/quake3e.ded.x64 \
  +cvar_restart +set net_port 27960 +set fs_basepath /home/ioq3srv/.quake3 \
  +set dedicated 2 +set fs_homepath /home/ioq3srv \
  +set fs_basegame ${BASEGAME} +set fs_game ${GAME} \
  +set ttycon 0 +set rconpassword ${RCON} \
  +set logfile 2 +set com_hunkmegs 150 +set vm_rtChecks 0 \
  +set sv_maxclients 32 +set sv_pure 0 +exec server.cfg

FROM briancullinan/quake3e:serve-quake3e AS serve-both

COPY --from=briancullinan/quake3e:build-both /tmp/planet_quake /tmp/planet_quake

EXPOSE 8080/tcp

CMD /tmp/planet_quake/code/xquakejs/bin/start.sh

FROM briancullinan/quake3e:serve-tools AS repack

RUN \
  echo "# INSTALL REPACK DEPENDENCIES ##########################################" && \
  apt-get update && \
  apt-get install -y systemd imagemagick imagemagick-common vorbis-tools vim python

VOLUME [ "/tmp/baseq3" ]

CMD node /home/ioq3srv/quakejs/bin/repack.js --no-graph --no-overwrite /tmp/baseq3
