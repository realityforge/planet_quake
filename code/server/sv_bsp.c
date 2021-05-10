
#ifdef USE_MEMORY_MAPS

#include "server.h"
#include "../qcommon/cm_public.h"
#include "../game/bg_public.h"
#define MAIN_C
#include "../tools/q3map2/q3map2.h"
#undef MAIN_C



static char stroke[MAX_QPATH] = "";

static char output[4096 * 128] = "";
static dheader_t header;
static int brushC = 0;

static void SV_SetStroke( const char *path ) {
	memcpy(stroke, path, sizeof(stroke));
}

static char *SV_MakeWall( int p1[3], int p2[3] ) {
	static char wall[4096*2];
	int minMaxMap[6][3][3] = {
		{{p1[0], p1[1], p2[2]}, {p1[0], p1[1], p1[2]}, {p1[0], p2[1], p1[2]}},
		{{p2[0], p2[1], p2[2]}, {p2[0], p2[1], p1[2]}, {p2[0], p1[1], p1[2]}},
		{{p2[0], p1[1], p2[2]}, {p2[0], p1[1], p1[2]}, {p1[0], p1[1], p1[2]}},
		{{p1[0], p2[1], p2[2]}, {p1[0], p2[1], p1[2]}, {p2[0], p2[1], p1[2]}},
		{{p1[0], p2[1], p1[2]}, {p1[0], p1[1], p1[2]}, {p2[0], p1[1], p1[2]}},
		{{p1[0], p1[1], p2[2]}, {p1[0], p2[1], p2[2]}, {p2[0], p2[1], p2[2]}},
	};
	wall[0] = '\0';
	brushC++;
	Q_strcat(wall, sizeof(wall), va("// brush %i\n"
		"{\n", brushC));
	for(int i = 0; i < ARRAY_LEN(minMaxMap); i++) {
		Q_strcat(wall, sizeof(wall),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) %s 0 0 0 1 1 0 0 0\n",
			minMaxMap[i][0][0], minMaxMap[i][0][1], minMaxMap[i][0][2],
			minMaxMap[i][1][0], minMaxMap[i][1][1], minMaxMap[i][1][2],
			minMaxMap[i][2][0], minMaxMap[i][2][1], minMaxMap[i][2][2],
			stroke
		));
	}
	Q_strcat(wall, sizeof(wall), "}\n");
	return wall;
}


static char *SV_MakeBox( vec3_t min, vec3_t max ) {
	static char box[4096];
	box[0] = '\0';	
	int  wallMap[12][3] = {
		{min[0], min[1], min[2]-16},
		{max[0], max[1], min[2]},
		
		{min[0]-16, min[1], min[2]},
		{min[0],    max[1], max[2]},
		
		{min[0], min[1]-16, min[2]},
		{max[0], min[1],    max[2]},
		
		
		{min[0], min[1], max[2]},
		{max[0], max[1], max[2]+16},
		
		{max[0],    min[1], min[2]},
		{max[0]+16, max[1], max[2]},
		
		{min[0], max[1],    min[2]},
		{max[0], max[1]+16, max[2]}
	};

	for(int i = 0; i < 6; i++) {
		int *p1 = wallMap[i*2];
		int *p2 = wallMap[i*2+1];
		Q_strcat(box, sizeof(box), SV_MakeWall(p1, p2));
	}

	return box;
}


void SV_MakeSkybox( void ) {
	char skybox[4096 * 2];
	vec3_t  vs[2];
	if(!com_sv_running || !com_sv_running->integer
		|| sv.state != SS_GAME) {
		vs[0][0] = vs[0][1] = vs[0][2] = -2000;
		vs[1][0] = vs[1][1] = vs[1][2] = 2000;
	} else {
		int h = CM_InlineModel( 0, 2, gvm );
		CM_ModelBounds( h, vs[0], vs[1] );
	}

	brushC = 0;
	skybox[0] = '\0';
	Q_strcat(skybox, sizeof(skybox), "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n");
	
	SV_SetStroke("e1u1/sky1");
	Q_strcat(skybox, sizeof(skybox), SV_MakeBox(vs[0], vs[1]));
	
	Q_strcat(skybox, sizeof(skybox), "}\n");

	Q_strcat(skybox, sizeof(skybox), 
		va("{\n"
		"\"classname\" \"misc_skybox\"\n"
		"\"origin\" \"%i %i %i\"\n"
		"}\n", 
		 (int)(vs[1][0] - 64),
		 (int)(vs[1][1] - 64),
		 (int)(vs[1][2] - 64)));

	Q_strcat(skybox, sizeof(skybox), 
		"{\n"
		"\"classname\" \"info_player_start\"\n"
		"\"origin\" \"16 64 -52\"\n"
		"}\n");

	memcpy(output, skybox, sizeof(skybox));
}


// TODO
static char *SV_MakeMaze( void ) {
	return "";
}


// TODO: wall is just a square platform
static char *SV_MakeCube(
	vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4,
	vec3_t p5, vec3_t p6, vec3_t p7, vec3_t p8) {
	static char plat[4096];
	int quadMap[6][3][3] = {
		{{p8[0], p8[1], p8[2]}, {p5[0], p5[1], p5[2]}, {p1[0], p1[1], p1[2]}},
		{{p4[0], p4[1], p4[2]}, {p3[0], p3[1], p3[2]}, {p7[0], p7[1], p7[2]}},
		{{p8[0], p8[1], p8[2]}, {p7[0], p7[1], p7[2]}, {p6[0], p6[1], p6[2]}},
		{{p2[0], p2[1], p2[2]}, {p3[0], p3[1], p3[2]}, {p4[0], p4[1], p4[2]}},
		{{p6[0], p6[1], p6[2]}, {p2[0], p2[1], p2[2]}, {p1[0], p1[1], p1[2]}},
		{{p7[0], p7[1], p7[2]}, {p3[0], p3[1], p3[2]}, {p2[0], p2[1], p2[2]}},
	};
	plat[0] = '\0';
	brushC++;
	Q_strcat(plat, sizeof(plat), va("// brush %i\n"
		"{\n", brushC));
	for(int i = 0; i < ARRAY_LEN(quadMap); i++) {
		Q_strcat(plat, sizeof(plat),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) %s 0 0 0 1 1 0 0 0\n",
			quadMap[i][0][0], quadMap[i][0][1], quadMap[i][0][2],
			quadMap[i][1][0], quadMap[i][1][1], quadMap[i][1][2],
			quadMap[i][2][0], quadMap[i][2][1], quadMap[i][2][2],
			stroke
		));
	}
	Q_strcat(plat, sizeof(plat), "}\n");
	return plat;
}


// TODO: wall is just a square platform
static char *SV_MakePlatform(vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4) {
	static char plat[4096];
	// TODO: make nice with edges like Edge of Oblivion on quake 3 space maps
	return plat;
}


static char *SV_MakeShootsAndLadders(vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4) {
	static char plat[4096];
	// TODO: ramps and wind tunnels like Edge of Oblivion with different shaped pyramids and stuff in space
	return plat;
}


static float circleCorners[8];
static void SV_MakeCircle(int splits, int i, float radius, float width, float height) {
	//splits = ((int)ceil(sqrt(splits))) ^ 2;
	float angle = 360.0 / splits;
	//float padX = (width - radius * 2) / 2;
	float padY = radius;
	float splitsPerSide = splits / 4.0; // sides
	//float diff = splitsPerSide - floor(splits / 4.0);
	float offset = floor(splits / 8.0);
	// alternate which corners are used to form the circle as it goes around, this keeps the brush uniform
	// start from the top left corner
	float x1 = radius * sin(M_PI * 2.0 * (angle * i) / 360.0);
	float y1 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * i) / 360.0)) + padY;
	float x2 = radius * sin(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0);
	float y2 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0)) + padY;
	float x3 = (width / ceil(splitsPerSide)) * (i + 1.0);
	float y3 = (height / 2.0);
	float x4 = (width / ceil(splitsPerSide)) * (i - 0.0);
	float y4 = (height / 2.0);
	
	if(i >= (splitsPerSide * 3.0 - offset)) {
		x3 = x2;
		y3 = y2;
		x2 = x1;
		y2 = y1;
		int numSplits = splits - ceil(splitsPerSide * 3.0);
		x1 = -(width / 2.0);
		y1 = (height / numSplits) * (i - floor(splitsPerSide * 3.0) - 0.0);
		x4 = -(width / 2.0);
		y4 = (height / numSplits) * (i - floor(splitsPerSide * 3.0) + 1.0);
	} else if(i >= (splitsPerSide * 2.0 - offset)) {
		x3 = x1;
		y3 = y1;
		x4 = x2;
		y4 = y2;
		int numSplits = (splits - ceil(splitsPerSide * 2.0)) - (splits - ceil(splitsPerSide * 3.0));
		x1 = (width / numSplits) * (floor(splitsPerSide * 2.0) - i - 1.0);
		y1 = -(height / 2.0);
		x2 = (width / numSplits) * (floor(splitsPerSide * 2.0) - i + 0.0);
		y2 = -(height / 2.0);
	} else if(i >= (splitsPerSide - offset)) {
		x4 = x1;
		y4 = y1;
		x1 = x2;
		y1 = y2;
		int numSplits = round(splitsPerSide * 2.0 - splitsPerSide);
		x3 = (width / 2.0);
		y3 = (height / numSplits) * (round(splitsPerSide) - i + 0.0);
		x2 = (width / 2.0);
		y2 = (height / numSplits) * (round(splitsPerSide) - i - 1.0);
	} else {
	}
	circleCorners[0] = x1 + (width / 2);
	circleCorners[1] = y1 + (height / 2);
	circleCorners[2] = x2 + (width / 2);
	circleCorners[3] = y2 + (height / 2);
	circleCorners[4] = x3 + (width / 2);
	circleCorners[5] = y3 + (height / 2);
	circleCorners[6] = x4 + (width / 2);
	circleCorners[7] = y4 + (height / 2);
}


static char *SV_MakePortal( float radius, vec3_t min, vec3_t max ) {
	static char portal[4096*24];
	float splits = 24.0;
	float offset = floor(splits / 8.0);
	portal[0] = '\0';
	for(float i = -offset; i < ceil(splits - offset); i++) {
		SV_MakeCircle(splits, i, radius, max[0] - min[0], max[1] - min[1]);
		float x1 = circleCorners[0];
		float y1 = circleCorners[1];
		float x2 = circleCorners[2];
		float y2 = circleCorners[3];
		float x3 = circleCorners[4];
		float y3 = circleCorners[5];
		float x4 = circleCorners[6];
		float y4 = circleCorners[7];

		
		vec3_t  wallMap[48] = {
			
			{min[0] + x1, min[1] + y1, min[2] - 16},
			{min[0] + x2, min[1] + y2, min[2] - 16},
			{min[0] + x3, min[1] + y3, min[2] - 16},
			{min[0] + x4, min[1] + y4, min[2] - 16},

			{min[0] + x1, min[1] + y1, min[2]}, 
			{min[0] + x2, min[1] + y2, min[2]}, 
			{min[0] + x3, min[1] + y3, min[2]}, 
			{min[0] + x4, min[1] + y4, min[2]},

			{min[0],      min[1] + x4, min[2] + y4},
			{min[0] - 16, min[1] + x4, min[2] + y4},
			{min[0] - 16, min[1] + x3, min[2] + y3},
			{min[0],      min[1] + x3, min[2] + y3},

			{min[0],      min[1] + x1, min[2] + y1}, 
			{min[0] - 16, min[1] + x1, min[2] + y1}, 
			{min[0] - 16, min[1] + x2, min[2] + y2}, 
			{min[0],      min[1] + x2, min[2] + y2},

			{min[0] + x1, min[1], min[2] + y1}, 
			{min[0] + x2, min[1], min[2] + y2}, 
			{min[0] + x3, min[1], min[2] + y3}, 
			{min[0] + x4, min[1], min[2] + y4},

			{min[0] + x1, min[1] - 16, min[2] + y1}, 
			{min[0] + x2, min[1] - 16, min[2] + y2}, 
			{min[0] + x3, min[1] - 16, min[2] + y3}, 
			{min[0] + x4, min[1] - 16, min[2] + y4},
			
			
			{min[0] + x1, min[1] + y1, max[2]},
			{min[0] + x2, min[1] + y2, max[2]},
			{min[0] + x3, min[1] + y3, max[2]},
			{min[0] + x4, min[1] + y4, max[2]},

			{min[0] + x1, min[1] + y1, max[2] + 16}, 
			{min[0] + x2, min[1] + y2, max[2] + 16}, 
			{min[0] + x3, min[1] + y3, max[2] + 16}, 
			{min[0] + x4, min[1] + y4, max[2] + 16},

			{max[0], min[1] + x1, min[2] + y1},
			{max[0], min[1] + x2, min[2] + y2},
			{max[0], min[1] + x3, min[2] + y3},
			{max[0], min[1] + x4, min[2] + y4},

			{max[0] + 16, min[1] + x1, min[2] + y1}, 
			{max[0] + 16, min[1] + x2, min[2] + y2}, 
			{max[0] + 16, min[1] + x3, min[2] + y3}, 
			{max[0] + 16, min[1] + x4, min[2] + y4},

			{min[0] + x1, max[1] + 16, min[2] + y1}, 
			{min[0] + x2, max[1] + 16, min[2] + y2}, 
			{min[0] + x3, max[1] + 16, min[2] + y3}, 
			{min[0] + x4, max[1] + 16, min[2] + y4},

			{min[0] + x1, max[1], min[2] + y1}, 
			{min[0] + x2, max[1], min[2] + y2}, 
			{min[0] + x3, max[1], min[2] + y3}, 
			{min[0] + x4, max[1], min[2] + y4}
		};

		for(int i = 0; i < 6; i++) {
			Q_strcat(portal, sizeof(portal), SV_MakeCube(
				wallMap[i * 8]    , wallMap[i * 8 + 1], wallMap[i * 8 + 2], wallMap[i * 8 + 3],
				wallMap[i * 8 + 4], wallMap[i * 8 + 5], wallMap[i * 8 + 6], wallMap[i * 8 + 7]
			));
		}
		
	}

	return portal;
}


static int SV_MakeHypercube( void ) {
	float radius = 100.0;
	int offset = 0;
	int width = 1200;
	int height = 1200;
	int spacing = 400;
	int rows = 1;
	int cols = 1;
	int totalWidth = width * cols + spacing * (cols - 1);
	int totalHeight = height * rows + spacing * (rows - 1);
	vec3_t  vs[2];
	int padding = (width - radius * 2) / 2 - 32;

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	brushC = 0;
	output[0] = '\0';
	strcpy(output, "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"\"_color\" \"1.000000 0.776471 0.776471\"\n"
		"\"ambient\" \"5\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_sunlight\" \"3500\"\n");
	offset += strlen(output);

	SV_SetStroke("e1u1/sky1");
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

		SV_SetStroke(va("e1u1/cube%i", i));
		strcpy(&output[offset], SV_MakePortal(radius, vs[0], vs[1]));
		offset += strlen(&output[offset]);
	}

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
			{vs[0][0] + padding, vs[0][1] + padding, vs[0][2]-32},
			{vs[1][0] - padding, vs[1][1] - padding, vs[0][2]-16},
			
			{vs[0][0]-32,    vs[0][1] + padding, vs[0][2] + padding},
			{vs[0][0]-16,    vs[1][1] - padding, vs[1][2] - padding},
			
			{vs[0][0] + padding, vs[0][1]-32,    vs[0][2] + padding},
			{vs[1][0] - padding, vs[0][1]-16,    vs[1][2] - padding},
			
			
			{vs[0][0] + padding, vs[0][1] + padding, vs[1][2]+16},
			{vs[1][0] - padding, vs[1][1] - padding, vs[1][2]+32},
			
			{vs[1][0]+16,    vs[0][1] + padding, vs[0][2] + padding},
			{vs[1][0]+32,    vs[1][1] - padding, vs[1][2] - padding},
			
			{vs[0][0] + padding, vs[1][1]+16,    vs[0][2] + padding},
			{vs[1][0] - padding, vs[1][1]+32,    vs[1][2] - padding}
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

			SV_SetStroke("e1u1/portal1");
			//strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
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
			{vs[1][0] - (width / 2), vs[1][1] - (height / 2), vs[0][2]+32},
			{vs[0][0]+32,            vs[1][1] - (width / 2),  vs[1][2] - (height / 2)},
			{vs[1][0] - (width / 2), vs[0][1]+32,             vs[1][2] - (height / 2)},

			{vs[0][0] + (width / 2), vs[0][1] + (height / 2), vs[1][2]-32},
			{vs[1][0]-32,            vs[0][1] + (width / 2),  vs[0][2] + (height / 2)},
			{vs[0][0] + (width / 2), vs[1][1]-32,             vs[0][2] + (height / 2)}
		};
		
		for(int j = 0; j < 6; j++) {
			// add destinations
			strcpy(&output[offset], 
				va("{\n"
				"\"classname\" \"misc_teleporter_dest\"\n"
				"\"targetname\" \"teleport_%i_%i\"\n"
				"\"origin\" \"%i %i %i\"\n"
				"\"angle\" \"-90\"\n"
				"}\n",
				 (i+1)%(rows*cols), (j+1)%6, 
				 destinationOffsets[j][0], 
				 destinationOffsets[j][1], 
				 destinationOffsets[j][2]));
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

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
	
		strcpy(&output[offset], 
			va("{\n"
			"\"classname\" \"light\"\n"
			"\"origin\" \"%i %i %i\"\n"
			"\"angle\" \"240\"\n"
			"\"light\" \"1000\""
			"}\n", -(totalWidth / 2) + (x * (width + spacing)) + width - 128,
			 -(totalHeight / 2) + (y * (height + spacing)) + height - 128,
			 (height / 2) - 128));

		offset += strlen(&output[offset]);
	}

/*
	int i = 0, j, y, x;
	for ( j = 0 ; j < bg_numItems ; j++ ) {
		if(!bg_itemlist[j].classname
			|| !Q_stristr(bg_itemlist[j].classname, "weapon_")) {
			continue;
		}
		y = i / cols;
	  x = i % cols;

		strcpy(&output[offset],
		 va("{\n"
		 "\"classname\" \"%s\"\n"
		 "\"origin\" \"%i %i %i\"\n"
		 "}\n", bg_itemlist[j].classname, 
		 	-(totalWidth / 2) + (x * (width + spacing)) + 32,
			-(totalHeight / 2) + (y * (height + spacing)) + height - 32,
			-(height / 2) + 32));

		i++;
		offset += strlen(&output[offset]);
	}

	i = 0;
	for ( j = 0 ; j < bg_numItems ; j++ ) {
		if(!bg_itemlist[j].classname
			|| !Q_stristr(bg_itemlist[j].classname, "ammo_")) {
			continue;
		}
		y = i / cols;
	  x = i % cols;

		strcpy(&output[offset],
		 va("{\n"
		 "\"classname\" \"%s\"\n"
		 "\"origin\" \"%i %i %i\"\n"
		 "}\n", bg_itemlist[j].classname,
		  -(totalWidth / 2) + (x * (width + spacing)) + width - 32,
			-(totalHeight / 2) + (y * (height + spacing)) + 32,
			-(height / 2) + 32));
		offset += strlen(&output[offset]);

		strcpy(&output[offset],
		 va("{\n"
		 "\"classname\" \"%s\"\n"
		 "\"origin\" \"%i %i %i\"\n"
		 "}\n", bg_itemlist[j].classname, 
		 	-(totalWidth / 2) + (x * (width + spacing)) + 32,
			-(totalHeight / 2) + (y * (height + spacing)) + 32,
			-(height / 2) + 32));

		i++;
		offset += strlen(&output[offset]);
	}
*/

	offset += strlen(&output[offset]);
	
	// make teleporters

	return offset;
}


static drawVert_t *AddDrawVertsLump( void ){
	int i, size;
	bspDrawVert_t   *in;
	drawVert_t  *buffer, *out;

	if(dDrawVerts) {
		free(dDrawVerts);
	}

	/* allocate output buffer */
	size = numBSPDrawVerts * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspDrawVerts;
	out = buffer;
	for ( i = 0; i < numBSPDrawVerts; i++ )
	{
		VectorCopy( in->xyz, out->xyz );
		out->st[ 0 ] = in->st[ 0 ];
		out->st[ 1 ] = in->st[ 1 ];

		out->lightmap[ 0 ] = in->lightmap[ 0 ][ 0 ];
		out->lightmap[ 1 ] = in->lightmap[ 0 ][ 1 ];

		VectorCopy( in->normal, out->normal );

		out->color[ 0 ] = in->color[ 0 ][ 0 ];
		out->color[ 1 ] = in->color[ 0 ][ 1 ];
		out->color[ 2 ] = in->color[ 0 ][ 2 ];
		out->color[ 3 ] = in->color[ 0 ][ 3 ];

		in++;
		out++;
	}

	dDrawVerts = buffer;
  header.lumps[LUMP_DRAWVERTS].filelen = size;
	
	return buffer;
}


static dbrushside_t *AddBrushSidesLump( void )
{
	int i, size;
	bspBrushSide_t  *in;
	dbrushside_t *buffer, *out;

	if(dBrushSides) {
		free(dBrushSides);
	}

	/* allocate output buffer */
	size = numBSPBrushSides * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspBrushSides;
	out = buffer;
	for ( i = 0; i < numBSPBrushSides; i++ )
	{
		out->planeNum = in->planeNum;
		out->shaderNum = in->shaderNum;
		in++;
		out++;
	}

	dBrushSides = buffer;
  header.lumps[LUMP_BRUSHSIDES].filelen = size;
	
	return buffer;
}


static dsurface_t *AddDrawSurfacesLump( void ){
	int i, size;
	bspDrawSurface_t    *in;
	dsurface_t   *buffer, *out;

	if(dDrawSurfaces) {
		free(dDrawSurfaces);
	}

	/* allocate output buffer */
	size = numBSPDrawSurfaces * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspDrawSurfaces;
	out = buffer;
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		out->shaderNum = in->shaderNum;
		out->fogNum = in->fogNum;
		out->surfaceType = in->surfaceType;
		out->firstVert = in->firstVert;
		out->numVerts = in->numVerts;
		out->firstIndex = in->firstIndex;
		out->numIndexes = in->numIndexes;

		out->lightmapNum = in->lightmapNum[ 0 ];
		out->lightmapX = in->lightmapX[ 0 ];
		out->lightmapY = in->lightmapY[ 0 ];
		out->lightmapWidth = in->lightmapWidth;
		out->lightmapHeight = in->lightmapHeight;

		VectorCopy( in->lightmapOrigin, out->lightmapOrigin );
		VectorCopy( in->lightmapVecs[ 0 ], out->lightmapVecs[ 0 ] );
		VectorCopy( in->lightmapVecs[ 1 ], out->lightmapVecs[ 1 ] );
		VectorCopy( in->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );

		out->patchWidth = in->patchWidth;
		out->patchHeight = in->patchHeight;

		in++;
		out++;
	}

	dDrawSurfaces = buffer;
  header.lumps[LUMP_SURFACES].filelen = size;
	
	return buffer;
}


static byte *AddLightGridLumps( void ){
	int i;
	bspGridPoint_t  *in;
	byte *buffer, *out;

	if(dGridPoints) {
		free(dGridPoints);
	}

	/* dummy check */
	if ( bspGridPoints == NULL ) {
		return 0;
	}

	/* allocate temporary buffer */
	buffer = safe_malloc( numBSPGridPoints * sizeof( *out ) * 8 );

	/* convert */
	in = bspGridPoints;
	out = buffer;
	for ( i = 0; i < numBSPGridPoints; i++ )
	{
		VectorCopy( in->ambient[ 0 ], &out[0] );
		VectorCopy( in->directed[ 0 ], &out[3] );

		out[ 6 ] = in->latLong[ 0 ];
		out[ 7 ] = in->latLong[ 1 ];

		in++;
		out++;
	}

	//dGridPoints = buffer;
  header.lumps[LUMP_LIGHTGRID].filelen = 0; //numBSPGridPoints * sizeof( *out );
	
	return buffer;
}

static int lumpsStupidOrder[] = {
	LUMP_SHADERS, LUMP_PLANES, LUMP_LEAFS, LUMP_NODES,
	LUMP_BRUSHES, LUMP_BRUSHSIDES, LUMP_LEAFSURFACES,
	LUMP_LEAFBRUSHES, LUMP_MODELS, LUMP_DRAWVERTS,
	LUMP_SURFACES, LUMP_VISIBILITY, LUMP_LIGHTMAPS, LUMP_LIGHTGRID,
	LUMP_ENTITIES, LUMP_FOGS, LUMP_DRAWINDEXES, 
};

static void SV_AssignMemoryDatas( void ) {
	memset( &header, 0, sizeof( header ) );
  // TODO: do the same prep that multiworld `load game` command does
  // load all the lumps as if they came from the file
  header.ident = BSP_IDENT;
  header.version = BSP3_VERSION;
	dShaders = (void *)bspShaders;
  header.lumps[LUMP_SHADERS].filelen = numBSPShaders * sizeof( dshader_t );
	dPlanes = (void *)bspPlanes;
  header.lumps[LUMP_PLANES].filelen = numBSPPlanes * sizeof( dplane_t );
	dLeafs = (void *)bspLeafs;
  header.lumps[LUMP_LEAFS].filelen = numBSPLeafs * sizeof( dleaf_t );
	dNodes = (void *)bspNodes;
  header.lumps[LUMP_NODES].filelen = numBSPNodes * sizeof(dnode_t );
	dBrushes = (void *)bspBrushes;
  header.lumps[LUMP_BRUSHES].filelen = numBSPBrushes * sizeof( dbrush_t );
	AddBrushSidesLump();
	dLeafSurfaces = (void *)bspLeafSurfaces;
  header.lumps[LUMP_LEAFSURFACES].filelen = numBSPLeafSurfaces * sizeof( int );
  dLeafBrushes = (void *)bspLeafBrushes;
  header.lumps[LUMP_LEAFBRUSHES].filelen = numBSPLeafBrushes * sizeof( int );
  dModels = (void *)bspModels;
  header.lumps[LUMP_MODELS].filelen = numBSPModels * sizeof( dmodel_t );
	AddDrawVertsLump();
	AddDrawSurfacesLump();
	//dModels = (void *)drawSurfaces;
	//header.lumps[LUMP_SURFACES].filelen = numBSPDrawSurfaces * sizeof( dsurface_t );
  dVisBytes = (void *)bspVisBytes;
  header.lumps[LUMP_VISIBILITY].filelen = numBSPVisBytes;
  dLightBytes = (void *)bspLightBytes;
  header.lumps[LUMP_LIGHTMAPS].filelen = numBSPLightBytes;
	AddLightGridLumps();
  dEntData = (void *)&bspEntData;
  header.lumps[LUMP_ENTITIES].filelen = bspEntDataSize;
  dFogs = (void *)bspFogs;
  header.lumps[LUMP_FOGS].filelen = numBSPFogs * sizeof( dfog_t );
  dDrawIndexes = (void *)bspDrawIndexes;
  header.lumps[LUMP_DRAWINDEXES].filelen = numBSPDrawIndexes * sizeof( int );

	for(int i = 0; i < HEADER_LUMPS; i++) {
		header.lumps[i].fileofs = 0;
	}
}


static void SV_LoadMapFromMemory( void ) {
	SV_AssignMemoryDatas();

	// load into heap
	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs (&header.lumps[LUMP_LEAFS]);
	CMod_LoadLeafBrushes (&header.lumps[LUMP_LEAFBRUSHES]);
	CMod_LoadLeafSurfaces (&header.lumps[LUMP_LEAFSURFACES]);
	CMod_LoadPlanes (&header.lumps[LUMP_PLANES]);
	CMod_LoadBrushSides (&header.lumps[LUMP_BRUSHSIDES]);
	CMod_LoadBrushes (&header.lumps[LUMP_BRUSHES]);
	CMod_LoadSubmodels (&header.lumps[LUMP_MODELS]);
	CMod_LoadNodes (&header.lumps[LUMP_NODES]);
	CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES], "\0");
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );

	/* advertisements */
	//AddLump( file, (bspHeader_t*) header, LUMP_ADVERTISEMENTS, bspAds, numBSPAds * sizeof( bspAdvertisement_t ) );

  // TODO: detect memory map from gamestate on client and download over UDP
  
  // TODO: make a copy of the map in memory in case a client requests, it can be sent
}


void SV_WriteMemoryMapToClient(client_t *cl, int slot) {
	char marker[ 1024 ];
	int curindex;

	SV_AssignMemoryDatas();
	
	time_t t = I_FloatTime();
	//sprintf( marker, "I LOVE QUAKE.GAMES %s on %s)", Q3MAP_VERSION, asctime( localtime( &t ) ) );
	sprintf( marker, "I LOVE MY Q3MAP2 %s on %s)", Q3MAP_VERSION, asctime( localtime( &t ) ) );

	for(int i = 0; i < HEADER_LUMPS; i++) {
		lump_t *lump = &header.lumps[lumpsStupidOrder[i]];
		lump_t *prev = &header.lumps[lumpsStupidOrder[i - 1]];
		if(i == 0)
			lump->fileofs = ((sizeof(dheader_t) + strlen(marker) + 1) + 3) & ~3;
		else
			lump->fileofs = prev->fileofs + ((prev->filelen + 3) & ~3);
//Com_Printf("Lump: %i - %i\n", lump->fileofs, lump->filelen);
	}

	void *orderedLumpDatas[] = {
		dShaders,
		dPlanes,
		dLeafs,
		dNodes,
		dBrushes,
		dBrushSides,
		dLeafSurfaces,
		dLeafBrushes,
		dModels,
		dDrawVerts,
		dDrawSurfaces,
		dVisBytes,
		dLightBytes,
		dGridPoints,
		dEntData,
		dFogs,
		dDrawIndexes
	};

	cl->downloadSize = header.lumps[lumpsStupidOrder[HEADER_LUMPS-1]].fileofs + header.lumps[lumpsStupidOrder[HEADER_LUMPS-1]].filelen;
	// regenerate entire file
	//cl->downloadCurrentBlock = 0;
	//cl->downloadCount = 0;
	Com_Printf("Download %i > %i\n", cl->downloadSize, cl->downloadCount);
	while (cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW &&
		cl->downloadSize > cl->downloadCount) {

		curindex = (cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW);
		cl->downloadBlockSize[curindex] = 0;

		if (!cl->downloadBlocks[curindex]) {
			cl->downloadBlocks[curindex] = Z_Malloc( MAX_DOWNLOAD_BLKSIZE );
		}

		for(int i = 0; i < HEADER_LUMPS; i++) {
			lump_t *lump = &header.lumps[lumpsStupidOrder[i]];
			void *data = orderedLumpDatas[i];
			if(lump->fileofs + lump->filelen < cl->downloadCount) {
				// already past this lump in the download setup
				continue;
			}

			if(cl->downloadCurrentBlock == 0 && cl->downloadCount == 0 && i == 0) {
Com_Printf("Beginning header\n");
				memcpy(&cl->downloadBlocks[curindex][0], &header.ident, sizeof(int));
				memcpy(&cl->downloadBlocks[curindex][4], &header.version, sizeof(int));
				for(int j = 0; j < HEADER_LUMPS; j++) {
					lump_t *lump = &header.lumps[j];
					//if(j != 15) {
						memcpy(&cl->downloadBlocks[curindex][8 + j * 8], &lump->fileofs, sizeof(int));
						memcpy(&cl->downloadBlocks[curindex][12 + j * 8], &lump->filelen, sizeof(int));
					//}
				}
				memcpy(&cl->downloadBlocks[curindex][sizeof(header)], marker, strlen(marker) + 1);
				cl->downloadCount = lump->fileofs;
			}

			//Com_Printf("Lump ofs: %i, %i, %i\n", lumpsStupidOrder[i], lump->fileofs, cl->downloadCount - lump->fileofs);
			if(lump->fileofs - cl->downloadCount > 3) {
				Com_Error(ERR_DROP, "Should never happen because the previous loop should fill or break.");
			} else {
				int fillStart = (cl->downloadCount % MAX_DOWNLOAD_BLKSIZE);
				if(lump->fileofs + lump->filelen > (cl->downloadCurrentBlock + 1) * MAX_DOWNLOAD_BLKSIZE) {
					// fill the whole block
					cl->downloadBlockSize[curindex] = MAX_DOWNLOAD_BLKSIZE;
					// diff from count because previous loop might have been a partial lump
					int diffLength = MAX_DOWNLOAD_BLKSIZE - fillStart;
					if(diffLength > 0) {
						memcpy(&cl->downloadBlocks[curindex][fillStart], &data[cl->downloadCount - lump->fileofs], diffLength);
					}
					Com_Printf("Lump fill (%i, %i): %i, %i (%i left)\n", cl->downloadCurrentBlock, lumpsStupidOrder[i],
					  cl->downloadBlockSize[curindex], lump->filelen,
					  (lump->fileofs + lump->filelen) - (cl->downloadCurrentBlock + 1) * MAX_DOWNLOAD_BLKSIZE);
					cl->downloadCount += diffLength;
					break;
				} else {
					// fill partially with this lump, then loop and fill with next lump
					int remainingLength = (lump->fileofs + lump->filelen) - cl->downloadCount;
					cl->downloadBlockSize[curindex] += remainingLength;
					if(remainingLength > 0) {
						memcpy(&cl->downloadBlocks[curindex][fillStart], &data[cl->downloadCount - lump->fileofs], remainingLength);
					}
					Com_Printf("Lump end (%i, %i): %i, %i\n", cl->downloadCurrentBlock, lumpsStupidOrder[i],
					 	cl->downloadBlockSize[curindex], lump->filelen);
					cl->downloadCount += remainingLength;
					// loop back around and start on new lump
					if(cl->downloadBlockSize[curindex] >= MAX_DOWNLOAD_BLKSIZE) {
						break;
					}
				}
			}
		}
		if(cl->downloadClientBlock * MAX_DOWNLOAD_BLKSIZE > cl->downloadSize + MAX_DOWNLOAD_BLKSIZE) {
			Com_Error(ERR_DROP, "Should never happen!\n");
		}

		// Load in next block
		if(cl->downloadCount >= cl->downloadSize) {
			cl->downloadCurrentBlock++;
			break;
		} else if (cl->downloadBlockSize[curindex] == MAX_DOWNLOAD_BLKSIZE) {
			cl->downloadCurrentBlock++;
		} else {
			Com_Error(ERR_DROP, "Should never happen because block should be filled with data\n");
		}
	}
}


int SV_MakeMap( void ) {

	int length = SV_MakeHypercube();
	
	int result = CM_LoadMapFromMemory();

	fileHandle_t mapfile = FS_SV_FOpenFileWrite( va("*memory%i.map", result) );
	FS_Write( output, length, mapfile );    // overwritten later
	FS_FCloseFile( mapfile );

	BSPMemory(output, result);

  SV_LoadMapFromMemory();
	
	return result;
}

#endif
