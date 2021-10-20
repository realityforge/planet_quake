# Install script for directory: /Users/briancullinan/planet_quake/libs/minizip-ng

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/briancullinan/planet_quake/libs/minizip-ng/libminizip.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminizip.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminizip.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminizip.a")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake"
         "/Users/briancullinan/planet_quake/libs/minizip-ng/CMakeFiles/Export/lib/cmake/minizip/minizip.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "/Users/briancullinan/planet_quake/libs/minizip-ng/CMakeFiles/Export/lib/cmake/minizip/minizip.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "/Users/briancullinan/planet_quake/libs/minizip-ng/CMakeFiles/Export/lib/cmake/minizip/minizip-noconfig.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES
    "/Users/briancullinan/planet_quake/libs/minizip-ng/minizip-config-version.cmake"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/minizip-config.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_os.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_crypt.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_buf.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_mem.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_split.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_os.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_zip.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_zip_rw.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_libcomp.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_bzip.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_lzma.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_zstd.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_pkcrypt.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_strm_wzaes.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/mz_compat.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/zip.h"
    "/Users/briancullinan/planet_quake/libs/minizip-ng/unzip.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/briancullinan/planet_quake/libs/minizip-ng/minizip.pc")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/briancullinan/planet_quake/libs/minizip-ng/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
