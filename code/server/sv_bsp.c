
#ifdef USE_MEMORY_MAPS

#include "server.h"
#include "../qcommon/vm_local.h"
#include "../qcommon/cm_public.h"
#include "../game/bg_public.h"


static char stroke[MAX_QPATH] = "";

static char output[4096 * 256] = "";
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
#ifdef USE_MULTIVM_SERVER
		int h = CM_InlineModel( 0, 2, gvmi );
#else
    int h = CM_InlineModel( 0, 2, 0 );
#endif
		CM_ModelBounds( h, vs[0], vs[1] );
	}

	brushC = 0;
	skybox[0] = '\0';
	Q_strcat(skybox, sizeof(skybox), "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n");
	
	SV_SetStroke("sky1");
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
	// get both ends of "circular" edge by adding 1 to i and calculating the next x and y  coordinates
	float x2 = radius * sin(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0);
	float y2 = -radius * (1.0 - cos(M_PI * 2.0 * (angle * (i + 1.0)) / 360.0)) + padY;
	// split the edge of the box evenly to make a square border around the circle
	// so the wall looks like we punched a hole in it with a cookie cutter
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


#define  SIDE_NORTH  1
#define  SIDE_EAST   2
#define  SIDE_SOUTH  4
#define  SIDE_WEST   8
#define  SIDE_TOP    16
#define  SIDE_BOTTOM 32
#define  SIDE_ALL    63 // ?


static char *SV_MakePortal( float radius, vec3_t min, vec3_t max, int minSegment, int maxSegment, int sides ) {
	static char portal[4096*24];
	int thickness = 16;
	int splits = 24.0;
	int offset = floor(splits / 8.0);
	portal[0] = '\0';
	for(int i = -offset; i < ceil(splits - offset); i++) {
		if((minSegment == -1 && maxSegment == -1) 
			|| ((i + offset) % splits <= maxSegment
			&& (i + offset) % splits >= minSegment)) {
			SV_MakeCircle(splits, i, radius, max[0] - min[0], max[1] - min[1]);
		} else {
			continue;
		}
		float x1 = circleCorners[0];
		float y1 = circleCorners[1];
		float x2 = circleCorners[2];
		float y2 = circleCorners[3];
		float x3 = circleCorners[4];
		float y3 = circleCorners[5];
		float x4 = circleCorners[6];
		float y4 = circleCorners[7];

		
		vec3_t  wallMap[48] = {
			
			{min[0] + x1, min[1] + y1, min[2]},
			{min[0] + x2, min[1] + y2, min[2]},
			{min[0] + x3, min[1] + y3, min[2]},
			{min[0] + x4, min[1] + y4, min[2]},

			{min[0] + x1, min[1] + y1, min[2] + thickness}, 
			{min[0] + x2, min[1] + y2, min[2] + thickness}, 
			{min[0] + x3, min[1] + y3, min[2] + thickness}, 
			{min[0] + x4, min[1] + y4, min[2] + thickness},

			{min[0] + thickness, min[1] + x4, min[2] + y4},
			{min[0],      min[1] + x4, min[2] + y4},
			{min[0],      min[1] + x3, min[2] + y3},
			{min[0] + thickness, min[1] + x3, min[2] + y3},

			{min[0] + thickness, min[1] + x1, min[2] + y1}, 
			{min[0],      min[1] + x1, min[2] + y1}, 
			{min[0],      min[1] + x2, min[2] + y2}, 
			{min[0] + thickness, min[1] + x2, min[2] + y2},

			{min[0] + x1, min[1] + thickness, min[2] + y1}, 
			{min[0] + x2, min[1] + thickness, min[2] + y2}, 
			{min[0] + x3, min[1] + thickness, min[2] + y3}, 
			{min[0] + x4, min[1] + thickness, min[2] + y4},

			{min[0] + x1, min[1], min[2] + y1}, 
			{min[0] + x2, min[1], min[2] + y2}, 
			{min[0] + x3, min[1], min[2] + y3}, 
			{min[0] + x4, min[1], min[2] + y4},
			
			
			{min[0] + x1, min[1] + y1, max[2] - thickness},
			{min[0] + x2, min[1] + y2, max[2] - thickness},
			{min[0] + x3, min[1] + y3, max[2] - thickness},
			{min[0] + x4, min[1] + y4, max[2] - thickness},

			{min[0] + x1, min[1] + y1, max[2]}, 
			{min[0] + x2, min[1] + y2, max[2]}, 
			{min[0] + x3, min[1] + y3, max[2]}, 
			{min[0] + x4, min[1] + y4, max[2]},

			{max[0] - thickness, min[1] + x1, min[2] + y1},
			{max[0] - thickness, min[1] + x2, min[2] + y2},
			{max[0] - thickness, min[1] + x3, min[2] + y3},
			{max[0] - thickness, min[1] + x4, min[2] + y4},

			{max[0], min[1] + x1, min[2] + y1}, 
			{max[0], min[1] + x2, min[2] + y2}, 
			{max[0], min[1] + x3, min[2] + y3}, 
			{max[0], min[1] + x4, min[2] + y4},

			{min[0] + x1, max[1], min[2] + y1}, 
			{min[0] + x2, max[1], min[2] + y2}, 
			{min[0] + x3, max[1], min[2] + y3}, 
			{min[0] + x4, max[1], min[2] + y4},

			{min[0] + x1, max[1] - thickness, min[2] + y1}, 
			{min[0] + x2, max[1] - thickness, min[2] + y2}, 
			{min[0] + x3, max[1] - thickness, min[2] + y3}, 
			{min[0] + x4, max[1] - thickness, min[2] + y4}
		};

		for(int i = 0; i < 6; i++) {
			if(!((i == 0 && sides & SIDE_TOP)
				|| (i == 1 && sides & SIDE_WEST)
				|| (i == 2 && sides & SIDE_SOUTH)
				|| (i == 3 && sides & SIDE_TOP)
				|| (i == 4 && sides & SIDE_EAST)
				|| (i == 5 && sides & SIDE_NORTH)
				))
				continue;
			Q_strcat(portal, sizeof(portal), SV_MakeCube(
				wallMap[i * 8]    , wallMap[i * 8 + 1], wallMap[i * 8 + 2], wallMap[i * 8 + 3],
				wallMap[i * 8 + 4], wallMap[i * 8 + 5], wallMap[i * 8 + 6], wallMap[i * 8 + 7]
			));
		}
	}
	return portal;
}


static int SV_MakeHypercube( void ) {
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

	brushC = 0;
	output[0] = '\0';
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


// TODO
static int SV_MakeMaze( void ) {
	// create a 4D maze, actually just 4 colored mazes stacked, 
	//   maybe add some nice windows that look like the skybox before impossibly going around the corridor
	// something about recursive division seems beautiful relevent to square brushes
	//   https://en.wikipedia.org/wiki/Maze_generation_algorithm#Recursive_division_method
	int offset = 0;
	int cellWidth = 200;
	int cellHeight = 200;
	int gridRows = 16; // do double so #### represents walls and space represent hallways
	int gridCols = 16; // 17 x 17??
	gridRows |= 1; // make odd or +1 so theres a wall on both sides
	gridCols |= 1;
	int thickness = 16;
	int spacing = 200;
	int totalWidth = cellWidth * (gridCols / 2) + thickness * ((gridCols / 2) - 1);
	int totalHeight = cellHeight * (gridRows / 2) + thickness * ((gridRows / 2) - 1);
	vec3_t  vs[2];

	// layout the maze just for debugging
	char maze[gridCols][gridRows];
	for(int x = 0; x < gridCols; x++) {
		for(int y = 0; y < gridRows; y++) {
			maze[0][y] = '#';
			maze[x][0] = '#';
			maze[gridCols-1][y] = '#';
			maze[x][gridRows-1] = '#';
		}
	}
	int *areaStack = Hunk_AllocateTempMemory(gridRows * gridCols * 4 * sizeof(int));
	int minX;
	int minY;
	int maxX;
	int maxY;

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	brushC = 0;
	output[0] = '\0';
	strcpy(output, "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"\"_color\" \"1 1 1\"\n"
		"\"message\" \"Deathmaze\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_ambient\" \"10\"\n"
		"\"gridsize\" \"512.0 512.0 512.0\"\n"
  );
	offset += strlen(output);

	SV_SetStroke("sky1");
	strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
	offset += strlen(&output[offset]);
	
  
  // make 4 maze floors stacked
	for(int m = 0; m < 4; m++) {
		int safety = 0;
		int stackI = 0;
		memset(maze, ' ', sizeof(maze));

		// make offsets for centering
		vs[0][0] = -(totalWidth / 2);
		vs[1][0] = +(totalWidth / 2);

		vs[0][1] = -(totalHeight / 2);
		vs[1][1] = +(totalHeight / 2);

		vs[0][2] = -(2 * (cellWidth + spacing)) + m * (cellWidth + spacing);
		vs[1][2] = vs[0][2] + cellHeight;
		
    // make outside walls of the maze
    
		SV_SetStroke(va("cube%i", m));
		for(int i = 0; i < 4; i++) {
			if(i % 2 == 0) {
				int start = (rand() % (gridCols / 2)); // + 2; // increase the chances of overlaping a corner by 2 ?
				int end = (rand() % (gridCols / 2)); // + 2;
				if(start > end) {
					int tmp = start;
					start = end;
					end = tmp;
				}
				vec3_t windowOffsets[4] = {
					{vs[0][0] + start * (cellWidth + thickness),             i == 2 
            ? (vs[0][1] - thickness)              : (vs[1][1] - cellHeight + thickness), vs[0][2]},
					{vs[0][0] + start * (cellWidth + thickness) + cellWidth, i == 2 
            ? (vs[0][1] + cellHeight - thickness) : (vs[1][1] + thickness), vs[1][2]},
					
					{vs[0][0] + end * (cellWidth + thickness),             i == 2 
            ? (vs[0][1] - thickness)              : (vs[1][1] - cellHeight) + thickness, vs[0][2]},
					{vs[0][0] + end * (cellWidth + thickness) + cellWidth, i == 2 
            ? (vs[0][1] + cellHeight - thickness) : (vs[1][1] + thickness), vs[1][2]}
				};
				int side = SIDE_NORTH;
				if(i == 2) {
					side = SIDE_SOUTH;
				}
				// TODO: something else instead of repeating  this?
				strcpy(&output[offset], SV_MakePortal(50.0, windowOffsets[0], windowOffsets[1], -3, 2, side));
				offset += strlen(&output[offset]);
				strcpy(&output[offset], SV_MakePortal(50.0, windowOffsets[0], windowOffsets[1], 15, 23, side));
				offset += strlen(&output[offset]);

				strcpy(&output[offset], SV_MakePortal(50.0, windowOffsets[2], windowOffsets[3], 3, 14, side));
				offset += strlen(&output[offset]);
				
				int windowWalls[8][3] = {
					{vs[0][0],                                   i == 2 ? (vs[0][1] - thickness) : vs[1][1], vs[0][2]},
					{vs[0][0] + start * (cellWidth + thickness), i == 2 ? vs[0][1] : (vs[1][1] + thickness), vs[1][2]},
					
					// halfway from starting cell on top and bottom of radius
					{vs[0][0] + start * (cellWidth + thickness) + (cellWidth / 2), 
					 i == 2 ? (vs[0][1] - thickness) : vs[1][1], 
					 vs[0][2]},
					{vs[0][0] + end * (cellWidth + thickness) + (cellWidth / 2),   
					 i == 2 ? vs[0][1] : (vs[1][1] + thickness), 
					 vs[0][2] + (cellHeight - 100) / 2},
					
					{vs[0][0] + start * (cellWidth + thickness) + (cellWidth / 2), 
					 i == 2 ? (vs[0][1] - thickness) : vs[1][1], 
					 vs[1][2] - (cellHeight - 100) / 2},
					{vs[0][0] + end * (cellWidth + thickness) + (cellWidth / 2),   
					 i == 2 ? vs[0][1] : (vs[1][1] + thickness), 
					 vs[1][2]},
					
					{vs[0][0] + (end + 1) * (cellWidth + thickness) - thickness,   i == 2 
            ? (vs[0][1] - thickness) : vs[1][1], vs[0][2]},
					{vs[1][0],                                         i == 2 
            ? vs[0][1] : (vs[1][1] + thickness), vs[1][2]}
				};
				for(int j = 0; j < 4; j++) {
					// skip side walls when spanning the whole length
					if(start == 0 && j == 0) continue;
					if(start == end && j > 0 && j < 3) continue;
					if(end == gridCols/2 - 1 && j == 3) continue;

					strcpy(&output[offset], SV_MakeWall(windowWalls[j*2], windowWalls[j*2+1]));
					offset += strlen(&output[offset]);
				}
			} else {
				int start = (rand() % (gridRows / 2)); // + 2; // increase the chances of overlaping a corner by 2 ?
				int end = (rand() % (gridRows / 2)); // + 2;
				if(start > end) {
					int tmp = start;
					start = end;
					end = tmp;
				}
				vec3_t windowOffsets[4] = {
					{i == 3 ? (vs[0][0] - thickness)             : (vs[1][0] - cellWidth + thickness), 
            vs[0][1] + start * (cellHeight + thickness),              vs[0][2]},
					{i == 3 ? (vs[0][0] + cellWidth - thickness) : (vs[1][0] + thickness),             
            vs[0][1] + start * (cellHeight + thickness) + cellHeight, vs[1][2]},
					
					{i == 3 ? (vs[0][0] - thickness)             : (vs[1][0] - cellWidth) + thickness, 
            vs[0][1] + end * (cellHeight + thickness),              vs[0][2]},
					{i == 3 ? (vs[0][0] + cellWidth - thickness) : (vs[1][0] + thickness),             
            vs[0][1] + end * (cellHeight + thickness) + cellHeight, vs[1][2]}
				};
				int side = SIDE_EAST;
				if(i == 3) {
					side = SIDE_WEST;
				}
				// TODO: something else instead of repeating  this?
				strcpy(&output[offset], SV_MakePortal(50.0, windowOffsets[0], windowOffsets[1], -3, 2, side));
				offset += strlen(&output[offset]);
				strcpy(&output[offset], SV_MakePortal(50.0, windowOffsets[0], windowOffsets[1], 15, 23, side));
				offset += strlen(&output[offset]);

				strcpy(&output[offset], SV_MakePortal(50.0, windowOffsets[2], windowOffsets[3], 3, 14, side));
				offset += strlen(&output[offset]);
				
				int windowWalls[8][3] = {
					{i == 3 ? (vs[0][0] - thickness) : vs[1][0], vs[0][1],                                    vs[0][2]},
					{i == 3 ? vs[0][0] : (vs[1][0] + thickness), vs[0][1] + start * (cellHeight + thickness), vs[1][2]},
					
					// halfway from starting cell on top and bottom of radius
					{i == 3 ? (vs[0][0] - thickness) : vs[1][0], 
					 vs[0][1] + start * (cellHeight + thickness) + (cellHeight / 2), 
  				 vs[0][2]},
					{i == 3 ? vs[0][0] : (vs[1][0] + thickness), 
					 vs[0][1] + end * (cellHeight + thickness) + (cellHeight / 2),   
  				 vs[0][2] + (cellHeight - 100) / 2},
					
					{i == 3 ? (vs[0][0] - thickness) : vs[1][0], 
					 vs[0][1] + start * (cellHeight + thickness) + (cellHeight / 2), 
  				 vs[1][2] - (cellHeight - 100) / 2},
					{i == 3 ? vs[0][0] : (vs[1][0] + thickness), 
					 vs[0][1] + end * (cellHeight + thickness) + (cellHeight / 2),   
  				 vs[1][2]},
					
					{i == 3 ? (vs[0][0] - thickness) : vs[1][0], vs[0][1] + (end + 1) * (cellWidth + thickness) - thickness,   
            vs[0][2]},
					{i == 3 ? vs[0][0] : (vs[1][0] + thickness), vs[1][1],                                                     
            vs[1][2]}
				};
				for(int j = 0; j < 4; j++) {
					// skip side walls when spanning the whole length
					if(start == 0 && j == 0) continue;
					if(start == end && j > 0 && j < 3) continue;
					if(end == gridRows/2 - 1 && j == 3) continue;

					strcpy(&output[offset], SV_MakeWall(windowWalls[j*2], windowWalls[j*2+1]));
					offset += strlen(&output[offset]);
				}
			}
		}
		
		
		// TODO: save holes higher up so teleporters and trigger_push can be added in seperate loops
		//   the top of the previous maze matches the bottom of the next maze
		int holes[3];

		// randomly select 3 holes in every maze, 
		for(int h = 0; h < 3; h++) {
			holes[h] = rand() % ((gridCols / 2) * (gridRows / 2));
		}

		// sort the holes
		int min, max, med;
		if( holes[0] > holes[1] ){
		 if( holes[0] > holes[2] ){
		  max = holes[0];
		  if( holes[1] > holes[2] ){
		   med = holes[1];
		   min = holes[2];
		  }else{
		   med = holes[2];
		   min = holes[1];
		  }
		 }else{
		  med = holes[0];
		  max = holes[2];
		  min = holes[1];
		 }
		}else{
		 if( holes[1] > holes[2] ){
		  max = holes[1];
		  if( holes[0] > holes[2] ){
		   med = holes[0];
		   min = holes[2];
		  }else{
		   med = holes[2];
		   min = holes[0];
		  }
		 }else{
		  med = holes[1];
		  max = holes[2];
		  min = holes[0];
		 }
		}
		holes[0] = min;
		holes[1] = med;
		holes[2] = max;

		// draw the floors and ceilings inbetween
		for(int hi = 0; hi < 3; hi++) {
			int x = holes[hi] % (gridCols / 2);
			int y = holes[hi] / (gridCols / 2);
			int x0 = hi > 0 ? holes[hi-1] % (gridCols / 2) : 0;
			int y0 = hi > 0 ? holes[hi-1] / (gridCols / 2) : 0;
			int x2 = hi < 2 ? holes[hi+1] % (gridCols / 2) : 0;
			int y2 = hi < 2 ? holes[hi+1] / (gridCols / 2) : 0;
			

			// make a hole in the floor, then fill it in 
			int floorOffsets[8][3] = {
        // fill in wide side
				{vs[0][0], hi == 0 ? vs[0][1] : (vs[0][1] + (y0 + 1) * (cellHeight + thickness)), vs[0][2] - thickness}, 
				{vs[1][0], vs[0][1] + y * (cellHeight + thickness), vs[0][2]},

				// fill in long sides, up to hole
				{hi > 0 && y0 == y ? (vs[0][0] + (x0 + 1) * (cellWidth + thickness)) : vs[0][0], 
				 vs[0][1] + y * (cellHeight + thickness), 
				 vs[0][2] - thickness},
				{vs[0][0] + x * (cellWidth + thickness), 
				 vs[0][1] + y * (cellHeight + thickness) + cellHeight + thickness, 
				 vs[0][2]},
				
				{vs[0][0] + (x + 1) * (cellWidth + thickness), 
				 vs[0][1] + y * (cellHeight + thickness), 
 				 vs[0][2] - thickness},
				{hi < 2 && y2 == y ? (vs[0][0] + x2 * (cellWidth + thickness)) : vs[1][0], 
				 vs[0][1] + y * (cellHeight + thickness) + cellHeight + thickness, 
 				 vs[0][2]},

				{vs[0][0], vs[0][1] + (y + 1) * (cellHeight + thickness), vs[0][2] - thickness},
				{vs[1][0], hi == 2 ? vs[1][1] : (vs[0][1] + y2 * (cellHeight + thickness)), vs[0][2]}, // fill in wide side
				// TODO: add bottom or top of next floor here
			};

			for(int w = 0; w < 4; w++) {
				// skip spacing floor if there is no space between holes
				if(y == 0 && w == 0) continue;
				if(x == 0 && w == 1) continue;
				if(x == (gridCols / 2) - 1 && w == 2) continue;
				if(y == (gridRows / 2) - 1 && w == 3) continue;
				// zero thickness walls just means one of the squares is on the same line or next to each other
				if(floorOffsets[w*2+1][1] - floorOffsets[w*2][1] <= 0) continue;
				if(floorOffsets[w*2+1][0] - floorOffsets[w*2][0] <= 0) continue;
				// don't repeat inner floors
				if(y0 == y && hi > 0 && w == 1) continue;
				if(hi < 2 && w == 3) continue;
				//Com_Printf("Making wall: %i x %i <> %i x %i\n", floorOffsets[w*2][0], floorOffsets[w*2][1],
				//	floorOffsets[w*2+1][0], floorOffsets[w*2+1][1]);
				strcpy(&output[offset], SV_MakeWall(floorOffsets[w*2], floorOffsets[w*2+1]));
				offset += strlen(&output[offset]);
			}
		}

		// build inner maze using a areaStack to list every division, skips divisions that would be too small
		while(safety < 6) {
			// initialize the spaces with the entire maze
			stackI--;
			if(stackI == -1) {
				minX = 1;
				minY = 1;
				maxX = (gridCols / 2);
				maxY = (gridRows / 2);
				stackI = 1;
			}
			else if(stackI == 0) {
				break;
			} else {
				minX = areaStack[stackI*4+0];
				minY = areaStack[stackI*4+1];
				maxX = areaStack[stackI*4+2];
				maxY = areaStack[stackI*4+3];
			}
			Com_Printf("Maze block: %i x %i <> %i x %i\n", minX, minY, maxX, maxY);
			int whichDirection = rand() % 4;
			int wallX = (rand() % (maxX - minX)) + minX;
			int wallY = (rand() % (maxY - minY)) + minY;

			Com_Printf("Maze walls: %i x %i\n", wallX, wallY);
			// add the 4 walls to the stack for sub-dividing
			if(wallX - minX > 1
				&& wallY - minY > 1) {
				//Com_Printf("Adding top, left: %i x %i <> %i x %i\n", minX, minY, wallX, wallY);
				areaStack[stackI*4+0] = minX;
				areaStack[stackI*4+1] = minY;
				areaStack[stackI*4+2] = wallX;
				areaStack[stackI*4+3] = wallY;
				stackI++;
			}
			if(wallX - minX > 1
				&& maxY - wallY > 1) {
				//Com_Printf("Adding bottom, left: %i x %i <> %i x %i\n", minX, wallY, wallX, maxY);
				areaStack[stackI*4+0] = minX;
				areaStack[stackI*4+1] = wallY + 1;
				areaStack[stackI*4+2] = wallX;
				areaStack[stackI*4+3] = maxY;
				stackI++;
			}
			if(maxX - wallX > 1
				&& wallY - minY > 1) {
				//Com_Printf("Adding top, right: %i x %i <> %i x %i\n", wallX, minY, maxX, wallY);
				areaStack[stackI*4+0] = wallX + 1;
				areaStack[stackI*4+1] = minY;
				areaStack[stackI*4+2] = maxX;
				areaStack[stackI*4+3] = wallY;
				stackI++;
			}
			if(maxX - wallX > 1
				&& maxY - wallY > 1) {
				//Com_Printf("Adding bottom, right: %i x %i <> %i x %i\n", wallX, wallY, maxX, maxY);
				areaStack[stackI*4+0] = wallX + 1;
				areaStack[stackI*4+1] = wallY + 1;
				areaStack[stackI*4+2] = maxX;
				areaStack[stackI*4+3] = maxY;
				stackI++;
			}

			for(int i = 0; i < 3; i++) {
				//if(i == 1) continue;
				// make 4 or 5 walls around 3 gaps dividing 4 spaces, 
				//   guarunteed path to every edge
				
				/* closest a wall can get to edge
				# #  3 characters for border and hallway
				# #      ##  Would just create a double thick wall
				#        ##  
				# #      ##
				*/
				// this is probably degraded to rand() * 2 due to clustering in cheap time 
				//   based rand(), most rand()s are subject to clustering
				int gap;

				if((whichDirection+i)%2==0) {
					if((whichDirection+i)%4 < 2) { // already place 2 walls must be on the other side
						if(wallX - minX <= 1) gap = wallX - 1;
						else gap = ((rand() % (wallX - minX)) + minX);

						int wall[2][3] = {
							{vs[0][0] + (minX - 1) * (cellWidth + thickness),    
							 vs[0][1] + wallY * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + gap * (cellWidth + thickness),
							 vs[0][1] + wallY * (cellHeight + thickness) + thickness,
							 vs[1][2]}
						};
						if(wall[1][0] != wall[0][0]) {
							strcpy(&output[offset], SV_MakeWall(wall[0], wall[1]));
							offset += strlen(&output[offset]);
						}
						int wall2[2][3] = {
							{vs[0][0] + (gap + 1) * (cellWidth + thickness),    
							 vs[0][1] + wallY * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + (i == 1 
								? maxX * (cellWidth + thickness)
								: wallX * (cellWidth + thickness)),
							 vs[0][1] + wallY * (cellHeight + thickness) + thickness,
							 vs[1][2]}
						};
						if(wall2[1][0] != wall2[0][0]) {
							strcpy(&output[offset], SV_MakeWall(wall2[0], wall2[1]));
							offset += strlen(&output[offset]);
						}
            
					} else {
						if(maxX - wallX <= 1) gap = wallX;
						else gap = ((rand() % (maxX - wallX)) + wallX);

						int wall[2][3] = {
							{vs[0][0] + (i == 1
								? (minX - 1) * (cellWidth + thickness)
								: wallX * (cellWidth + thickness)),    
							 vs[0][1] + wallY * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + gap * (cellWidth + thickness),
							 vs[0][1] + wallY * (cellHeight + thickness) + thickness,
							 vs[1][2]}
						};
						if(wall[1][0] != wall[0][0]) {
							strcpy(&output[offset], SV_MakeWall(wall[0], wall[1]));
							offset += strlen(&output[offset]);
						}
						int wall2[2][3] = {
							{vs[0][0] + (gap + 1) * (cellWidth + thickness),    
							 vs[0][1] + wallY * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + maxX * (cellWidth + thickness),
							 vs[0][1] + wallY * (cellHeight + thickness) + thickness,
							 vs[1][2]}
						};
						if(wall2[1][0] != wall2[0][0]) {
							strcpy(&output[offset], SV_MakeWall(wall2[0], wall2[1]));
							offset += strlen(&output[offset]);
						}
            
            
					}
	//Com_Printf("Maze door: %i x %i\n", gap * 2 + 1, wallY * 2);
					for(int fillX = (minX - 1) * 2; fillX <= maxX * 2; fillX++) {
						if(maze[fillX][wallY*2] == '*') 
							continue;
						else if(fillX == gap * 2 + 1)
							maze[fillX][wallY*2] = '*';
						else 
							maze[fillX][wallY*2] = '#';
					}
					
					// TODO: SV_MakeWall once or twice depending on gap > min and < max - 2
				} else {
					if((whichDirection+i)%4 < 2) {
						if(wallY - minY <= 1) gap = wallY - 1;
						else gap = ((rand() % (wallY - minY)) + minY);

						int wall[2][3] = {
							{vs[0][0] + wallX * (cellWidth + thickness),    
							 vs[0][1] + (minY - 1) * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + wallX * (cellWidth + thickness) + thickness,
							 vs[0][1] + gap * (cellHeight + thickness),
							 vs[1][2]}
						};
						if(wall[1][1] != wall[0][1]) {
							strcpy(&output[offset], SV_MakeWall(wall[0], wall[1]));
							offset += strlen(&output[offset]);
						}
						int wall2[2][3] = {
							{vs[0][0] + wallX * (cellWidth + thickness),    
							 vs[0][1] + (gap + 1) * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + wallX * (cellWidth + thickness) + thickness,
							 vs[0][1] + (i == 1 
							 	? maxY * (cellHeight + thickness)
							  : wallY * (cellHeight + thickness)),
							 vs[1][2]}
						};
						if(wall2[1][1] != wall2[0][1]) {
							strcpy(&output[offset], SV_MakeWall(wall2[0], wall2[1]));
							offset += strlen(&output[offset]);
						}
					} else {
						if(maxY - wallY <= 1) gap = wallY;
						else gap = ((rand() % (maxY - wallY)) + wallY);

						int wall[2][3] = {
							{vs[0][0] + wallX * (cellWidth + thickness),    
							 vs[0][1] + (i == 1 // no gap across from it
							 ? (minY - 1) * (cellHeight + thickness)
							 : wallY * (cellHeight + thickness)), 
							 vs[0][2]},

							{vs[0][0] + wallX * (cellWidth + thickness) + thickness,
							 vs[0][1] + gap * (cellHeight + thickness),
							 vs[1][2]}
						};
						if(wall[1][1] != wall[0][1]) {
							strcpy(&output[offset], SV_MakeWall(wall[0], wall[1]));
							offset += strlen(&output[offset]);
						}
						int wall2[2][3] = {
							{vs[0][0] + wallX * (cellWidth + thickness),    
							 vs[0][1] + (gap + 1) * (cellHeight + thickness), 
							 vs[0][2]},

							{vs[0][0] + wallX * (cellWidth + thickness) + thickness,
							 vs[0][1] + maxY * (cellHeight + thickness),
							 vs[1][2]}
						};
						if(wall2[1][1] != wall2[0][1]) {
							strcpy(&output[offset], SV_MakeWall(wall2[0], wall2[1]));
							offset += strlen(&output[offset]);
						}
					}
	//Com_Printf("Maze door: %i x %i\n", wallX * 2, gap * 2 + 1);
					for(int fillY = (minY - 1) * 2; fillY <= maxY * 2; fillY++) {
						if(maze[wallX*2][fillY] == '*') 
							continue;
						else if(fillY == gap * 2 + 1)
							maze[wallX*2][fillY] = '*';
						else 
							maze[wallX*2][fillY] = '#';
					}
          
          
				}
			}

			safety++;
		} // end while

		Com_Printf("Maze:\n");
		for(int y = 0; y < gridRows; y++) {
			for(int x = 0; x < gridCols; x++) {
				Com_Printf("%c", maze[x][y]);
			}
			Com_Printf("\n");
		}
		Com_Printf("\n");
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
	
	Hunk_FreeTempMemory( areaStack );
	
	for(int m = 0; m < 4; m++) {
		// make offsets for centering
		vs[0][0] = -(totalWidth / 2);
		vs[1][0] = +(totalWidth / 2);

		vs[0][1] = -(totalHeight / 2);
		vs[1][1] = +(totalHeight / 2);

		vs[0][2] = -(2 * (cellWidth + spacing)) + m * (cellWidth + spacing);
		vs[1][2] = vs[0][2] + cellHeight;
		
		// TODO: randomly place entry points
		strcpy(&output[offset], 
			va("{\n"
			"\"classname\" \"info_player_start\"\n"
			"\"origin\" \"%i %i %i\"\n"
			"}\n", 
			(int)(vs[0][0] + 64),
			(int)(vs[0][1] + 64),
			(int)(vs[0][2] + 32)));
		offset += strlen(&output[offset]);
	}
  
  // make a grid of lights
  for(int m = 0; m < 4; m++) {
    // make offsets for centering
    vs[0][0] = -(totalWidth / 2);
    vs[1][0] = +(totalWidth / 2);

    vs[0][1] = -(totalHeight / 2);
    vs[1][1] = +(totalHeight / 2);

    vs[0][2] = -(2 * (cellWidth + spacing)) + m * (cellWidth + spacing);
    vs[1][2] = vs[0][2] + cellHeight;
    

    for(int x = 0; x < floor(gridCols / 4.0); x++) {
      for(int y = 0; y < floor(gridRows / 4.0); y++) {
        strcpy(&output[offset], 
          va("{\n"
          "\"classname\" \"light\"\n"
          "\"origin\" \"%i %i %i\"\n"
          "\"light\" \"400\"\n"
          "\"radius\" \"128\"\n"
          "\"scale\" \"5\"\n"
          "\"target\" \"light_%i_%i_%i\"\n"
          "}\n", 
           (int)(vs[0][0] + (x * 2 + 1) * (cellWidth + thickness)),
           (int)(vs[0][1] + (y * 2 + 1) * (cellHeight + thickness)),
           (int)(vs[1][2] + 32),
           m, x, y));
        offset += strlen(&output[offset]);
        strcpy(&output[offset], 
          va("{\n"
    			"\"classname\" \"info_notnull\"\n"
    			"\"targetname\" \"light_%i_%i_%i\"\n"
    			"\"origin\" \"%i %i %i\"\n"
    			"}\n", m, x, y, 
           (int)(vs[0][0] + (x * 2 + 1) * (cellWidth + thickness)),
           (int)(vs[0][1] + (y * 2 + 1) * (cellHeight + thickness)),
           (int)(vs[0][2] + 32)));
        offset += strlen(&output[offset]);
      }
    }
  }

	// TODO: jumppads / teleporters / pickups

	return offset;
}


// TODO: wall is just a square platform
static char *SV_MakePlatform(vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4) {
	static char plat[4096];
	// TODO: make nice with edges like Edge of Oblivion on quake 3 space maps
	return plat;
}


static int SV_MakeShutesAndLadders() {
	vec3_t  vs[2];
	int cellWidth = 200;
	int cellHeight = 200;
	int m = 0;
	int spacing = 200;
	int totalWidth = 1000;
	int totalHeight = 1000;

	// TODO: ramps and wind tunnels like Edge of Oblivion with different shaped pyramids and stuff in space
	vs[0][0] = -(totalWidth / 2);
	vs[1][0] = +(totalWidth / 2);

	vs[0][1] = -(totalHeight / 2);
	vs[1][1] = +(totalHeight / 2);

	vs[0][2] = -(2 * (cellWidth + spacing)) + m * (cellWidth + spacing);
	vs[1][2] = vs[0][2] + cellHeight;

	SV_MakePlatform(vs[0], vs[1], vs[0], vs[1]);

	return 0;
}



void RGBToHSL( int R, int G, int B, int *H, int *S, int *L )
{
  float fR = R / 255.0;
  float fG = G / 255.0;
  float fB = B / 255.0;


  float fCMin = fR;
  if (fG < fCMin)
  {
      fCMin = fG;
  }
  if (fB < fCMin)
  {
      fCMin = fB;
  }

  float fCMax = fR;
  if (fG > fCMax)
  {
      fCMax = fG;
  }
  if (fB > fCMax)
  {
      fCMax = fB;
  }


  *L = (int)(50.0 * (fCMin + fCMax));

  if (fCMin == fCMax)
  {
      *S = 0;
      *H = 0;
      return;

  }
  else if (*L < 50)
  {
      *S = (int)(100.0 * (fCMax - fCMin) / (fCMax + fCMin));
  }
  else
  {
      *S = (int)(100.0 * (fCMax - fCMin) / (2.0 - fCMax - fCMin));
  }

  if (fCMax == fR)
  {
      *H = (int)(60.0 * (fG - fB) / (fCMax - fCMin));
  }
  if (fCMax == fG)
  {
      *H = (int)(60.0 * (fB - fR) / (fCMax - fCMin) + 120.0);
  }
  if (fCMax == fB)
  {
      *H = (int)(60.0 * (fR - fG) / (fCMax - fCMin) + 240.0);
  }
  if (*H < 0)
  {
      *H = (int)(*H + 360.0);
  }
}


qboolean isOverlapping(vec2_t l1, vec2_t r1, vec2_t l2, vec2_t r2)
{
    if (l1[0] == r1[0] || l1[1] == r1[1] || l2[0] == r2[0]
        || l2[1] == r2[1]) {
        // the line cannot have positive overlap
        return qfalse;
    }
 
    // If one rectangle is on left side of other
    if (l1[0] >= r2[0] || l2[0] >= r1[0])
        return qfalse;
 
    // If one rectangle is above other
    if (l1[1] >= r2[1] || l2[1] >= r1[1])
        return qfalse;
 
    return qtrue;
}


extern void CL_LoadJPG( const char *filename, unsigned char **pic, int *width, int *height );
static int SV_MakeMonacoF1() {
  int R, G, B, H, S, L;
  qboolean isRoad = qfalse;
  qboolean skipArea = qfalse;
  vec3_t  vs[2];
  int offset = 0;
  int maxWidth, maxHeight;
  int a1, a2, a3, w, h, x, y, bx, by;
  int pixel, xBase, yBase;
  int MAX_BRUSHES = 2000;

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	brushC = 0;
	output[0] = '\0';
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

	SV_SetStroke("sky1");
	strcpy(&output[offset], SV_MakeBox(vs[0], vs[1]));
	offset += strlen(&output[offset]);

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
  vec2_t *areaStack = Hunk_AllocateTempMemory(256 * 256 * sizeof(vec2_t));
  int areaCount = 0;
  // because we are trying to minimize the number of blocks, this algorithm is
  //   very greedy, which means it will take a lot of extra processing
  int areaHor = ceil(width / 256.0);
  int areaVer = ceil(height / 256.0);
  for(a1 = 0; a1 < areaHor; a1++) {
    for(a2 = 0; a2 < areaVer; a2++) {
      // TODO: make as few cubes as possible by decimating/
      //   checking if cube is inside a bigger cube that still fits along a spline
      //   probably going to need a stack for this, in order to form the splines
      memset(areaStack, 0, 256 * 256 * sizeof(vec2_t));
      areaCount = 0;
      // on the last loop, don't go past the edge of the image, only scan remaining area
      maxWidth = 256;
      if(a1 + 1 == areaHor) {
        maxWidth = width % 256;
      }
      maxHeight = 256;
      if(a2 + 1 == areaVer) {
        maxHeight = height % 256;
      }
      // start by fitting the largest possible cube of uninterrupted road
      // TODO: remove the lower limit restriction when it works
      for(w = 6; w > 0; w--) {
        for(h = 6; h > 0; h--) {
      //for(int w = 64; w > 1; w--) {
      //  for(int h = 64; h > 1; h--) {
          // cube can fit anywhere between the edges of the area so subtract the
          //   width/height from the traversable x and y position
          for(x = 0; x < maxWidth - w; x++) {
            for(y = 0; y < maxHeight - h; y++) {
              // since we can only get smaller skip the cubes that are already formed
              skipArea = qfalse;
              for(a3 = 0; a3 < areaCount; a3++) {
                // check if any of the corners intersect with another block
                if(isOverlapping((vec2_t){x, y}, (vec2_t){x + w, y + h}, 
                  areaStack[a3 * 2], areaStack[a3 * 2 + 1])) {
                  skipArea = qtrue;
                  break;
                }
              }
              if(skipArea) {
                isRoad = qfalse;
                continue;
              }
              
              
              /*
              pixel = (a2 * 256 * width * 4) // area rows
                + (a1 * 256 * 4) // area columns
                + (y * width * 4 + x * 4); // x and y inside area
              R = pic[pixel + 0];
              G = pic[pixel + 1];
              B = pic[pixel + 2];
              //RGBToHSL(R, G, B, &H, &S, &L);
              //printf("x: %i, y: %i, R: %i, G: %i, B: %i\n", a1 * 256 + x, a2 * 256 + y, R, G, B);
              printf("x: %i, y: %i, H: %i, S: %i, L: %i\n", a1 * 256 + x, a2 * 256 + y, H, S, L);
              */
              
              // finally scan the whole space for blue
              isRoad = qtrue;
              for(bx = x; bx < x + w; bx++) {
                for(by = y; by < y + h; by++) {
                  // this is in rgb format already, look for anything within 5% of blue
                  //   we do this by converting to HSL and comparing hue with a factor of 
                  //   S (saturation) and L (luminosity)
                  pixel = (a2 * 256 * width * 4) // area rows
                    + (a1 * 256 * 4) // area columns
                    + (by * width * 4 + bx * 4); // x and y inside area
                  R = pic[pixel + 0];
                  G = pic[pixel + 1];
                  B = pic[pixel + 2];
                  // RGB: 33, 179, 236
                  // HSL: 197, 86, 93
                  RGBToHSL(R, G, B, &H, &S, &L);
                  //printf("H: %i, S: %i, L: %i\n", H, S, L);
                  if(H > 180 && H < 220
                    && S > 50 && L > 40) {
                  //if(abs(H - 197) / 360 < 0.05 ) {
                  } else {
                    isRoad = qfalse;
                    break;
                  }
                } // by
                if(!isRoad) break; // exit early because it's not a road
              } // bx
              
              if(isRoad && areaCount < 128) {
                // we finally found a good block
                // make a road voxel at the position on blue pixels
                areaStack[areaCount*2][0] = x;
                areaStack[areaCount*2][1] = y;
                areaStack[areaCount*2+1][0] = x + w;
                areaStack[areaCount*2+1][1] = y + h;
                areaCount++;
                SV_SetStroke("cube1");
                xBase = a1 * 256 + x;
                yBase = a2 * 256 + y;
                //printf("fitted!: a1: %i, a2: %i, x: %i, y: %i\n", a1, a2, xBase, yBase);
                char *road = SV_MakeCube(
                  (vec3_t){xBase,   yBase,   4}, 
                  (vec3_t){xBase,   yBase+h, 4}, 
                  (vec3_t){xBase+w, yBase+h, 4}, 
                  (vec3_t){xBase+w, yBase,   4},

                  (vec3_t){xBase,   yBase,   0}, 
                  (vec3_t){xBase,   yBase+h, 0}, 
                  (vec3_t){xBase+w, yBase+h, 0}, 
                  (vec3_t){xBase+w, yBase,   0}
                );
                strcpy(&output[offset], road);
                offset += strlen(&output[offset]);
              }
              
              if(brushC >= MAX_BRUSHES) break;
            } // y
            if(brushC >= MAX_BRUSHES) break;
          } // x


          if(brushC >= MAX_BRUSHES) break;
        } // h
        if(brushC >= MAX_BRUSHES) break;
      } // w

      
      if(brushC >= MAX_BRUSHES) break;
      // TODO: make some buildings and side roads and barriers and z-height
    } // a2
    if(brushC >= MAX_BRUSHES) break;
  } // a1

	strcpy(&output[offset], "}\n");
	offset += 2;
  
  Hunk_FreeTempMemory( areaStack );

  strcpy(&output[offset], 
    va("{\n"
    "\"classname\" \"info_player_start\"\n"
    "\"origin\" \"%i %i %i\"\n"
    "\"angle\" \"180\"\n"
    "}\n", -1050, 550, 200));
  offset += strlen(&output[offset]);

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
	
  return offset;
}


extern int Q3MAP2Main( int argc, char **argv );

int SV_MakeMap( char *memoryMap ) {
	int length = 0;
  fileHandle_t mapfile;

  if((length = FS_FOpenFileRead( va("maps/%s.map", memoryMap), &mapfile, qtrue )) > -1) {
    FS_FCloseFile(mapfile);
    return length;
  }

	if(Q_stricmp(memoryMap, "megamaze") == 0) {
		length = SV_MakeMaze();
	} else if (Q_stricmp(memoryMap, "megacube") == 0) {
		length = SV_MakeHypercube();
	} else if (Q_stricmp(memoryMap, "megashutes") == 0) {
		length = SV_MakeShutesAndLadders();
	} else if (Q_stricmp(memoryMap, "megaf1") == 0) {
		length = SV_MakeMonacoF1();
	} else {
    return 0;
  }
  
  // TODO: overwrite if make-time is greater than 1 min?

	mapfile = FS_FOpenFileWrite( va("maps/%s.map", memoryMap) );
	FS_Write( output, length, mapfile );    // overwritten later
	FS_FCloseFile( mapfile );

  //gamedir = Cvar_VariableString( "fs_game" );
	//basegame = Cvar_VariableString( "fs_basegame" );
  Cvar_Set( "buildingMap", memoryMap );
  char *mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), 
    Cvar_VariableString("fs_game"), va("maps/%s.map", memoryMap) );
  char *compileMap[] = {
    "q3map2",
    "-v",
    "-fs_basepath",
    (char *)Cvar_VariableString("fs_basepath"),
    "-game",
    "quake3",
    "-meta",
    "-patchmeta",
    mapPath
  };
	Q3MAP2Main(ARRAY_LEN(compileMap), compileMap);

/*
  char *bspPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), 
    Cvar_VariableString("fs_game"), va("maps/%s.bsp", memoryMap) );
  char *compileLight[] = {
    "q3map2",
    "-light",
    "-fs_basepath",
    (char *)Cvar_VariableString("fs_basepath"),
    "-game",
    "quake3",
    "-faster",
    "-cheap",
    "-bounce",
    "1",
    bspPath
  };
  Q3MAP2Main(ARRAY_LEN(compileLight), compileLight);
*/

  Cvar_Set( "buildingMap", "" );
	return length;
}

#endif
