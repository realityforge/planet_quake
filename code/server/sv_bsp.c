
#ifdef USE_MEMORY_MAPS

#include "server.h"
#include "../qcommon/vm_local.h"
#include "../qcommon/cm_public.h"
#include "../game/bg_public.h"

char *FS_RealPath(const char *localPath);

static char stroke[MAX_QPATH] = "";

static char output[2 * 1024 * 1024] = ""; // 2MB TODO: make alloc and optional
static int brushC = 0;

static qboolean isOverlapping(vec2_t l1, vec2_t r1, vec2_t l2, vec2_t r2);

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
			// top 
			{min[0] + x1, min[1] + y1, min[2]},
			{min[0] + x2, min[1] + y2, min[2]},
			{min[0] + x3, min[1] + y3, min[2]},
			{min[0] + x4, min[1] + y4, min[2]},

			{min[0] + x1, min[1] + y1, min[2] + thickness}, 
			{min[0] + x2, min[1] + y2, min[2] + thickness}, 
			{min[0] + x3, min[1] + y3, min[2] + thickness}, 
			{min[0] + x4, min[1] + y4, min[2] + thickness},

      // west
			{min[0] + thickness, min[1] + x4, min[2] + y4},
			{min[0],             min[1] + x4, min[2] + y4},
			{min[0],             min[1] + x3, min[2] + y3},
			{min[0] + thickness, min[1] + x3, min[2] + y3},

			{min[0] + thickness, min[1] + x1, min[2] + y1}, 
			{min[0],             min[1] + x1, min[2] + y1}, 
			{min[0],             min[1] + x2, min[2] + y2}, 
			{min[0] + thickness, min[1] + x2, min[2] + y2},

      // south
			{min[0] + x1, min[1] + thickness, min[2] + y1}, 
			{min[0] + x2, min[1] + thickness, min[2] + y2}, 
			{min[0] + x3, min[1] + thickness, min[2] + y3}, 
			{min[0] + x4, min[1] + thickness, min[2] + y4},

			{min[0] + x1, min[1], min[2] + y1}, 
			{min[0] + x2, min[1], min[2] + y2}, 
			{min[0] + x3, min[1], min[2] + y3}, 
			{min[0] + x4, min[1], min[2] + y4},
			
			// bottom 
			{min[0] + x1, min[1] + y1, max[2] - thickness},
			{min[0] + x2, min[1] + y2, max[2] - thickness},
			{min[0] + x3, min[1] + y3, max[2] - thickness},
			{min[0] + x4, min[1] + y4, max[2] - thickness},

			{min[0] + x1, min[1] + y1, max[2]}, 
			{min[0] + x2, min[1] + y2, max[2]}, 
			{min[0] + x3, min[1] + y3, max[2]}, 
			{min[0] + x4, min[1] + y4, max[2]},

      // east
			{max[0] - thickness, min[1] + x1, min[2] + y1},
			{max[0] - thickness, min[1] + x2, min[2] + y2},
			{max[0] - thickness, min[1] + x3, min[2] + y3},
			{max[0] - thickness, min[1] + x4, min[2] + y4},

			{max[0], min[1] + x1, min[2] + y1}, 
			{max[0], min[1] + x2, min[2] + y2}, 
			{max[0], min[1] + x3, min[2] + y3}, 
			{max[0], min[1] + x4, min[2] + y4},

      // north
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
	int *roadStack = Hunk_AllocateTempMemory(gridRows * gridCols * 4 * sizeof(int));
	int minX;
	int minY;
	int maxX;
	int maxY;

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	brushC = 0;
	memset(output, 0, sizeof(output));
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

		// build inner maze using a roadStack to list every division, skips divisions that would be too small
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
				minX = roadStack[stackI*4+0];
				minY = roadStack[stackI*4+1];
				maxX = roadStack[stackI*4+2];
				maxY = roadStack[stackI*4+3];
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
				roadStack[stackI*4+0] = minX;
				roadStack[stackI*4+1] = minY;
				roadStack[stackI*4+2] = wallX;
				roadStack[stackI*4+3] = wallY;
				stackI++;
			}
			if(wallX - minX > 1
				&& maxY - wallY > 1) {
				//Com_Printf("Adding bottom, left: %i x %i <> %i x %i\n", minX, wallY, wallX, maxY);
				roadStack[stackI*4+0] = minX;
				roadStack[stackI*4+1] = wallY + 1;
				roadStack[stackI*4+2] = wallX;
				roadStack[stackI*4+3] = maxY;
				stackI++;
			}
			if(maxX - wallX > 1
				&& wallY - minY > 1) {
				//Com_Printf("Adding top, right: %i x %i <> %i x %i\n", wallX, minY, maxX, wallY);
				roadStack[stackI*4+0] = wallX + 1;
				roadStack[stackI*4+1] = minY;
				roadStack[stackI*4+2] = maxX;
				roadStack[stackI*4+3] = wallY;
				stackI++;
			}
			if(maxX - wallX > 1
				&& maxY - wallY > 1) {
				//Com_Printf("Adding bottom, right: %i x %i <> %i x %i\n", wallX, wallY, maxX, maxY);
				roadStack[stackI*4+0] = wallX + 1;
				roadStack[stackI*4+1] = wallY + 1;
				roadStack[stackI*4+2] = maxX;
				roadStack[stackI*4+3] = maxY;
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
	
	Hunk_FreeTempMemory( roadStack );
	
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
//static char *SV_MakePlatform(vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4) {
//	static char plat[4096];
	// TODO: make nice with edges like Edge of Oblivion on quake 3 space maps
//	return plat;
//}


/*
static void SV_FlipHorizontal(vec3_t *flip) {
  int temp0 = flip[0][0];
  int temp1 = flip[1][0];
  flip[0][0] = flip[3][0];
  flip[1][0] = flip[2][0];
  flip[2][0] = temp1;
  flip[3][0] = temp0;
  temp0 = flip[0][2];
  temp1 = flip[1][2];
  flip[0][2] = flip[3][2];
  flip[1][2] = flip[2][2];
  flip[2][2] = temp1;
  flip[3][2] = temp0;
}


static void SV_FlipVertical(vec3_t *flip) {
  int temp0 = flip[0][1];
  int temp3 = flip[3][1];
  flip[0][1] = flip[1][1];
  flip[3][1] = flip[2][1];
  flip[1][1] = temp0;
  flip[2][1] = temp3;
  temp0 = flip[0][2];
  temp3 = flip[3][2];
  flip[0][2] = flip[1][2];
  flip[3][2] = flip[2][2];
  flip[1][2] = temp0;
  flip[2][2] = temp3;
}
*/


static int SV_MakeAtlantis() {
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

	brushC = 0;
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
static int SV_MakeChutesAndLadders() {
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

	brushC = 0;
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


double deg2Rad(const double deg)
{
	return (deg * (M_PI / 180.0));
}


// source: https://github.com/gfiumara/CIEDE2000/blob/master/CIEDE2000.cpp
float CIEDE2000(double l1, double a1, double b1, double l2, double a2, double b2)
{
	/* 
	 * "For these and all other numerical/graphical 􏰀delta E00 values
	 * reported in this article, we set the parametric weighting factors
	 * to unity(i.e., k_L = k_C = k_H = 1.0)." (Page 27).
	 */
	const double k_L = 1.0, k_C = 1.0, k_H = 1.0;
	const double deg360InRad = deg2Rad(360.0);
	const double deg180InRad = deg2Rad(180.0);
	const double pow25To7 = 6103515625.0; /* pow(25, 7) */
	
	/*
	 * Step 1 
	 */
	/* Equation 2 */
	double C1 = sqrt((a1 * a1) + (b1 * b1));
	double C2 = sqrt((a2 * a2) + (b2 * b2));
	/* Equation 3 */
	double barC = (C1 + C2) / 2.0;
	/* Equation 4 */
	double G = 0.5 * (1 - sqrt(pow(barC, 7) / (pow(barC, 7) + pow25To7)));
	/* Equation 5 */
	double a1Prime = (1.0 + G) * a1;
	double a2Prime = (1.0 + G) * a2;
	/* Equation 6 */
	double CPrime1 = sqrt((a1Prime * a1Prime) + (b1 * b1));
	double CPrime2 = sqrt((a2Prime * a2Prime) + (b2 * b2));
	/* Equation 7 */
	double hPrime1;
	if (b1 == 0 && a1Prime == 0)
		hPrime1 = 0.0;
	else {
		hPrime1 = atan2(b1, a1Prime);
		/* 
		 * This must be converted to a hue angle in degrees between 0 
		 * and 360 by addition of 2􏰏 to negative hue angles.
		 */
		if (hPrime1 < 0)
			hPrime1 += deg360InRad;
	}
	double hPrime2;
	if (b2 == 0 && a2Prime == 0)
		hPrime2 = 0.0;
	else {
		hPrime2 = atan2(b2, a2Prime);
		/* 
		 * This must be converted to a hue angle in degrees between 0 
		 * and 360 by addition of 2􏰏 to negative hue angles.
		 */
		if (hPrime2 < 0)
			hPrime2 += deg360InRad;
	}
	
	/*
	 * Step 2
	 */
	/* Equation 8 */
	double deltaLPrime = l2 - l1;
	/* Equation 9 */
	double deltaCPrime = CPrime2 - CPrime1;
	/* Equation 10 */
	double deltahPrime;
	double CPrimeProduct = CPrime1 * CPrime2;
	if (CPrimeProduct == 0)
		deltahPrime = 0;
	else {
		/* Avoid the fabs() call */
		deltahPrime = hPrime2 - hPrime1;
		if (deltahPrime < -deg180InRad)
			deltahPrime += deg360InRad;
		else if (deltahPrime > deg180InRad)
			deltahPrime -= deg360InRad;
	}
	/* Equation 11 */
	double deltaHPrime = 2.0 * sqrt(CPrimeProduct) *
    sin(deltahPrime / 2.0);
	
	/*
	 * Step 3
	 */
	/* Equation 12 */
	double barLPrime = (l1 + l2) / 2.0;
	/* Equation 13 */
	double barCPrime = (CPrime1 + CPrime2) / 2.0;
	/* Equation 14 */
	double barhPrime, hPrimeSum = hPrime1 + hPrime2;
	if (CPrime1 * CPrime2 == 0) {
		barhPrime = hPrimeSum;
	} else {
		if (fabs(hPrime1 - hPrime2) <= deg180InRad)
			barhPrime = hPrimeSum / 2.0;
		else {
			if (hPrimeSum < deg360InRad)
				barhPrime = (hPrimeSum + deg360InRad) / 2.0;
			else
				barhPrime = (hPrimeSum - deg360InRad) / 2.0;
		}
	}
	/* Equation 15 */
	double T = 1.0 - (0.17 * cos(barhPrime - deg2Rad(30.0))) +
    (0.24 * cos(2.0 * barhPrime)) +
    (0.32 * cos((3.0 * barhPrime) + deg2Rad(6.0))) - 
    (0.20 * cos((4.0 * barhPrime) - deg2Rad(63.0)));
	/* Equation 16 */
	double deltaTheta = deg2Rad(30.0) *
    exp(-pow((barhPrime - deg2Rad(275.0)) / deg2Rad(25.0), 2.0));
	/* Equation 17 */
	double R_C = 2.0 * sqrt(pow(barCPrime, 7.0) /
    (pow(barCPrime, 7.0) + pow25To7));
	/* Equation 18 */
	double S_L = 1 + ((0.015 * pow(barLPrime - 50.0, 2.0)) /
    sqrt(20 + pow(barLPrime - 50.0, 2.0)));
	/* Equation 19 */
	double S_C = 1 + (0.045 * barCPrime);
	/* Equation 20 */
	double S_H = 1 + (0.015 * barCPrime * T);
	/* Equation 21 */
	double R_T = (-sin(2.0 * deltaTheta)) * R_C;
	
	/* Equation 22 */
  double deltaE = sqrt(
    pow(deltaLPrime / (k_L * S_L), 2.0) +
    pow(deltaCPrime / (k_C * S_C), 2.0) +
    pow(deltaHPrime / (k_H * S_H), 2.0) + 
    (R_T * (deltaCPrime / (k_C * S_C)) * (deltaHPrime / (k_H * S_H))));

	return (deltaE);
}


double F(double input) // function f(...), which is used for defining L, a and b changes within [4/29,1]
{
  if (input > 0.008856)
    return (pow(input, 0.333333333)); // maximum 1
  else
    return ((841/108)*input + 4/29);  //841/108 = 29*29/36*16
}


void XYZtoLab(double X, double Y, double Z, double *L, double *a, double *b)
{
  // TODO: make sure these are correct
  const double Xo = 244.66128; // reference white
  const double Yo = 255.0;
  const double Zo = 277.63227;
  *L = 116 * F(Y / Yo) - 16; // maximum L = 100
  *a = 500 * (F(X / Xo) - F(Y / Yo)); // maximum 
  *b = 200 * (F(Y / Yo) - F(Z / Zo));
}


// source http://www.easyrgb.com/en/math.php
void RGBtoXYZ(double R, double G, double B, double *X, double *Y, double *Z) {
  // Assume RGB has the type invariance satisfied, i.e., channels \in [0,255]
  float var_R = R / 255.0;
  float var_G = G / 255.0;
  float var_B = B / 255.0;

  var_R = (var_R > 0.04045) ? pow((var_R + 0.055) / 1.055, 2.4)
                            : var_R / 12.92;
  var_G = (var_G > 0.04045) ? pow((var_G + 0.055) / 1.055, 2.4)
                            : var_G / 12.92;
  var_B = (var_B > 0.04045) ? pow((var_B + 0.055) / 1.055, 2.4)
                            : var_B / 12.92;

  var_R *= 100;
  var_G *= 100;
  var_B *= 100;

  *X = var_R * 0.4124 + var_G * 0.3576 +
       var_B * 0.1805;
  *Y = var_R * 0.2126 + var_G * 0.7152 +
       var_B * 0.0722;
  *Z = var_R * 0.0193 + var_G * 0.1192 +
       var_B * 0.9505;
}


void rgb2lab(int R, int G, int B, double *L, double *a, double *b){
  double X, Y, Z;
  RGBtoXYZ(R, G, B, &X, &Y, &Z);
  XYZtoLab(X, Y, Z, L, a, b);
}



static qboolean isOverlapping(vec2_t l1, vec2_t r1, vec2_t l2, vec2_t r2)
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


double deltaE(double l1, double a1, double b1, double l2, double a2, double b2){
  double deltaL = l1 - l2;
  double deltaA = a1 - a2;
  double deltaB = b1 - b2;
  double c1 = sqrt(a1 * a1 + b1 * b1);
  double c2 = sqrt(a2 * a2 + b2 * b2);
  double deltaC = c1 - c2;
  double deltaH = deltaA * deltaA + deltaB * deltaB - deltaC * deltaC;
  deltaH = deltaH < 0 ? 0 : sqrt(deltaH);
  double sc = 1.0 + 0.045 * c1;
  double sh = 1.0 + 0.015 * c1;
  double deltaLKlsl = deltaL / (1.0);
  double deltaCkcsc = deltaC / (sc);
  double deltaHkhsh = deltaH / (sh);
  double i = deltaLKlsl * deltaLKlsl + deltaCkcsc * deltaCkcsc + deltaHkhsh * deltaHkhsh;
  return i < 0 ? 0 : sqrt(i);
}

 
void bubbleSort(float *arr, int n, vec2_t *arr2)
{
  int i, j;
  for (i = 0; i < n-1; i++) {
    for (j = 0; j < n-i-1; j++) {
      if (arr[j] > arr[j+1]) {
        float temp = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = temp;
        vec2_t temp2;
        temp2[0] = arr2[j][0];
        temp2[1] = arr2[j][1];
        arr2[j][0] = arr2[j+1][0];
        arr2[j][1] = arr2[j+1][1];
        arr2[j+1][0] = temp2[0];
        arr2[j+1][1] = temp2[1];
      }
    }
  }
}


void SV_GetAreaBlocks(
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


void SV_GetAreaPaths(
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



void SV_GetAreaColors(int AREA_BLOCK, 
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



extern void CL_LoadJPG( const char *filename, unsigned char **pic, int *width, int *height );

static int SV_MakeMonacoF1() {
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

	brushC = 0;
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


// slice up a BSP and resave it without recompiling.
void SV_SpliceBSP(const char *memoryMap, const char *altName) {
	vec3_t mins, maxs;
  fileHandle_t mapfile;
	char cmd[MAX_CMD_LINE];
	char buf[MAX_TOKEN_CHARS];
	const char *buffer = NULL;
  int offset = 0;
	int length = 0;
	int depth = 0;
	int count = 0;
	int countEntities = 0;
	int countPoints = 0;
	int tokenStartPos = 0;
	int brushStartPos = 0;
	int patchStartPos = 0;
	int entityStartPos = 0;
	qboolean isNumeric = qfalse;
	qboolean isWorldspawn = qfalse;
	qboolean isKey = qfalse;
	qboolean isValue = qfalse;
	qboolean isOrigin = qfalse;
	qboolean isLocation = qfalse;
	qboolean isClassname = qfalse;
	qboolean ignoreLine = qfalse;
	qboolean isInside = qfalse; // trying to prove at least 1 point is inside the bounds
	qboolean vertIsInside = qfalse;
	//qboolean isPatch = qfalse;

	brushC = 0;
	memset(output, 0, sizeof(output));
	strcpy(&output[offset], "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n");
	offset += strlen(&output[offset]);

  length = FS_ReadFile(va("maps/%s.map", memoryMap), (void **)&buffer);

	Q_strncpyz(cmd, Cmd_ArgsFrom(0), sizeof(cmd));

	// parse mins and maxs from option
	Cmd_TokenizeString(sv_bspSplice->string);
	mins[0] = atof(Cmd_Argv(0));
	mins[1] = atof(Cmd_Argv(1));
	mins[2] = atof(Cmd_Argv(2));
	maxs[0] = atof(Cmd_Argv(3));
	maxs[1] = atof(Cmd_Argv(4));
	maxs[2] = atof(Cmd_Argv(5));
	Com_Printf("Splicing BSP from %f %f %f to %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);
	Cmd_Clear();

	// read every brush and entity origin
	while(1) {
		if(count >= length) break;
		// ignore comments
		if(buffer[count] == '/' && buffer[count+1] == '/') {
			ignoreLine = qtrue;
			count++;
			continue;
		}
		if(buffer[count] == '\n') ignoreLine = qfalse;
		if(ignoreLine) {
			count++;
			continue;
		}

		// first curls are entities
		//   second curls are brushes
		if(buffer[count] == '{') {
			depth++;
			if(depth == 1) {
				isWorldspawn = qfalse;
				isLocation = qfalse;
				entityStartPos = count;
				isInside = qfalse;
			}
			if(depth == 2) {
				brushStartPos = count;
				tokenStartPos = 0;
				//Com_Printf("beginning brush: %i\n", count);
				if(isWorldspawn) {
					isInside = qfalse;
				}
			}
			if(depth == 3) {
				patchStartPos = count;
				tokenStartPos = 0;
			}
		}
		if(buffer[count] == '}') {
			if(depth == 1 && isWorldspawn) {
				strcpy(&output[offset], "\n}\n");
				offset += 3;
			}
			if(depth == 1 && !isWorldspawn && isInside) {
				// copy entire entity
				Q_strncpyz(&output[offset], &buffer[entityStartPos], count - entityStartPos + 1);
				offset += strlen(&output[offset]);
				strcpy(&output[offset], "\n}\n");
				offset += 3;
				countEntities++;
			}
			//if(depth == 2 && isWorldspawn)
			//	Com_Printf("end brush: %i\n", offset);
			if(depth == 2 && isWorldspawn && isInside) {
				// copy brushes out of worldspawn
				Q_strncpyz(&output[offset], &buffer[brushStartPos], count - brushStartPos + 1);
				offset += strlen(&output[offset]);
				strcpy(&output[offset], "\n}\n");
				offset += 3;
				brushC++;
			}
			//if(depth == 3) patchStartPos = count;
			depth--;
		}
		if(depth == 1) {
			if(!isKey && !isValue && buffer[count] == '"') {
				if(isClassname || isOrigin) isValue = qtrue;
				else isKey = qtrue;
				tokenStartPos = count+1;
			} else if (isKey && buffer[count] == '"') {
				if(!Q_stricmpn(&buffer[tokenStartPos], "classname", 9)) {
					isClassname = qtrue;
				} else if(!Q_stricmpn(&buffer[tokenStartPos], "origin", 6)) {
					isOrigin = qtrue;
				//} else if(!Q_stricmpn(&buffer[tokenStartPos], "location", )) {
				//	isLocation = qtrue;
				} else {
					//Q_strncpyz(buf, buffer+tokenStartPos, sizeof(buf));
					//buf[count - tokenStartPos] = '\0';
					//Com_Printf("WARNING: unknown key \"%s\"\n", buf);
				}
				isKey = qfalse;
			} else if (isValue && buffer[count] == '"') {
				isValue = qfalse;
				if(isClassname) {
					if(!Q_stricmpn(buffer+tokenStartPos, "worldspawn", 10)) {
						isWorldspawn = qtrue;
					} else if(!Q_stricmpn(buffer+tokenStartPos, "target_location", 14)) {
						isLocation = qtrue;
					} else {
						//Com_Printf("");
					}
					isClassname = qfalse;
				} else if (isOrigin) {
					vec3_t origin;
					// check if model/entity origin is close enough to bounds
					Q_strncpyz(buf, buffer+tokenStartPos, sizeof(buf));
					buf[count - tokenStartPos] = '\0';
					Cmd_TokenizeString(buf);
					origin[0] = atof(Cmd_Argv(0));
					origin[1] = atof(Cmd_Argv(1));
					origin[2] = atof(Cmd_Argv(2));
					if(origin[0] >= mins[0] && origin[0] <= maxs[0] &&
					   origin[1] >= mins[1] && origin[1] <= maxs[1] &&
					   origin[2] >= mins[2] && origin[2] <= maxs[2]) {
						isInside = qtrue;
					}
					Cmd_Clear();
					isOrigin = qfalse;
				}
			} else if (!isKey && !isValue && (buffer[count] == '\t' || buffer[count] == ' ')) {
				// ignore whitespace in between
			} else {

			}
		}
		// tokenize first 9/15 numbers but only use first 3 of each 5
		else if (!isInside && (depth == 2 || depth == 3)) {
			if(buffer[count] == '\n') {
				countPoints = 0;
				isNumeric = qfalse;
				vertIsInside = qfalse;
			} else if (isNumeric 
				&& (buffer[count] == ' ' || buffer[count] == '\t'
				|| buffer[count] == '(' || buffer[count] == ')')) {
				// if it is outside mins and maxs, ignore it
				int pointsPerLine = (depth == 2 ? 3 : 5);
				if(countPoints < (depth == 2 ? 9 : 15) && countPoints % pointsPerLine < 3) {
					Q_strncpyz(buf, buffer+tokenStartPos, sizeof(buf));
					buf[count - tokenStartPos] = '\0';
					float point = atof(buf);
					if(buffer[tokenStartPos-1] == '-')
						point = -point;
					//if(count - tokenStartPos > 1)
					//	Com_Printf("checking point: %f >= %f <= %f\n", 
					//		point, mins[countPoints % pointsPerLine], maxs[countPoints % pointsPerLine]);
					// skip until the end of the entity
					if(countPoints % pointsPerLine == 0) {
						vertIsInside = qtrue;
					}
					if(point >= mins[countPoints % pointsPerLine] 
						&& point <= maxs[countPoints % pointsPerLine]) {
					} else {
						// 1 of 3 not inside means vert is outside
						vertIsInside = qfalse;
					}
				}
				tokenStartPos = 0;
				countPoints++;
				// check at least 3 vertexes
				if(vertIsInside && countPoints % pointsPerLine == 0) {
					isInside = qtrue;
				}
				isNumeric = qfalse;
			} else if (buffer[count] >= '0' && buffer[count] <= '9') {
				if(!isNumeric)
					tokenStartPos = count;
				isNumeric = qtrue;
			} else {
				tokenStartPos = 0;
				isNumeric = qfalse;
			}
		}
		count++;
	}

	// always writes to home directory
	mapfile = FS_FOpenFileWrite(altName);
	FS_Write( output, offset, mapfile );    // overwritten later
	FS_FCloseFile( mapfile );
	// reset cmd string, TODO: not sure why this should matter for map name from SV_Map_f
	Cmd_TokenizeString(cmd);
}


extern int Q3MAP2Main( int argc, char **argv );

int SV_MakeMap( char *map ) {
	char memoryMap[MAX_QPATH];
	char *mapPath;
  fileHandle_t mapfile;
	int length = 0;
	Q_strncpyz( memoryMap, map, sizeof(memoryMap) );

	// early exit unless we force rebuilding
	if(!sv_bspRebuild->integer && FS_RealPath( va("maps/%s.bsp", memoryMap) )) {
		return 1;
	}

	if(Q_stricmp(memoryMap, "megamaze") == 0) {
		length = SV_MakeMaze();
	} else if (Q_stricmp(memoryMap, "megacube") == 0) {
		length = SV_MakeHypercube();
	} else if (Q_stricmp(memoryMap, "megachutes") == 0) {
		length = SV_MakeChutesAndLadders();
	} else if (Q_stricmp(memoryMap, "megaf1") == 0) {
		length = SV_MakeMonacoF1();
	} else if (Q_stricmp(memoryMap, "megalantis") == 0) {
		length = SV_MakeAtlantis();
	} else if (Q_stricmp(memoryMap, "test") == 0) {
	}
  
	if(length) {
		// TODO: overwrite if make-time is greater than 1 min?
		// always writes to home directory
		mapfile = FS_FOpenFileWrite( va("maps/%s.map", memoryMap) );
		FS_Write( output, length, mapfile );    // overwritten later
		FS_FCloseFile( mapfile );
		mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), 
    Cvar_VariableString("fs_game"), va("maps/%s.map", memoryMap) );
	}

	// TODO: use virtual_fs to load .map from pk3, more of a q3map2 problem
	// TODO: detect prior formats and decompile it to .map
	// TODO: compare file time with BSP
	// TODO: check if lightmap exists and compare with BSP time
	// TODO: copy to home directory?
	// TODO: need an API to get exact paths?

  //gamedir = Cvar_VariableString( "fs_game" );
	//basegame = Cvar_VariableString( "fs_basegame" );
	// if there is no map file for it, try to make one!
	mapPath = FS_RealPath( va("maps/%s.bsp", memoryMap) );
	if(sv_bspMap->integer && mapPath
		// don't do it if we already extracted
		// && FS_RealPath( va("maps/%s_converted.map", memoryMap) )
	) {
		// someone extracted the bsp file intentionally?
		Cvar_Set( "buildingMap", memoryMap );
		char *compileMap[] = {
			"q3map2",
			"-v",
			"-fs_basepath",
			(char *)Cvar_VariableString("fs_basepath"),
			"-game",
			"quake3",
			"-convert",
			"-keeplights",
			"-format",
			"map",
			// "-readmap", // TODO: use for normalizing mbspc bsp2map conversions
			mapPath
		};
		Q3MAP2Main(ARRAY_LEN(compileMap), compileMap);
		Cvar_Set( "buildingMap", "" );
	}
	
	if(sv_bspSplice->string[0] != '\0') {
		SV_SpliceBSP(memoryMap, va("maps/%s_spliced.map", memoryMap));
	}
	
	if (!mapPath) {
		// no bsp file exists, try to make one, check for .map file
		mapPath = FS_RealPath( va("maps/%s.map", memoryMap) );
		if(sv_bspRebuild->integer && mapPath) {
			Cvar_Set( "buildingMap", memoryMap );
			char *compileMap[] = {
				"q3map2",
				"-v",
				"-fs_basepath",
				(char *)Cvar_VariableString("fs_basepath"),
				"-game",
				"quake3",
				"-meta",
		    "-patchmeta",
				"-keeplights",
				mapPath
			};
			Q3MAP2Main(ARRAY_LEN(compileMap), compileMap);
		}
	}

	if(sv_bspLight->integer
		&& (length = FS_FOpenFileRead( va("maps/%s/lm_0000.tga", memoryMap), NULL, qtrue )) == -1) {
		// then we can decide not to update LM?
		char *bspPath = FS_RealPath( va("maps/%s.bsp", memoryMap) );
		char *compileLight[] = {
			"q3map2",
			"-light",
			"-external",
			"-fs_basepath",
			(char *)Cvar_VariableString("fs_basepath"),
			"-game",
			"quake3",
			"-faster",
			"-cheap",
			//"-patchshadows",
			//"-gridsize",
			//"512.0 512.0 512.0",
			"-bounce",
			// TODO: one room at a time, and update in between bounces
			"2", 
			bspPath
		};
		Q3MAP2Main(ARRAY_LEN(compileLight), compileLight);
	}

	// TODO: generate AAS file for bots, missing, or updated maps
	if(sv_bspAAS->integer
		&& (length = FS_FOpenFileRead( va("maps/%s.aas", memoryMap), NULL, qtrue )) == -1) {
		
	}

  Cvar_Set( "buildingMap", "" );
	return length;
}

#endif
