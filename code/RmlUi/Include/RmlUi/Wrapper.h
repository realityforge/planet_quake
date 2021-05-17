
#ifndef __WRAPPER_H
#define __WRAPPER_H

#ifdef __cplusplus
#include "./Core/FileInterface.h"

extern "C" {

typedef struct FileInterface RmlFileInterface;

void Rml_SetFileInterface(FileInterface* file_interface);

}
#else
typedef struct {
  fileHandle_t (*Open)(const char *path);
  /// Closes a previously opened file.
  /// @param file The file handle previously opened through Open().
  void (*Close)(fileHandle_t file);

  /// Reads data from a previously opened file.
  /// @param buffer The buffer to be read into.
  /// @param size The number of bytes to read into the buffer.
  /// @param file The handle of the file.
  /// @return The total number of bytes read into the buffer.
  size_t (*Read)(void* buffer, size_t size, fileHandle_t file);
  /// Seeks to a point in a previously opened file.
  /// @param file The handle of the file to seek.
  /// @param offset The number of bytes to seek.
  /// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
  /// @return True if the operation completed successfully, false otherwise.
  qboolean (*Seek)(fileHandle_t file, long offset, int origin);
  /// Returns the current position of the file pointer.
  /// @param file The handle of the file to be queried.
  /// @return The number of bytes from the origin of the file.
  size_t (*Tell)(fileHandle_t file);

  /// Returns the length of the file.
  /// The default implementation uses Seek & Tell.
  /// @param file The handle of the file to be queried.
  /// @return The length of the file in bytes.
  size_t (*Length)(fileHandle_t file);

  /// Load and return a file.
  /// @param path The path to the file to load.
  /// @param out_data The string contents of the file.
  /// @return True on success.
  qboolean (*LoadFile)(const char *path, char *out_data);
} RmlFileInterface;

void Rml_SetFileInterface(RmlFileInterface* file_interface);
#endif


#endif
