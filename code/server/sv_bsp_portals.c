#include "server.h"

#ifdef USE_MEMORY_MAPS

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

char *SV_MakePortal( float radius, vec3_t min, vec3_t max, int minSegment, int maxSegment, int sides ) {
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

#endif
