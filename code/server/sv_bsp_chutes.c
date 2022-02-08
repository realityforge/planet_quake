#include "server.h"

#ifdef USE_MEMORY_MAPS

/* Edge of Oblivion clone
It would be cool if distance was a thing for this map, but with the 
chutes and ladders making the platforms faster to traverse to weapons
2 pyramids on either end, a pyramid has top platform, middle entrace,
and bottom floor with a hole to fall out or a sidways jump pad that 
bounces player back up to lowest platform

chutes should be 6 sided, span vertically and horizontally with 
trigger_push, a couple of mover platforms in between

Possible fog for distance, furthest pyramic is just barely visible, maybe 
dark enough to hide in.

Platforms in between can be ranom arrangement, similar code to maze except
walls don't span the whole length, and that length determines the floor
of the platform, walls are short and can be jumped like Edge of Oblivion
Spans 3 floors that are each 2x height of pyramid

Platform easter egg, must go through teleporter when moving platform is underneath
otherwise teleporter goes into void


*/
int SV_MakeChutesAndLadders( void ) {
	vec3_t  vs[2];
  int offset = 0;
	//int rampWidth = 150;
	int cellHeight = 6000;
	int totalWidth = 12000;
	int totalHeight = 12000;
  int pyramidSize = 800;
  int pyramicHeight = 2000;

	// TODO: ramps and wind tunnels like Edge of Oblivion with different shaped pyramids and stuff in space
	vs[0][0] = -(totalWidth / 2);
	vs[1][0] = +(totalWidth / 2);

	vs[0][1] = -(totalHeight / 2);
	vs[1][1] = +(totalHeight / 2);

	vs[0][2] = -cellHeight;
	vs[1][2] = +cellHeight;

	memset(output, 0, sizeof(output));
	strcpy(output, "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"\"_color\" \"1 1 1\"\n"
		"\"message\" \"Chutes And Ladders\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_ambient\" \"10\"\n"
		"\"gridsize\" \"512.0 512.0 512.0\"\n"
  );
	offset += strlen(output);

  SV_SetStroke("sky1");
	strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
	offset += strlen(&output[offset]);
  
  // make a pyramid with 3 levels on both sides of the map
  for(int i = 0; i < 2; i++) {
    // make 4 sides top and bottom
    int center[2];
    if(i == 0) {
      center[0] = -5000;
      center[1] = -5000;
    } else {
      center[0] = 5000;
      center[1] = 5000;
    }
    int offsets[16][2] = {
      {center[0] - pyramidSize - 16, center[1] - pyramidSize},
      {center[0] - pyramidSize - 16, center[1] + pyramidSize},
      {center[0] - pyramidSize,      center[1] + pyramidSize},
      {center[0] - pyramidSize,      center[1] - pyramidSize},

      {center[0] - pyramidSize,      center[1] + pyramidSize},
      {center[0] - pyramidSize,      center[1] + pyramidSize + 16},
      {center[0] + pyramidSize,      center[1] + pyramidSize + 16},
      {center[0] + pyramidSize,      center[1] + pyramidSize},
      
      {center[0] + pyramidSize,      center[1] - pyramidSize},
      {center[0] + pyramidSize,      center[1] + pyramidSize},
      {center[0] + pyramidSize + 16, center[1] + pyramidSize},
      {center[0] + pyramidSize + 16, center[1] - pyramidSize},
      
      {center[0] - pyramidSize,      center[1] - pyramidSize - 16},
      {center[0] - pyramidSize,      center[1] - pyramidSize},
      {center[0] + pyramidSize,      center[1] - pyramidSize},
      {center[0] + pyramidSize,      center[1] - pyramidSize - 16}
    };
    int centers[16][2] = {
      {center[0] - (pyramidSize / 2) - 16, center[1] - (pyramidSize / 2)},
      {center[0] - (pyramidSize / 2) - 16, center[1] + (pyramidSize / 2)},
      {center[0] - (pyramidSize / 2),      center[1] + (pyramidSize / 2)},
      {center[0] - (pyramidSize / 2),      center[1] - (pyramidSize / 2)},

      {center[0] - (pyramidSize / 2),      center[1] + (pyramidSize / 2)},
      {center[0] - (pyramidSize / 2),      center[1] + (pyramidSize / 2) + 16},
      {center[0] + (pyramidSize / 2),      center[1] + (pyramidSize / 2) + 16},
      {center[0] + (pyramidSize / 2),      center[1] + (pyramidSize / 2)},
      
      {center[0] + (pyramidSize / 2),      center[1] - (pyramidSize / 2)},
      {center[0] + (pyramidSize / 2),      center[1] + (pyramidSize / 2)},
      {center[0] + (pyramidSize / 2) + 16, center[1] + (pyramidSize / 2)},
      {center[0] + (pyramidSize / 2) + 16, center[1] - (pyramidSize / 2)},
      
      {center[0] - (pyramidSize / 2),      center[1] - (pyramidSize / 2) - 16},
      {center[0] - (pyramidSize / 2),      center[1] - (pyramidSize / 2)},
      {center[0] + (pyramidSize / 2),      center[1] - (pyramidSize / 2)},
      {center[0] + (pyramidSize / 2),      center[1] - (pyramidSize / 2) - 16}
    };
    for(int s = 0; s < 16; s+=4) {
      SV_SetStroke("cube1");
      char *top = SV_MakeCube(
        (vec3_t){centers[s+0][0], centers[s+0][1], pyramicHeight}, 
        (vec3_t){centers[s+1][0], centers[s+1][1], pyramicHeight}, 
        (vec3_t){centers[s+2][0], centers[s+2][1], pyramicHeight}, 
        (vec3_t){centers[s+3][0], centers[s+3][1], pyramicHeight},

        (vec3_t){offsets[s+0][0], offsets[s+0][1], 100}, 
        (vec3_t){offsets[s+1][0], offsets[s+1][1], 100}, 
        (vec3_t){offsets[s+2][0], offsets[s+2][1], 100}, 
        (vec3_t){offsets[s+3][0], offsets[s+3][1], 100}
      );
      strcpy(&output[offset], top);
      offset += strlen(&output[offset]);

      char *bottom = SV_MakeCube(
        (vec3_t){offsets[s+0][0], offsets[s+0][1], 0}, 
        (vec3_t){offsets[s+1][0], offsets[s+1][1], 0}, 
        (vec3_t){offsets[s+2][0], offsets[s+2][1], 0}, 
        (vec3_t){offsets[s+3][0], offsets[s+3][1], 0},

        (vec3_t){centers[s+0][0], centers[s+0][1], -pyramicHeight}, 
        (vec3_t){centers[s+1][0], centers[s+1][1], -pyramicHeight}, 
        (vec3_t){centers[s+2][0], centers[s+2][1], -pyramicHeight}, 
        (vec3_t){centers[s+3][0], centers[s+3][1], -pyramicHeight}
      );
      strcpy(&output[offset], bottom);
      offset += strlen(&output[offset]);
    }
  }
  
  // TODO: add a small maze inside 2 floors of the pyramid for extra fun 
  //   spawning and collecting weapons
  
  
  // there is basically 8,000 units between both pyramids,
  //   8,000 / 4 = 2,000 / 5 = 400
  // randomize 1 - 25 possible x/y positions, repeat 16 times for each quadrant
  // connect platforms randomly, until every position has at least 1 connection
  //   guaruntee that ramp to platforms on different heights do not exceed
  //   a specific incline
  /* too many platforms, too small
  for(int qx = 0; qx < 4; qx++) { // x * 2000
    for(int qy = 0; qy < 4; qy++) { // y * 2000
      
      for(int i = 0; i < 4; i++) {
        int spotX = rand() % 5;
        int spotY = rand() % 5;
        int spotZ = rand() % 5;

        // TODO: make platform
        char *platform = SV_MakeCube(
          (vec3_t){qx * 2000 + spotX * 400 - 100,      qy * 2000 + spotY * 400 - 100,      -200 + spotZ * 100 + 64}, 
          (vec3_t){qx * 2000 + spotX * 400 - 100,      qy * 2000 + spotY * 400 + 100,      -200 + spotZ * 100 + 64}, 
          (vec3_t){qx * 2000 + spotX * 400 + 100,      qy * 2000 + spotY * 400 + 100,      -200 + spotZ * 100 + 64}, 
          (vec3_t){qx * 2000 + spotX * 400 + 100,      qy * 2000 + spotY * 400 - 100,      -200 + spotZ * 100 + 64},

          (vec3_t){qx * 2000 + spotX * 400 - 100 + 32, qy * 2000 + spotY * 400 - 100 + 32, -200 + spotZ * 100}, 
          (vec3_t){qx * 2000 + spotX * 400 - 100 + 32, qy * 2000 + spotY * 400 + 100 - 32, -200 + spotZ * 100}, 
          (vec3_t){qx * 2000 + spotX * 400 + 100 - 32, qy * 2000 + spotY * 400 + 100 - 32, -200 + spotZ * 100}, 
          (vec3_t){qx * 2000 + spotX * 400 + 100 - 32, qy * 2000 + spotY * 400 - 100 + 32, -200 + spotZ * 100}
        );
        strcpy(&output[offset], platform);
        offset += strlen(&output[offset]);
      }

    }
  }
  */
  
  int totalPlatforms = 30;
  vec4_t *platStack = Z_Malloc(totalPlatforms * sizeof(vec4_t));
  int numPlatforms = 2;
  platStack[0][0] = -5000;
  platStack[0][1] = -5000;
  platStack[0][2] = 0;
  platStack[0][3] = pyramidSize;
  platStack[1][0] = 5000;
  platStack[1][1] = 5000;
  platStack[1][2] = 0;
  platStack[1][3] = pyramidSize;
  vec3_t *rampStack = Z_Malloc(totalPlatforms * 12 * sizeof(vec3_t));
  //int numRamps = 0;
  qboolean *platSides = Z_Malloc(totalPlatforms * 4 * sizeof(qboolean));
  memset(platSides, qfalse, totalPlatforms * sizeof(qboolean[4]));

  for(int i = numPlatforms; i < totalPlatforms; i++) {
    int safety = 10;
    int spotX;
    int spotY;
    int spotZ;
    int size = (rand() % 8 + 3) * 50;
    qboolean found = qfalse;
    do {
      spotX = -6000 + (rand() % 7 + 1) * 1500;
      spotY = -6000 + (rand() % 7 + 1) * 1500;
      spotZ = -500  + (rand() % 10) * 200;
      found = qfalse;
      // prevent duplicate platforms
      for(int j = 0; j < numPlatforms; j++) {
        if((j < 2 || platStack[j][2] == spotZ) &&
          isOverlapping((vec2_t){spotX - size, spotY - size},
                        (vec2_t){spotX + size, spotY + size},
                        (vec2_t){platStack[j][0] - platStack[j][3], platStack[j][1] - platStack[j][3]},
                        (vec2_t){platStack[j][0] + platStack[j][3], platStack[j][1] + platStack[j][3]})) {
          found = qtrue;
          break;
        }
      }
    } while (--safety > 0);
    if(found) {
      Com_Printf("WARNING: found platform, can't place platform, too crowded.\n");
      continue; 
    }
    platStack[i][0] = spotX;
    platStack[i][1] = spotY;
    platStack[i][2] = spotZ;
    platStack[i][3] = size;

    platSides[i * 4 + 0] = qtrue;
    platSides[i * 4 + 1] = qtrue;
    platSides[i * 4 + 2] = qtrue;
    platSides[i * 4 + 3] = qtrue;

    // TODO: SV_MakePlatform
    SV_SetStroke("cube2");
    char *platform = SV_MakeCube(
      (vec3_t){spotX - size,      spotY - size,      spotZ + 64}, 
      (vec3_t){spotX - size,      spotY + size,      spotZ + 64}, 
      (vec3_t){spotX + size,      spotY + size,      spotZ + 64}, 
      (vec3_t){spotX + size,      spotY - size,      spotZ + 64},
      // make the underneath a little angled for style
      (vec3_t){spotX - size + 32, spotY - size + 32, spotZ}, 
      (vec3_t){spotX - size + 32, spotY + size - 32, spotZ}, 
      (vec3_t){spotX + size - 32, spotY + size - 32, spotZ}, 
      (vec3_t){spotX + size - 32, spotY - size + 32, spotZ}
    );
    strcpy(&output[offset], platform);
    offset += strlen(&output[offset]);
    
    numPlatforms++;
  }

  // top, right, bottom, left like CSS
	/*
  int sideOffsets[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
  };
	*/

  for(int i = 0; i < numPlatforms; i++) {
    // connect every platform to one other platform, 
    //   provided the ramp does not exceed 30degree incline and the side
    //   of the platform is facing and not connecting to another platform
    // find closest instead?
    // measure length between the centers of all 4 sides to find the closest
    
    
    
    /*
    // Doesn't work to choose random, need to be more exhaustive,
    //   then optimize by least number of removals of cross-overs

    qboolean found = qfalse;
    int safety = 10;
    do {
      int plat = rand() % numPlatforms;
      if(plat == i) { continue; } // early exit

      float nearestLength = (float)0x7FFFFFFF;
      int nearestSide1 = -1; // 0-3
      int nearestSide2 = -1; // 0-3
      float length;
      for(int s1 = 0; s1 < 4; s1++) {
        for(int s2 = 0; s2 < 4; s2++) {
          length = sqrt(pow((platStack[plat][0] + sideOffsets[s2][0] * platStack[plat][3]) - (platStack[i][0] + sideOffsets[s1][0] * platStack[i][3]), 2)
                      + pow((platStack[plat][1] + sideOffsets[s2][1] * platStack[plat][3]) - (platStack[i][1] + sideOffsets[s1][1] * platStack[i][3]), 2));
          if(length < nearestLength) {
            nearestSide1 = s1;
            nearestSide2 = s2;
            nearestLength = length;
          }
        }
      }
      float rise = platStack[plat][2] - platStack[i][2];
      if(nearestSide1 == -1 || nearestSide2 == -1
        || platSides[plat * 4 + nearestSide2] == qfalse // side already used, not going to work
        || platSides[i * 4 + nearestSide1] == qfalse) {
        Com_Printf("WARNING: skipping, ramp location already used.\n");
        continue;
      }
      if((rise != 0 && fabsf(rise) / fabsf(length) > 1.75) || length == 0) {
        Com_Printf("WARNING: skipping, ramp too steep rise: %f over run: %f.\n", fabsf(rise), fabsf(length));
        continue;
      }
      */

      // make 0/1/2 corners to connect the 2 platforms
      // calculate which direction the ramps and corner need to be
      //   from selected sides
      /*
            +---+
            |   |---+
            +---+   |
                    |
                    |
                    |   +---+
                    +---|   |
                        +---+
      */
      /*
      int numSegments = 0;
      if(nearestSide1 % 2 == nearestSide2 % 2) {
        // are the platforms in line with each other in some direction?
        if(platStack[plat][0] == platStack[i][0]
          || platStack[plat][1] == platStack[i][1]) {
          numSegments = 1;
        } else {
          numSegments = 3;
        }
      } else {
        numSegments = 2;
      }
      
      if(nearestSide1 % 2 == 0) {
        float horizontalLength = fabsf(platStack[plat][1] - platStack[i][1]);
        float verticalLength = fabsf(platStack[plat][0] - platStack[i][0]);
        float totalLength = rise / (horizontalLength + verticalLength);
        // x's
        rampStack[numRamps * 4 + 0][0] =
        rampStack[numRamps * 4 + 1][0] = -rampWidth + platStack[i][0]; // always 0: + sideOffsets[s1][4] * platSides[s1][0];
        rampStack[numRamps * 4 + 2][0] =
        rampStack[numRamps * 4 + 3][0] = rampWidth + platStack[i][0]; // always 0:  + sideOffsets[s1][4] * platSides[s1][0];
        // y's
        rampStack[numRamps * 4 + 0][1] = 
        rampStack[numRamps * 4 + 3][1] = platStack[i][1] + platStack[i][3] * sideOffsets[nearestSide1][1];
        rampStack[numRamps * 4 + 1][1] =
        rampStack[numRamps * 4 + 2][1] = platStack[plat][1] + platStack[plat][3] * sideOffsets[nearestSide2][1];
        // z's
        // add both lengths together to calculate slope, then take percent of
        //   of length over entire delta-Z for height of corner
        rampStack[numRamps * 4 + 0][2] = 
        rampStack[numRamps * 4 + 3][2] = platStack[i][2];
        rampStack[numRamps * 4 + 1][2] = 
        rampStack[numRamps * 4 + 2][2] = platStack[i][2] + horizontalLength * totalLength;
        
        if(numSegments == 2) {
          rampStack[(numRamps + 1) * 4 + 0][0] =
          rampStack[(numRamps + 1) * 4 + 1][0] = platStack[i][0] + platStack[i][3] * sideOffsets[nearestSide1][0]; // always 0: + sideOffsets[s1][4] * platSides[s1][0];
          rampStack[(numRamps + 1) * 4 + 2][0] =
          rampStack[(numRamps + 1) * 4 + 3][0] = platStack[plat][0] + platStack[plat][3] * sideOffsets[nearestSide2][0]; // always 0:  + sideOffsets[s1][4] * platSides[s1][0];
          // y's
          rampStack[(numRamps + 1) * 4 + 0][1] = 
          rampStack[(numRamps + 1) * 4 + 3][1] = -rampWidth + platStack[plat][1];
          rampStack[(numRamps + 1) * 4 + 1][1] =
          rampStack[(numRamps + 1) * 4 + 2][1] = rampWidth + platStack[plat][1];
          // z's
          // add both lengths together to calculate slope, then take percent of
          //   of length over entire delta-Z for height of corner
          rampStack[(numRamps + 1) * 4 + 0][2] = 
          rampStack[(numRamps + 1) * 4 + 1][2] = platStack[i][2] + horizontalLength * totalLength;
          rampStack[(numRamps + 1) * 4 + 2][2] = 
          rampStack[(numRamps + 1) * 4 + 3][2] = platStack[plat][2];
        } else {
          
          // TODO: remove this when all ramps are done
          numSegments = 1;
        }
      } else {
  
        // TODO: remove this when all ramps are done
        numSegments = 0;
      }

      // make sure we don't accidentally run in to any other platforms
      qboolean intereferes = qfalse;
      for(int s = numRamps; s < numRamps + numSegments; s++) {
        // correct the order of vertexes, always goes clockwise starting 
        //   with bottom-left, closest to zero/negative infinity
        if(rampStack[s * 4 + 0][0] > rampStack[s * 4 + 2][0]) {
          SV_FlipHorizontal(&rampStack[numRamps * 4]);
        }
        if(rampStack[s * 4 + 0][1] > rampStack[s * 4 + 2][1]) {
          SV_FlipVertical(&rampStack[numRamps * 4]);
        }
        for(int j = 0; j < numPlatforms; j++) {
          if(j == i || j == plat) continue;
          if(isOverlapping((vec2_t){rampStack[s * 4 + 0][0], rampStack[s * 4 + 0][1]},
                           (vec2_t){rampStack[s * 4 + 2][0], rampStack[s * 4 + 2][1]},
                           (vec2_t){platStack[j][0] - platStack[j][3], platStack[j][1] - platStack[j][3]},
                           (vec2_t){platStack[j][0] + platStack[j][3], platStack[j][1] + platStack[j][3]})) {
            intereferes = qtrue;
            break;
          }
        }
        if(intereferes) {
          break;
        }
        // check if it interferes with other ramps
        for(int j = 0; j < numRamps; j++) {
          if(isOverlapping((vec2_t){rampStack[s * 4 + 0][0], rampStack[s * 4 + 0][1]},
                           (vec2_t){rampStack[s * 4 + 2][0], rampStack[s * 4 + 2][1]},
                           (vec2_t){rampStack[j * 4 + 0][0], rampStack[j * 4 + 0][1]},
                           (vec2_t){rampStack[j * 4 + 2][0], rampStack[j * 4 + 2][1]})) {
            intereferes = qtrue;
            break;
          }
        }
        if(intereferes) {
          break;
        }
      }
      if(intereferes) { 
        Com_Printf("WARNING: skipping ramp connection because it intereferes.\n");
        continue;
      }

      for(int s = numRamps; s < numRamps + numSegments; s++) {
        SV_SetStroke("cube3");
        char *platform = SV_MakeCube(
          (vec3_t){rampStack[s * 4 + 0][0], rampStack[s * 4 + 0][1], rampStack[s * 4 + 0][2] + 64},
          (vec3_t){rampStack[s * 4 + 1][0], rampStack[s * 4 + 1][1], rampStack[s * 4 + 1][2] + 64},
          (vec3_t){rampStack[s * 4 + 2][0], rampStack[s * 4 + 2][1], rampStack[s * 4 + 2][2] + 64},
          (vec3_t){rampStack[s * 4 + 3][0], rampStack[s * 4 + 3][1], rampStack[s * 4 + 3][2] + 64},

          (vec3_t){rampStack[s * 4 + 0][0], rampStack[s * 4 + 0][1], rampStack[s * 4 + 0][2] + 48},
          (vec3_t){rampStack[s * 4 + 1][0], rampStack[s * 4 + 1][1], rampStack[s * 4 + 1][2] + 48},
          (vec3_t){rampStack[s * 4 + 2][0], rampStack[s * 4 + 2][1], rampStack[s * 4 + 2][2] + 48},
          (vec3_t){rampStack[s * 4 + 3][0], rampStack[s * 4 + 3][1], rampStack[s * 4 + 3][2] + 48}
        );
        strcpy(&output[offset], platform);
        offset += strlen(&output[offset]);
      }

      platSides[plat * 4 + nearestSide2] = qfalse;
      platSides[i * 4 + nearestSide1] = qfalse;
      found = qtrue;
      numRamps += numSegments;

    } while (--safety > 0 && !found);
    if(!found) { continue; }
    */
  }
  

  strcpy(&output[offset], "}\n");
  offset += 2;

  Z_Free(platStack);
  Z_Free(platSides);
  Z_Free(rampStack);

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
