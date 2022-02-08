#include "server.h"

#ifdef USE_MEMORY_MAPS

int SV_MakeAtlantis( void ) {
  vec3_t  vs[2];
  int offset = 0;
  int cellHeight = 6000;
	int totalWidth = 12000;
	int totalHeight = 12000;

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
		"\"message\" \"Atlantis Arena\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_ambient\" \"10\"\n"
		"\"gridsize\" \"512.0 512.0 512.0\"\n"
  );
	offset += strlen(output);

  SV_SetStroke("sky1");
	strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
	offset += strlen(&output[offset]);
  
  // make water sphere
  /*
  SV_SetStroke("liquids/clear_calm1");
  char *top = SV_MakeCube(
    (vec3_t){-500, -500,  500}, 
    (vec3_t){-500,  500,  500}, 
    (vec3_t){ 500,  500,  500}, 
    (vec3_t){ 500, -500,  500},

    (vec3_t){-500, -500, -500}, 
    (vec3_t){-500,  500, -500}, 
    (vec3_t){ 500,  500, -500}, 
    (vec3_t){ 500, -500, -500}
  );
  strcpy(&output[offset], top);
  offset += strlen(&output[offset]);
  */
  
  int radius = 1000;
  int thickness = 2000;
  int splits = 16.0;
  int roundOffset = floor(splits / 8.0);
  float splitsPerSide = splits / 4.0; // sides
  float angle = 360.0 / splits;
  for(int i = -roundOffset; i < splitsPerSide + roundOffset; i++) {
    for(int zi = -roundOffset; zi < ceil(splits - roundOffset); zi++) {
      // source: https://math.stackexchange.com/questions/989900/calculate-x-y-z-from-two-specific-degrees-on-a-sphere
      float lat = M_PI * 2.0 * (angle * i) / 360.0;
      float lon = M_PI * 2.0 * (angle * zi) / 360.0;
      float lat2 = M_PI * 2.0 * (angle * (i + 1)) / 360.0;
      float lon2 = M_PI * 2.0 * (angle * (zi + 1)) / 360.0;
      /*
      close but not quite right
      float x1 = radius * sin();
      float y1 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * i) / 360.0)) + radius;
      float x4 = radius * sin(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0);
      float y4 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0)) + radius;
      
      float x2 = (thickness + radius) * sin(M_PI * 2.0 * (angle * i) / 360.0);
      float y2 = (-thickness - radius) * (1.0 - cos(M_PI * 2.0 * (angle * i) / 360.0)) + thickness + radius;
      float x3 = (thickness + radius) * sin(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0);
      float y3 = (-thickness - radius) * (1.0 - cos(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0)) + thickness + radius;

      //float z1 = radius * sin(M_PI * 2.0 * (angle * zi) / 360.0);
      //float z4 = radius * sin(M_PI * 2.0 * (angle * (zi + 1.0)) / 360.0);
      //float z2 = (thickness + radius) * sin(M_PI * 2.0 * (angle * zi) / 360.0);
      //float z3 = (thickness + radius) * sin(M_PI * 2.0 * (angle * (zi + 1.0)) / 360.0);
      float z1 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * zi) / 360.0)) + radius;
      float z4 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * (zi + 1.0)) / 360.0)) + radius;
      float z2 = (-thickness - radius) * (1.0 - cos(M_PI * 2.0 * (angle * zi) / 360.0)) + thickness + radius;
      float z3 = (-thickness - radius) * (1.0 - cos(M_PI * 2.0 * (angle * (zi + 1.0)) / 360.0)) + thickness + radius;
      */
      
      // swap some corners so the cube is always built the right way
      /*
      if(i >= (splitsPerSide * 3.0 - roundOffset)) {
        x3 = x2;
        y3 = y2;
        x2 = x1;
        y2 = y1;
        x1 = 
        y1 = 
        x4 = 
        y4 = 
      } else if(i >= (splitsPerSide * 2.0 - roundOffset)) {
        x3 = x1;
        y3 = y1;
        x4 = x2;
        y4 = y2;
        x1 = 
        y1 = 
        x2 = 
        y2 = 
      } else if(i >= (splitsPerSide - roundOffset)) {
        x4 = x1;
        y4 = y1;
        x1 = x2;
        y1 = y2;
        x3 = 
        y3 = 
        x2 = 
        y2 = 
      }
      */
      /*
      // test cubes
      SV_SetStroke("cube1");
      char *bottomRight = SV_MakeCube(
        (vec3_t){v1[0],    v1[1],    v1[2]+32},
        (vec3_t){v1[0],    v1[1]+32, v1[2]+32},
        (vec3_t){v1[0]+32, v1[1]+32, v1[2]+32},
        (vec3_t){v1[0]+32, v1[1],    v1[2]+32},

        (vec3_t){v1[0],    v1[1],    v1[2]},
        (vec3_t){v1[0],    v1[1]+32, v1[2]},
        (vec3_t){v1[0]+32, v1[1]+32, v1[2]},
        (vec3_t){v1[0]+32, v1[1],    v1[2]}
      );
      strcpy(&output[offset], bottomRight);
      offset += strlen(&output[offset]);
      */
      
      SV_SetStroke("liquids/clear_calm1");
      char *bottomRight = SV_MakeCube(
        (vec3_t){
          radius * cos(lat) * sin(lon2)+1,
          -radius * cos(lat) * cos(lon2)+1,
          radius * sin(lat)+1
        },
        (vec3_t){
          (radius + thickness) * cos(lat) * sin(lon2)+1,
          -(radius + thickness) * cos(lat) * cos(lon2)+1,
          (radius + thickness) * sin(lat)+1
        },
        (vec3_t){
          (radius + thickness) * cos(lat2) * sin(lon2)+1,
          -(radius + thickness) * cos(lat2) * cos(lon2)+1,
          (radius + thickness) * sin(lat2)+1
        },
        (vec3_t){
          radius * cos(lat2) * sin(lon2)+1,
          -radius * cos(lat2) * cos(lon2)+1,
          radius * sin(lat2)+1
        },

        (vec3_t){
          radius * cos(lat) * sin(lon),
          -radius * cos(lat) * cos(lon) ,
          radius * sin(lat)
        },
        (vec3_t){
          (radius + thickness) * cos(lat) * sin(lon),
          -(radius + thickness) * cos(lat) * cos(lon),
          (radius + thickness) * sin(lat)
        },
        (vec3_t){
          (radius + thickness) * cos(lat2) * sin(lon),
          -(radius + thickness) * cos(lat2) * cos(lon),
          (radius + thickness) * sin(lat2)
        },
        (vec3_t){
          radius * cos(lat2) * sin(lon),
          -radius * cos(lat2) * cos(lon),
          radius * sin(lat2)
        }
      );
      strcpy(&output[offset], bottomRight);
      offset += strlen(&output[offset]);
      
#if 0
      SV_SetStroke("liquids/clear_calm1");
      SV_SetStroke("cube1");
      char *bottomRight = SV_MakeCube(
        (vec3_t){x1,    y1,    100}, 
        (vec3_t){x1,    y1+32, 100}, 
        (vec3_t){x1+32, y1+32, 100}, 
        (vec3_t){x1+32, y1,    100},

        (vec3_t){x1,    y1,    0}, 
        (vec3_t){x1,    y1+32, 0}, 
        (vec3_t){x1+32, y1+32, 0}, 
        (vec3_t){x1+32, y1,    0}
      );
      strcpy(&output[offset], bottomRight);
      offset += strlen(&output[offset]);
      SV_SetStroke("cube2");
      char *topRight = SV_MakeCube(
        (vec3_t){x2,    thickness + y2,    100}, 
        (vec3_t){x2,    thickness + y2+32, 100}, 
        (vec3_t){x2+32, thickness + y2+32, 100}, 
        (vec3_t){x2+32, thickness + y2,    100},

        (vec3_t){x2,    thickness + y2,    0}, 
        (vec3_t){x2,    thickness + y2+32, 0}, 
        (vec3_t){x2+32, thickness + y2+32, 0}, 
        (vec3_t){x2+32, thickness + y2,    0}
      );
      strcpy(&output[offset], topRight);
      offset += strlen(&output[offset]);
      SV_SetStroke("cube3");
      char *topLeft = SV_MakeCube(
        (vec3_t){x3-32, thickness + y3-32, 100}, 
        (vec3_t){x3-32, thickness + y3,    100}, 
        (vec3_t){x3,    thickness + y3,    100}, 
        (vec3_t){x3,    thickness + y3-32, 100},

        (vec3_t){x3-32, thickness + y3-32, 0}, 
        (vec3_t){x3-32, thickness + y3,    0}, 
        (vec3_t){x3,    thickness + y3,    0}, 
        (vec3_t){x3,    thickness + y3-32, 0}
      );
      strcpy(&output[offset], topLeft);
      offset += strlen(&output[offset]);
      SV_SetStroke("cube4");
      char *bottomLeft = SV_MakeCube(
        (vec3_t){x4-32, y4-32, 100}, 
        (vec3_t){x4-32, y4,    100}, 
        (vec3_t){x4,    y4,    100}, 
        (vec3_t){x4,    y4-32, 100},

        (vec3_t){x4-32, y4-32, 0}, 
        (vec3_t){x4-32, y4,    0}, 
        (vec3_t){x4,    y4,    0}, 
        (vec3_t){x4,    y4-32, 0}
      );
      strcpy(&output[offset], bottomLeft);
      offset += strlen(&output[offset]);
#endif
    }
  }

  SV_SetStroke("cube3");
  char *centerPlat = SV_MakeCube(
    (vec3_t){-100, -100, 16},
    (vec3_t){-100,  100, 16},
    (vec3_t){ 100,  100, 16},
    (vec3_t){ 100, -100, 16},

    (vec3_t){-100, -100, 0},
    (vec3_t){-100,  100, 0},
    (vec3_t){ 100,  100, 0},
    (vec3_t){ 100, -100, 0}
  );
  strcpy(&output[offset], centerPlat);
  offset += strlen(&output[offset]);

  strcpy(&output[offset], "}\n");
  offset += 2;

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
    "}\n", 0, 0, 100));
  offset += strlen(&output[offset]);

	return offset;
}

#endif
