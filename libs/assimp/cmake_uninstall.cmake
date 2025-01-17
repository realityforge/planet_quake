IF(NOT EXISTS "/Users/briancullinan/planet_quake/libs/assimp/install_manifest.txt")
  MESSAGE(FATAL_ERROR "Cannot find install manifest: \"/Users/briancullinan/planet_quake/libs/assimp/install_manifest.txt\"")
ENDIF(NOT EXISTS "/Users/briancullinan/planet_quake/libs/assimp/install_manifest.txt")

FILE(READ "/Users/briancullinan/planet_quake/libs/assimp/install_manifest.txt" files)
STRING(REGEX REPLACE "\n" ";" files "${files}")
FOREACH(file ${files})
  MESSAGE(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  EXEC_PROGRAM(
    "/usr/local/Cellar/cmake/3.20.3/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
    OUTPUT_VARIABLE rm_out
    RETURN_VALUE rm_retval
    )
  IF(NOT "${rm_retval}" STREQUAL 0)
    MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
  ENDIF(NOT "${rm_retval}" STREQUAL 0)
ENDFOREACH(file)
