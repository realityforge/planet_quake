
typedef struct {
  uint16_t mTime;
  uint16_t mDate;
  uint32_t crc;
  uint32_t cSize;
  uint32_t uSize;
  char    *fName;
  uint32_t offset;
} zipHeader;


static void SV_WriteFile(byte *file, int fileSize, zipHeader *header, fileHandle_t zip)
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
  FS_Write( 0x04034b50, 4, zip );
  FS_Write( 0x0001, 2, zip );
  FS_Write( 0x0000, 2, zip );
  FS_Write( 0x0008, 2, zip );
  FS_Write( header->mTime, 2, zip );
  FS_Write( header->mDate, 2, zip );
  if(!header->crc) {
    
  }
  FS_Write( header->crc, 4, zip );
  // TODO: compress
  FS_Write( header->cSize, 4, zip );
  header->uSize = fileSize;
  FS_Write( header->uSize, 4, zip );
  FS_Write( strlen(header->fName), 2, zip );
  FS_Write( 0x0000, 2, zip );
  FS_Write( header->fName, strlen(header->fName), zip );
  
}


static void SV_WriteFileCompressed(byte *file, int compressedSize, zipHeader *header, fileHandle_t zip)
{
  
}


static void SV_WriteCentralDirectory(zipHeader **headers, int numFiles, fileHandle_t zip)
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

static void SV_MakeZip(char *zipName)
{
  qtime_t ptm;
  Com_RealTime( &ptm );

  SV_WriteFile(file, );
  (ptm->tm_mday) + (32 * (ptm->tm_mon+1)) + (512 * year);
  (ptm->tm_sec/2) + (32* ptm->tm_min) + (2048 * (uLong)ptm->tm_hour);
}
