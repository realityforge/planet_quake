#include "server.h"

#ifdef USE_MEMORY_MAPS

char *SV_MakePortal( float radius, vec3_t min, vec3_t max, int minSegment, int maxSegment, int sides );

int SV_MakeHypercube( void ) {
	float radius = 200.0;
	int offset = 0;
	int width = 600;
	int height = 600;
	int spacing = 300;
	int rows = 2;
	int cols = 2;
	int totalWidth = width * cols + spacing * (cols - 1);
	int totalHeight = height * rows + spacing * (rows - 1);
	vec3_t  vs[2];
	int padding = (width - radius * 2) / 2 - 32;

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	memset(output, 0, sizeof(output));
	strcpy(output, "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"\"_color\" \"1 1 1\"\n"
		"\"message\" \"Windows XP\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_ambient\" \"10\"\n"
		"\"gridsize\" \"512.0 512.0 512.0\"\n"
  );
	offset += strlen(output);

	SV_SetStroke("sky1");
	strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
	offset += strlen(&output[offset]);

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
		vs[0][0] = -(totalWidth / 2) + (x * (width + spacing));
		vs[1][0] = -(totalWidth / 2) + (x * (width + spacing)) + width;

		vs[0][1] = -(totalHeight / 2) + (y * (height + spacing));
		vs[1][1] = -(totalHeight / 2) + (y * (height + spacing)) + height;

		vs[0][2] = -(width / 2);
		vs[1][2] = (height / 2);

		SV_SetStroke(va("cube%i", i));
		strcpy(&output[offset], SV_MakePortal(radius, vs[0], vs[1], -1, -1, SIDE_ALL));
		offset += strlen(&output[offset]);
	}

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
		vs[0][0] = -(totalWidth / 2) + (x * (width + spacing)) - 16;
		vs[1][0] = -(totalWidth / 2) + (x * (width + spacing)) + width + 16;

		vs[0][1] = -(totalHeight / 2) + (y * (height + spacing)) - 16;
		vs[1][1] = -(totalHeight / 2) + (y * (height + spacing)) + height + 16;

		vs[0][2] = -(width / 2) - 16;
		vs[1][2] = (height / 2) + 16;

		SV_SetStroke(va("cube%i", i));
		strcpy(&output[offset], SV_MakePortal(radius - 100, vs[0], vs[1], 2, 3, SIDE_ALL));
		offset += strlen(&output[offset]);
		strcpy(&output[offset], SV_MakePortal(radius - 100, vs[0], vs[1], 8, 9, SIDE_ALL));
		offset += strlen(&output[offset]);
		strcpy(&output[offset], SV_MakePortal(radius - 100, vs[0], vs[1], 14, 15, SIDE_ALL));
		offset += strlen(&output[offset]);
		strcpy(&output[offset], SV_MakePortal(radius - 100, vs[0], vs[1], 20, 21, SIDE_ALL));
		offset += strlen(&output[offset]);
	}

	strcpy(&output[offset], "}\n");
	offset += 2;

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	strcpy(&output[offset], 
		va("{\n"
		"\"classname\" \"misc_skybox\"\n"
		"\"origin\" \"%i %i %i\"\n"
		"}\n", 
		 (int)(vs[1][0] - 64),
		 (int)(vs[1][1] - 64),
		 (int)(vs[1][2] - 64)));
 	offset += strlen(&output[offset]);

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
		vs[0][0] = -(totalWidth / 2) + (x * (width + spacing));
		vs[1][0] = -(totalWidth / 2) + (x * (width + spacing)) + width;

		vs[0][1] = -(totalHeight / 2) + (y * (height + spacing));
		vs[1][1] = -(totalHeight / 2) + (y * (height + spacing)) + height;

		vs[0][2] = -(width / 2);
		vs[1][2] = (height / 2);

		int wallMap[12][3] = {
			{vs[0][0] + padding, vs[0][1] + padding, vs[0][2]-48},
			{vs[1][0] - padding, vs[1][1] - padding, vs[0][2]-32},
			
			{vs[0][0]-48,    vs[0][1] + padding, vs[0][2] + padding},
			{vs[0][0]-32,    vs[1][1] - padding, vs[1][2] - padding},
			
			{vs[0][0] + padding, vs[0][1]-48,    vs[0][2] + padding},
			{vs[1][0] - padding, vs[0][1]-32,    vs[1][2] - padding},
			
			
			{vs[0][0] + padding, vs[0][1] + padding, vs[1][2]+32},
			{vs[1][0] - padding, vs[1][1] - padding, vs[1][2]+48},
			
			{vs[1][0]+32,    vs[0][1] + padding, vs[0][2] + padding},
			{vs[1][0]+48,    vs[1][1] - padding, vs[1][2] - padding},
			
			{vs[0][0] + padding, vs[1][1]+32,    vs[0][2] + padding},
			{vs[1][0] - padding, vs[1][1]+48,    vs[1][2] - padding}
		};
		
		for(int j = 0; j < 6; j++) {
			int *p1 = wallMap[j*2];
			int *p2 = wallMap[j*2+1];
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"trigger_teleport\"\n"
				"\"target\" \"teleport_%i_%i\"\n",
				 i, j));
			offset += strlen(&output[offset]);

			SV_SetStroke("portal1");
			//strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
			strcpy(&output[offset], SV_MakeWall(p1, p2));
			offset += strlen(&output[offset]);
			strcpy(&output[offset], "}\n");
			offset += 2;
		}
		
		// make jump accelerators
		int jumpMap[8][3] = {			
			{vs[0][0] + (width - 64) / 2,      vs[0][1] + (height - radius) / 2,      vs[0][2]},
			{vs[0][0] + (width - 64) / 2 + 64, vs[0][1] + (height - radius) / 2 + 64, vs[0][2] + 16},
			
			{vs[0][0] + (width - radius) / 2,      vs[0][1] + (height - 64) / 2,      vs[0][2]},
			{vs[0][0] + (width - radius) / 2 + 64, vs[0][1] + (height - 64) / 2 + 64, vs[0][2] + 16},
			
			// TODO: convert 100 and 164 to use radius or something?
			{vs[1][0] - (width - 64) / 2,      vs[1][1] - (height - radius) / 2,      vs[0][2]},
			{vs[1][0] - (width - 64) / 2 - 64, vs[1][1] - (height - radius) / 2 - 64, vs[0][2] + 16},
			
			{vs[1][0] - (width - radius) / 2,      vs[1][1] - (height - 64) / 2,      vs[0][2]},
			{vs[1][0] - (width - radius) / 2 - 64, vs[1][1] - (height - 64) / 2 - 64, vs[0][2] + 16},
		};
		
		for(int j = 0; j < 4; j++) {
			int *p1 = jumpMap[j*2];
			int *p2 = jumpMap[j*2+1];
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"trigger_push\"\n"
				"\"target\" \"push_%i_%i\"\n",
				 i, j));
			offset += strlen(&output[offset]);

			SV_SetStroke("portal1");
			strcpy(&output[offset], SV_MakeWall(p1, p2));
			offset += strlen(&output[offset]);
			strcpy(&output[offset], "}\n");
			offset += 2;
		}
	}
	

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
		vs[0][0] = -(totalWidth / 2) + (x * (width + spacing));
		vs[1][0] = -(totalWidth / 2) + (x * (width + spacing)) + width;

		vs[0][1] = -(totalHeight / 2) + (y * (height + spacing));
		vs[1][1] = -(totalHeight / 2) + (y * (height + spacing)) + height;

		vs[0][2] = -(width / 2);
		vs[1][2] = (height / 2);
		
		int destinationOffsets[6][3] = {
			{vs[1][0] - (width / 2), vs[1][1] - (height / 2), vs[0][2]+128},
			{vs[0][0]+32,            vs[1][1] - (width / 2),  vs[0][2] + 32},
			{vs[1][0] - (width / 2), vs[0][1]+32,             vs[0][2] + 32},

			{vs[0][0] + (width / 2), vs[0][1] + (height / 2), vs[1][2]-128},
			{vs[1][0]-32,            vs[0][1] + (width / 2),  vs[0][2] + 32},
			{vs[0][0] + (width / 2), vs[1][1]-32,             vs[0][2] + 32}
		};
		
		int v = 0;
		for(int j = 0; j < 6; j++) {
			// add destinations
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"misc_teleporter_dest\"\n"
				"\"targetname\" \"teleport_%i_%i\"\n"
				"\"origin\" \"%i %i %i\"\n"
				"\"angle\" \"%i\"\n"
				"}\n",
				(i+1)%(rows*cols), (j+3)%6, // adding 3 reverses top and bottom
				destinationOffsets[j][0], 
				destinationOffsets[j][1], 
				destinationOffsets[j][2],
			 j % 3 != 0 ? (v * 90) : 0));
			if(j % 3 != 0) {
				v++;
			}
			offset += strlen(&output[offset]);
		}
		
		strcpy(&output[offset], 
			va("{\n"
			"\"classname\" \"info_notnull\"\n"
			"\"targetname\" \"teleview_%i\"\n"
			"\"origin\" \"%i %i %i\"\n"
			"}\n",
			i,
			(int)(vs[0][0] + (width / 2)), 
			(int)(vs[0][1] + (height / 2)), 
			(int)(vs[0][2] + 32)));
		offset += strlen(&output[offset]);
		
		// make jump accelerators
		int jumpDestinationMap[4][3] = {			
			{vs[0][0] + width / 2, vs[0][1],              vs[0][2] + height / 2 - 64},
			{vs[0][0],             vs[0][1] + height / 2, vs[0][2] + height / 2 - 64},
			{vs[1][0] - width / 2, vs[1][1],              vs[0][2] + height / 2 - 64},
			{vs[1][0],             vs[1][1] - height / 2, vs[0][2] + height / 2 - 64}
		};
		
		for(int j = 0; j < 4; j++) {
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"info_notnull\"\n"
				"\"targetname\" \"push_%i_%i\"\n"
				"\"origin\" \"%i %i %i\"\n"
				"}\n",
				 i, (j + 2) % 4, // adding 3 reverses top and bottom
				 jumpDestinationMap[j][0], 
				 jumpDestinationMap[j][1], 
				 jumpDestinationMap[j][2]));
			offset += strlen(&output[offset]);
		}
	}

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
	
		strcpy(&output[offset], 
			va("{\n"
			"\"classname\" \"info_player_start\"\n"
			"\"origin\" \"%i %i %i\"\n"
			"\"angle\" \"180\"\n"
			"}\n", -(totalWidth / 2) + (x * (width + spacing)) + width - 32,
			 -(totalHeight / 2) + (y * (height + spacing)) + height - 32,
			 -(height / 2) + 32));

		offset += strlen(&output[offset]);
	}

	int lightCorners[4][2] = {
		{128, 128},
		{128, height - 128},
		{width - 128, 128},
		{width - 128, height - 128}
	};
	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
	
		for(int j = 0; j < 4; j++) {
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"light\"\n"
				"\"origin\" \"%i %i %i\"\n"
				"\"light\" \"400\"\n"
        "\"radius\" \"128\"\n"
        "\"scale\" \"5\"\n"
				"\"target\" \"light_%i_L\"\n"
				"}\n", -(totalWidth / 2) + (x * (width + spacing)) + lightCorners[j][0],
				 -(totalHeight / 2) + (y * (height + spacing)) + lightCorners[j][1],
				 (height / 2) - 128,
			 	 i));
			offset += strlen(&output[offset]);
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"light\"\n"
				"\"origin\" \"%i %i %i\"\n"
				"\"light\" \"400\"\n"
        "\"radius\" \"128\"\n"
        "\"scale\" \"5\"\n"
				"\"target\" \"light_%i_H\"\n"
				"}\n", -(totalWidth / 2) + (x * (width + spacing)) + lightCorners[j][0],
				 -(totalHeight / 2) + (y * (height + spacing)) + lightCorners[j][1],
				 -(height / 2) + 128,
			 	 i));
			offset += strlen(&output[offset]);
		}
		
		strcpy(&output[offset], 
			va("{\n"
			"\"classname\" \"info_notnull\"\n"
			"\"targetname\" \"light_%i_L\"\n"
			"\"origin\" \"%i %i %i\"\n"
			"}\n", i, -(totalWidth / 2) + (x * (width + spacing)) + width / 2,
			 -(totalHeight / 2) + (y * (height + spacing)) + height / 2,
			 -(height / 2) + 128));
		 offset += strlen(&output[offset]);
		 strcpy(&output[offset], 
 			va("{\n"
 			"\"classname\" \"info_notnull\"\n"
 			"\"targetname\" \"light_%i_H\"\n"
 			"\"origin\" \"%i %i %i\"\n"
 			"}\n", i, -(totalWidth / 2) + (x * (width + spacing)) + width / 2,
 			 -(totalHeight / 2) + (y * (height + spacing)) + height / 2,
 			 (height / 2) - 128));
		 offset += strlen(&output[offset]);
	}

	char *weapons[7] = {
//		"weapon_gauntlet",
		"weapon_shotgun",
//		"weapon_machinegun",
		"weapon_grenadelauncher",
		"weapon_rocketlauncher",
		"weapon_lightning",
		"weapon_railgun",
		"weapon_plasmagun",
		"weapon_bfg"
	};

	char *ammoPowerups[7] = {
		//"weapon_gauntlet",
		"ammo_shells",
		//"ammo_bullets",
		"ammo_grenades",
		"ammo_rockets",
		"ammo_lightning",
		"ammo_slugs",
		"ammo_cells",
		"ammo_bfg"
	};

	int corners[3][2] = {
		{32, 32},
		{32, height - 32},
		{width - 32, 32},
//		{width - 32, height - 32}
	};
	

	int ammoCorners[12][2] = {
		{64, 32},
		{32, 64},
		{64, 64},
		{64, height - 32},
		{32, height - 64},
		{64, height - 64},
		{width - 32, 64},
		{width - 64, 32},
		{width - 64, 64},
//		{width - 32, height - 32}
	};
	
	int w = 0;
	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
	  int x = i % cols;

		for(int j = 0; j < 3; j++) {
			char *weap = weapons[w % ARRAY_LEN(weapons)];
			strcpy(&output[offset],
			 va("{\n"
			 "\"classname\" \"%s\"\n"
			 "\"origin\" \"%i %i %i\"\n"
			 "}\n", weap, 
			 	-(totalWidth / 2) + (x * (width + spacing)) + corners[j][0],
				-(totalHeight / 2) + (y * (height + spacing)) + corners[j][1],
				-(height / 2) + 32));
			offset += strlen(&output[offset]);

			// make 3 ammos in every corner
			char *ammo = ammoPowerups[w % ARRAY_LEN(weapons)];
			for(int a = 0; a < 3; a++) {
				strcpy(&output[offset],
				 va("{\n"
				 "\"classname\" \"%s\"\n"
				 "\"origin\" \"%i %i %i\"\n"
				 "}\n", ammo, 
				 	-(totalWidth / 2) + (x * (width + spacing)) + ammoCorners[j * 3 + a][0],
					-(totalHeight / 2) + (y * (height + spacing)) + ammoCorners[j * 3 + a][1],
					-(height / 2) + 32));
				offset += strlen(&output[offset]);
			}

			w++;
		}
	}

	offset += strlen(&output[offset]);
	
	// make teleporters

	return offset;
}

#endif
