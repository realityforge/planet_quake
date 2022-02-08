#include "server.h"

#ifdef USE_MEMORY_MAPS

void CL_LoadJPG( const char *filename, unsigned char **pic, int *width, int *height );
void rgb2lab(int R, int G, int B, double *L, double *a, double *b);
float CIEDE2000(double l1, double a1, double b1, double l2, double a2, double b2);

static void SV_GetAreaBlocks(
  int areaX, int areaY,
  int AREA_BLOCK, 
  int maxWidth, int maxHeight, 
  float *diffStack, 
  vec2_t *blockStack, int *blockCount,
  int pointsTotal)
{
  int MAX_BLOCK = 32;
  int BLOCK_SIZE = 4;
  int a3;
  int w, h, x, y;
  int bx, by;
  qboolean skipArea;
  double sumArea;
  int sumTotal;
  // start by fitting the largest possible cube of uninterrupted road
  // removed the lower limit restriction when it works
  for(w = MAX_BLOCK; w > BLOCK_SIZE; w-=BLOCK_SIZE) {
    for(h = MAX_BLOCK; h > BLOCK_SIZE; h-=BLOCK_SIZE) {
  //for(int w = AREA_BLOCK; w > 0; w--) {
  //  for(int h = AREA_BLOCK; h > 0; h--) {
      // cube can fit anywhere between the edges of the area so subtract the
      //   width/height from the traversable x and y position
      for(x = 0; x < maxWidth - w; x+=BLOCK_SIZE) {
        for(y = 0; y < maxHeight - h; y+=BLOCK_SIZE) {
          // since we can only get smaller skip the cubes that are already formed
          skipArea = qfalse;
          for(a3 = 0; a3 < *blockCount; a3++) {
            // check if any of the corners intersect with another block
            if(isOverlapping(
              (vec2_t){areaX * AREA_BLOCK + x,     areaY * AREA_BLOCK + y}, 
              (vec2_t){areaX * AREA_BLOCK + x + w, areaY * AREA_BLOCK + y + h}, 
              blockStack[a3 * 2], blockStack[a3 * 2 + 1])) {
              skipArea = qtrue;
              break;
            }
          }
          if(skipArea) {
            continue;
          }
          
          // finally scan the whole space for average diff
          sumArea = 0;
          sumTotal = 0;
          for(bx = x; bx < x + w; bx++) {
            for(by = y; by < y + h; by++) {
              sumArea += diffStack[by * AREA_BLOCK + bx];
              sumTotal++;
            } // by
          } // bx
          
          if(sumArea / sumTotal < 1 && *blockCount < pointsTotal) {
            // we finally found a good block
            // make a road voxel at the position on blue pixels
            blockStack[(*blockCount)*2][0] = areaX * AREA_BLOCK + x;
            blockStack[(*blockCount)*2][1] = areaY * AREA_BLOCK + y;
            blockStack[(*blockCount)*2+1][0] = areaX * AREA_BLOCK + x + w;
            blockStack[(*blockCount)*2+1][1] = areaY * AREA_BLOCK + y + h;
            (*blockCount)++;
          }
          
          if(*blockCount >= pointsTotal) break;
        } // y
        if(*blockCount >= pointsTotal) break;
      } // x


      if(*blockCount >= pointsTotal) break;
    } // h
    if(*blockCount >= pointsTotal) break;
  } // w

}


static void SV_GetAreaPaths(
  int areaX, int areaY,
  int AREA_BLOCK, int PATH_WIDTH,
  int maxWidth, int maxHeight, 
  float *diffStack, 
  vec2_t *roadStack, int *roadCount,
  int pointsTotal)
{
  int x, y, px, py;
  uint64_t totalPointX = 0;
  uint64_t totalPointY = 0;
  int countPoint = 0;
  // dumb down the path by averaging blue road match pixels and saving center
  // since the path is no more than 8 pixels wide, there can be no more
  //   than 16x16 points per area if every square was filled with street
  //   probably there are no more than 32 points if the road circles back 
  //   around and crosses the same area
  for(x = 0; x <= maxWidth - PATH_WIDTH; x += PATH_WIDTH) {
    for(y = 0; y <= maxHeight - PATH_WIDTH; y += PATH_WIDTH) {
      totalPointX = 0;
      totalPointY = 0;
      countPoint = 0;

      // TODO: try the average algorithm here instead
      for(px = x; px < x + PATH_WIDTH; px++) {
        for(py = y; py < y + PATH_WIDTH; py++) {
          // make an average of point locations to get the center
          if(diffStack[py * AREA_BLOCK + px] < 8.0) {
            totalPointX += px;
            totalPointY += py;
            countPoint++;
          }
        } // py
      } // px


      if(*roadCount >= pointsTotal) {
        Com_Printf("WARNING: exceeded total possible points.\n");
      } else if (countPoint > 0) {
        roadStack[*roadCount][0] = areaX * AREA_BLOCK + totalPointX / countPoint;
        roadStack[*roadCount][1] = areaY * AREA_BLOCK + totalPointY / countPoint;
        (*roadCount)++;
      }
      if(*roadCount >= pointsTotal) break;
    } // y
    if(*roadCount >= pointsTotal) break;
  } // x

}



static void SV_GetAreaColors(int AREA_BLOCK, 
  int R, int G, int B,
  int areaX, int areaY,
  int maxWidth, int maxHeight, 
  unsigned char *pic, int picWidth,
  float *diffStack)
{
  int pixel;
  double L1, A1, B1, L2, A2, B2;
  int x, y;
  // road color
  //rgb2lab(34, 187, 255, &L1, &A1, &B1);
  // should be 70.90 -20.84 -44.99
  // source https://www.nixsensor.com/free-color-converter/
  //printf("L: %f, A: %f, B: %f\n", L1, A1, B1);
  rgb2lab(R, G, B, &L1, &A1, &B1);
  for(x = 0; x < maxWidth; x++) {
    for(y = 0; y < maxHeight; y++) {
      // this is in rgb format already, look for anything within 5% of blue
      //   we do this by converting to HSL and comparing hue with a factor of 
      //   S (saturation) and L (luminosity)
      pixel = (areaY * AREA_BLOCK * picWidth * 4) // area rows
            + (areaX * AREA_BLOCK * 4) // area columns
            + (y * picWidth * 4 + x * 4); // x and y inside area
      R = pic[pixel + 0];
      G = pic[pixel + 1];
      B = pic[pixel + 2];
      // RGB: 33, 179, 236
      // HSL: 197, 86, 93
      // not accurate enough
      rgb2lab(R, G, B, &L2, &A2, &B2);
      float diff = CIEDE2000(L1, A1, B1, L2, A2, B2);
      /*
      //RGBToHSL(R, G, B, &H, &S, &L);
      //printf("H: %i, S: %i, L: %i\n", H, S, L);
      //printf("x: %i, y: %i, R: %i, G: %i, B: %i\n", a1 * AREA_BLOCK + x, a2 * AREA_BLOCK + y, R, G, B);
      //double diff = deltaE(L1, A1, B1, L2, A2, B2);
      printf("x: %i, y: %i, diff: %f\n", a1 * AREA_BLOCK + x, a2 * AREA_BLOCK + y, diff);
      */
      diffStack[y * AREA_BLOCK + x] = diff;
    }
  }

}


int SV_MakeMonacoF1( void ) {
  vec3_t  vs[2];
  int offset = 0;
  int maxWidth, maxHeight;
  int a1, a2;
  int SCALE = 16;
  int MAX_BRUSHES = 3000;
  int AREA_BLOCK = 64;
  int PATH_WIDTH = 4;
  int POINTS_SEG = AREA_BLOCK / PATH_WIDTH;
  int POINTS_PER = POINTS_SEG * POINTS_SEG;

	memset(output, 0, sizeof(output));
	strcpy(output, "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"\"_color\" \"1 1 1\"\n"
		"\"message\" \"MonacoF1\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_ambient\" \"10\"\n"
		"\"gridsize\" \"512.0 512.0 512.0\"\n"
  );
	offset += strlen(output);


  // try some simple edge detection on the monaco satallite view
  unsigned char *pic;
  int height, width;
  CL_LoadJPG("maps/monaco-track-map-1.jpg", &pic, &width, &height);
  
  
  // loop through image and make "bricks" wherever there are certain colors, 
  // TODO: we'll figure out height later
  // the biggest a road section can get is 64 by 64, so make a buffer 256x256x2
  // since of scanning the image left to right, top to bottom in single rows, 
  //   instead we have to scan in 256x256 size chunks, so the stack can minimize
  //   the number of cubes generated
  // we search a 256x256 space to maximize the likelyhood that a 64x64 block fits
  //   because if we searched a 64x64 for 64x64 size blocks we will probably,
  //   only fit 32x32 or smaller because the roads/buildings will be broken up 
  //   along the edges of each area unless it's a perfect fit
  float  *diffStack = Z_Malloc(AREA_BLOCK * AREA_BLOCK * sizeof(float));
  //vec2_t *roadStack = Hunk_AllocateTempMemory(AREA_BLOCK * AREA_BLOCK * sizeof(vec2_t) * 2);
  // because we are trying to minimize the number of blocks, this algorithm is
  //   very greedy, which means it will take a lot of extra processing
  int areaHor = ceil(width * 1.0 / AREA_BLOCK);
  int areaVer = ceil(height * 1.0 / AREA_BLOCK);
  int pointsTotal = areaHor * areaVer * POINTS_PER;
  Com_Printf("Allocating enough memory to store %i total path points.\n", pointsTotal);
  int *nearestStack = Z_Malloc(POINTS_SEG * sizeof(int));
  vec2_t *closeStack = Z_Malloc(POINTS_SEG * sizeof(vec2_t));
  vec2_t *roadStack = Z_Malloc(pointsTotal * sizeof(vec2_t));
  vec2_t *blockStack = Z_Malloc(pointsTotal * sizeof(vec2_t) * 2);
  int roadCount = 0;
  int blockCount = 0;

  for(a1 = 0; a1 < areaHor; a1++) {
    for(a2 = 0; a2 < areaVer; a2++) {
      // TODO: make as few cubes as possible by decimating/
      //   checking if cube is inside a bigger cube that still fits along a spline
      //   probably going to need a stack for this, in order to form the splines
      //memset(roadStack, 0, pointsTotal * sizeof(vec2_t));
      memset(diffStack, 0, AREA_BLOCK * AREA_BLOCK * sizeof(float));

      // on the last loop, don't go past the edge of the image, only scan remaining area
      maxWidth = AREA_BLOCK;
      if(a1 + 1 == areaHor) {
        maxWidth = width % AREA_BLOCK;
      }
      maxHeight = AREA_BLOCK;
      if(a2 + 1 == areaVer) {
        maxHeight = height % AREA_BLOCK;
      }

      SV_GetAreaColors(
        AREA_BLOCK,
        34, 187, 255, 
        a1, a2,
        maxWidth, maxHeight, 
        pic, width, 
        diffStack);
      SV_GetAreaPaths(
        a1, a2, 
        AREA_BLOCK, PATH_WIDTH, 
        maxWidth, maxHeight, 
        diffStack, 
        roadStack, &roadCount, 
        pointsTotal);


      SV_GetAreaColors(
        AREA_BLOCK,
        75, 75, 83, 
        a1, a2,
        maxWidth, maxHeight, 
        pic, width, 
        diffStack);
      SV_GetAreaBlocks(
        a1, a2, 
        AREA_BLOCK,
        maxWidth, maxHeight,
        diffStack, 
        blockStack, &blockCount, 
        pointsTotal
      );
      
    } // a2
  } // a1
  
  
  // make some buildings and side roads and TODO: barriers and z-height
  int b, bx, by, bw, bh;
  for(b = 0; b < blockCount; b++) {
    bx = blockStack[b*2][0] * SCALE;
    by = blockStack[b*2][1] * SCALE;
    bw = blockStack[b*2+1][0] * SCALE;
    bh = blockStack[b*2+1][1] * SCALE;
    SV_SetStroke("cube1");
    char *road = SV_MakeCube(
      (vec3_t){bx, by, 100}, 
      (vec3_t){bx, bh, 100}, 
      (vec3_t){bw, bh, 100}, 
      (vec3_t){bw, by, 100},

      (vec3_t){bx, by, 0}, 
      (vec3_t){bx, bh, 0}, 
      (vec3_t){bw, bh, 0}, 
      (vec3_t){bw, by, 0}
    );
    strcpy(&output[offset], road);
    offset += strlen(&output[offset]);
    if(brushC >= MAX_BRUSHES) break;
  }
  
  
  // divide up by 64x64 again, draw the lines seperately, for each area
  //   then connect them together because there might be multiple roads
  //   in a single area, and a pit stop, this will allow those lines to
  //   connect to the main track loop, and also allow us to get rid of
  //   any outliers
  
  float sumX, sumY, sumXY, sumX2, slope;
  char *road;
  int x1, x2, x3, x4, y1, y2, y3, y4;
  float radians, length;
  float nearestLength = (float)0x7FFFFFFF;
  qboolean alreadyAdded = qfalse;
  int i, j, k, n;
  for(i = 0; i < roadCount; i++) {
    // find the closest 16 points on a line, find 2 then average and
    //   find 2 more that are neither of the first 2, and repeat
    nearestStack[0] = i;
    closeStack[0][0] = roadStack[i][0];
    closeStack[0][1] = roadStack[i][1];
    for(j = 1; j < POINTS_SEG; j++) {

      // TODO: find the closest point to any other point on the line, making a longer line
      //   then use the long line instead of PATH_WIDTH * SCALE * 2 below
      nearestLength = (float)0x7FFFFFFF;
      for(k = 0; k < roadCount; k++) {
        alreadyAdded = qfalse;
        for(n = 0; n < j; n++) {
          if(nearestStack[n] == k) {
            alreadyAdded = qtrue;
            break;
          }
        }
        if(alreadyAdded) continue;
        length = sqrt(pow(closeStack[j-1][0] - roadStack[k][0], 2)
          + pow(closeStack[j-1][1] - roadStack[k][1], 2));
          // TODO: make this PATH_WIDTH * 4 or something
        if(fabsf(length) < nearestLength && fabsf(length) < AREA_BLOCK * 2) {
          nearestLength = fabsf(length);
          nearestStack[j] = k;
        }
      }
      if(nearestLength == (float)0x7FFFFFFF) {
        Com_Error(ERR_FATAL, "nothing this close!");
      }
      closeStack[j][0] = roadStack[nearestStack[j]][0];
      closeStack[j][1] = roadStack[nearestStack[j]][1];
    }
    

    sumX = 0;
    sumY = 0;
    sumXY = 0;
    sumX2 = 0;
    for(j = 0; j < POINTS_SEG; j++) {
      sumX += closeStack[j][0];
      sumY += closeStack[j][1];
    }
    sumX /= POINTS_SEG;
    sumY /= POINTS_SEG;

    // find the slope for 16 points so we know which way to connect each segment of track
    // since every point looks for the next closest point we should end up with a continuous
    //   set of brushes
    for(j = 0; j < POINTS_SEG; j++) {
      sumXY += (closeStack[j][0] - sumX) * (closeStack[j][1] - sumY);
      sumX2 += pow(closeStack[j][0] - sumX, 2);
    }
    slope = sumXY / sumX2;

    /*
    if(roadStack[i][0] > 7 * AREA_BLOCK && roadStack[i][0] < 8 * AREA_BLOCK
      && roadStack[i][1] > 3 * AREA_BLOCK && roadStack[i][1] > 4 * AREA_BLOCK) {
      for(j = 0; j < POINTS_PER; j++) {
        printf("i: %i, x: %f, y: %f\n", nearestStack[j], closeStack[j][0], closeStack[j][1]);
      }
      printf("slope: %f\n", slope);
      break;
    }
    */

    // draw the next cube in the sequence to simplify so we know both sides of
    //   of 4 corners to draw, average the distance between the first "cube"
    //   points and the next the next cube points, divide up which points
    //   connect based on the slope, 45 degrees, -45 degrees, etc
    // this decides which sides are connected so we don't accidentally flip verts
    //   and end up with a brush that is backwards/invisible
    
    if(fabsf(slope) < 1) {
      /*
        +
         \
          \
           \
      */
      // bottom-left, top-left, top-right, bottom-right
      /*
      if(roadStack[nearestI2][0] < x1) {
        x1 = x2 = roadStack[nearestI2][0];
      } else {
        x3 = x4 = roadStack[nearestI2][0];
      }
      */
      radians = atan(slope); // always work on absolute so our points don't get reversed
      //printf("atan: %f\n", radians);
      x1 = roadStack[i][0]*SCALE;
      y1 = roadStack[i][1]*SCALE;

      x2 = roadStack[i][0]*SCALE;
      y2 = roadStack[i][1]*SCALE + PATH_WIDTH * SCALE * 2;

      x4 = roadStack[i][0]*SCALE + PATH_WIDTH * SCALE * 2 * cos(radians);
      y4 = roadStack[i][1]*SCALE + PATH_WIDTH * SCALE * 2 * sin(radians);

      x3 = x4;
      y3 = y4 + PATH_WIDTH * SCALE * 2;

      if(slope < 0)
        SV_SetStroke("cube3");
      else
        SV_SetStroke("cube0");
      road = SV_MakeCube(
        (vec3_t){x1, y1, 4}, 
        (vec3_t){x2, y2, 4}, 
        (vec3_t){x3, y3, 4}, 
        (vec3_t){x4, y4, 4},

        (vec3_t){x1, y1, 0}, 
        (vec3_t){x2, y2, 0}, 
        (vec3_t){x3, y3, 0}, 
        (vec3_t){x4, y4, 0}
      );
    } else {
      /*
           /
          /
         /
        +
      */
      /*
      if(roadStack[nearestI2][1] < y1) {
        y1 = y4 = roadStack[nearestI2][1];
      } else {
        y2 = y3 = roadStack[nearestI2][1];
      }
      */
      int mult = PATH_WIDTH * SCALE * 2;
      if(slope < 0) mult = -PATH_WIDTH * SCALE * 2;
      radians = atan(slope); // always work on absolute so our points don't get reversed
      x1 = roadStack[i][0]*SCALE;
      y1 = roadStack[i][1]*SCALE;

      x2 = roadStack[i][0]*SCALE + mult * cos(radians);
      y2 = roadStack[i][1]*SCALE + mult * sin(radians);

      x4 = roadStack[i][0]*SCALE + PATH_WIDTH * SCALE * 2;
      y4 = roadStack[i][1]*SCALE;

      x3 = x2 + PATH_WIDTH * SCALE * 2;
      y3 = y2;

      if(slope < 0)
        SV_SetStroke("cube1");
      else
        SV_SetStroke("cube2");
      road = SV_MakeCube(
        (vec3_t){x1, y1, 4}, 
        (vec3_t){x2, y2, 4}, 
        (vec3_t){x3, y3, 4}, 
        (vec3_t){x4, y4, 4},

        (vec3_t){x1, y1, 0}, 
        (vec3_t){x2, y2, 0}, 
        (vec3_t){x3, y3, 0}, 
        (vec3_t){x4, y4, 0}
      );
    }
    
    strcpy(&output[offset], road);
    offset += strlen(&output[offset]);
    if(brushC >= MAX_BRUSHES) break;
  }

  int minX = 0, maxX = 0, minY = 0, maxY = 0;
  for(int i = 0; i < roadCount; i++) {
    if(roadStack[i][0]*SCALE < minX) {
      minX = roadStack[i][0]*SCALE;
    }
    if(roadStack[i][1]*SCALE < minY) {
      minY = roadStack[i][1]*SCALE;
    }
    if(roadStack[i][0]*SCALE > maxX) {
      maxX = roadStack[i][0]*SCALE;
    }
    if(roadStack[i][1]*SCALE > maxY) {
      maxY = roadStack[i][1]*SCALE;
    }
  }

  for(int i = 0; i < blockCount; i++) {
    if(blockStack[i][0]*SCALE < minX) {
      minX = blockStack[i][0]*SCALE;
    }
    if(blockStack[i][1]*SCALE < minY) {
      minY = blockStack[i][1]*SCALE;
    }
    if(blockStack[i][0]*SCALE > maxX) {
      maxX = blockStack[i][0]*SCALE;
    }
    if(blockStack[i][1]*SCALE > maxY) {
      maxY = blockStack[i][1]*SCALE;
    }
  }

	vs[0][0] = minX - 200;
  vs[0][1] = minY - 200;
  vs[0][2] = -2000;
	vs[1][0] = maxX + 200;
  vs[1][1] = maxY + 200;
  vs[1][2] = 2000;
	SV_SetStroke("sky1");
	strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
	offset += strlen(&output[offset]);

	strcpy(&output[offset], "}\n");
	offset += 2;
  
  Z_Free( roadStack );
  Z_Free( blockStack );
  Z_Free( nearestStack );
  Z_Free( closeStack );
  Z_Free( diffStack );
  
  if(brushC >= MAX_BRUSHES) {
    Com_Printf("WARNING: exceeded max brushes.\n");
  }
  
	strcpy(&output[offset], 
		va("{\n"
		"\"classname\" \"misc_skybox\"\n"
		"\"origin\" \"%i %i %i\"\n"
		"}\n", 
		 (int)(vs[1][0] - 64),
		 (int)(vs[1][1] - 64),
		 (int)(vs[1][2] - 64)));
 	offset += strlen(&output[offset]);

  strcpy(&output[offset], 
    va("{\n"
    "\"classname\" \"info_player_start\"\n"
    "\"origin\" \"%i %i %i\"\n"
    "\"angle\" \"180\"\n"
    "}\n", 1050, 550, 200));
  offset += strlen(&output[offset]);
	
  return offset;
}

#endif
