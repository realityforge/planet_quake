
// TODO: make a server side in memory rezipper that collects requests
//   from individual clients and creates a new zip (3 seconds)
//   create a zip with the servers q3journal so clients know which
//   files are available for download
//   zip everything in memory based on the offsets requests whether its
//   in between a file header or file data
// TODO: remove index.json functionality from WASM build
// TODO: call out to system(convert) and oggenc for converting images and audio
//   and building new web compatible formats, and cache conversion on disk
//   to not repeat the process
// TODO: use LODs and MIPs (hi,med,lo) copies replaced with zeros, then grandually transfer and
//   load new data

#ifdef USE_DYNAMIC_ZIP
#include "server.h"

typedef struct {
  uint16_t mTime;
  uint16_t mDate;
  uint32_t crc;
  uint32_t cSize;
  uint32_t uSize;
  char    *fName;
  uint32_t offset;
} zipHeader_t;


static void SV_WriteFile(fileHandle_t file, int fileSize, zipHeader_t *header, fileHandle_t zip)
{
  /*
  local file header signature     4 bytes  (0x04034b50)
  version needed to extract       2 bytes
  general purpose bit flag        2 bytes
  compression method              2 bytes
  last mod file time              2 bytes
  last mod file date              2 bytes
  crc-32                          4 bytes
  compressed size                 4 bytes
  uncompressed size               4 bytes
  file name length                2 bytes
  extra field length              2 bytes
  */
  FS_Write( "\0x04\0x03\0x4b\0x50", 4, zip );
  FS_Write( "\0x00\0x01", 2, zip );
  FS_Write( "\0x00\0x00", 2, zip );
  FS_Write( "\0x00\0x08", 2, zip ); //deflate
  FS_Write( va("%02i", header->mTime), 2, zip );
  FS_Write( va("%02i", header->mDate), 2, zip );
  if(!header->crc) {
    
  }
  FS_Write( va("%04i", header->crc), 4, zip );
  // TODO: use stream compression instead
  //compress(file, fileSize, )
  FS_Write( va("%04i", header->cSize), 4, zip );
  header->uSize = fileSize;
  FS_Write( va("%04i", header->uSize), 4, zip );
  FS_Write( va("%02i", (int)strlen(header->fName)), 2, zip );
  FS_Write( "\0x00\0x00", 2, zip );
  FS_Write( header->fName, strlen(header->fName), zip );
  
}


static void SV_WriteFileCompressed(fileHandle_t file, int compressedSize, zipHeader_t *header, fileHandle_t zip)
{
  
}


static void SV_WriteCentralDirectory(zipHeader_t **headers, int numFiles, fileHandle_t zip)
{
  /*
  central file header signature   4 bytes  (0x02014b50)
  version made by                 2 bytes
  version needed to extract       2 bytes
  general purpose bit flag        2 bytes
  compression method              2 bytes
  last mod file time              2 bytes
  last mod file date              2 bytes
  crc-32                          4 bytes
  compressed size                 4 bytes
  uncompressed size               4 bytes
  file name length                2 bytes
  extra field length              2 bytes
  file comment length             2 bytes
  disk number start               2 bytes
  internal file attributes        2 bytes
  external file attributes        4 bytes
  relative offset of local header 4 bytes

  file name (variable size)
  extra field (variable size)
  file comment (variable size)
  */
}

static void SV_MakeZip(/* char **zipFiles, int numFiles, */ char *zipName)
{
  // TODO: use memory streams instead, request a specific block inside a pk3
  //   probably won't work because compressed size needs to be known by server
  //   so files will need to be compressed anyways in order to return 
  //   an accurate central directory
  char *zipFiles[] = {
    "maps/megamaze.bsp"
  };
  int numFiles = ARRAY_LEN(zipFiles);

  // crc the file list for tmp name, 
  int totalNameLength = 0;
  for(int i = 0; i < numFiles; i++) {
    totalNameLength += strlen(zipFiles[i]);
  }
  uint32_t crc = crc32_buffer((const byte*)zipFiles, totalNameLength + numFiles /* for null terminator */);

  fileHandle_t zipFile = FS_FOpenFileWrite( va("%s.%i.pk3", zipName, crc) );
  
  for(int i = 0; i < numFiles; i++) {
    qtime_t ptm;
    zipHeader_t header;
    fileHandle_t file;
    int fileLen = 0;
    memset(&header, 0, sizeof(header));
    Com_RealTime( &ptm );
    // convert tmz to stupid dos time
    header.mDate = (ptm.tm_mday) + (32 * (ptm.tm_mon+1)) + (512 * ptm.tm_year);
    header.mTime = (ptm.tm_sec/2) + (32* ptm.tm_min) + (2048 * (uint32_t)ptm.tm_hour);
    if((fileLen = FS_FOpenFileRead(zipFiles[i], &file, qfalse)) == -1) {
      continue;
    }
    // TODO: copy file from pk3 into memory already compressed, 
    //   instead of decompressing and recompressing
    // TODO: FS_FileIsInPAK ? SV_WriteFileCompressed
    
    SV_WriteFile(file, fileLen, &header, zipFile);
    FS_FCloseFile(file);
  }
  // TODO: SV_WriteCentralDirectory
  // TODO: when pk3 is done, CRC whole pk3 for sv_pure
  FS_FCloseFile( zipFile );
}

#endif
