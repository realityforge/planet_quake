#include "server.h"

#ifdef USE_MEMORY_MAPS

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
	qboolean isPatch = qfalse;

	memset(output, 0, sizeof(output));
	strcpy(&output[offset], "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n"
		"\"_keepLights\" \"1\"\n"
		"\"_ambient\" \"10\"\n"
		"\"gridsize\" \"512.0 512.0 512.0\"\n"
	);
	// TODO: filter out the keys from above below where it's copied to the output
	offset += strlen(&output[offset]);

  length = FS_ReadFile(memoryMap, (void **)&buffer);

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
				isPatch = qfalse;
				//Com_Printf("beginning brush: %i\n", count);
				if(isWorldspawn) {
					isInside = qfalse;
				}
			}
			if(depth == 3) {
				patchStartPos = count;
				tokenStartPos = 0;
				if(isPatch) {
					// skip next 2 lines is the texture and alignment
					if(strstr(&buffer[count], "\n") - &buffer[count] <= 2) {
						count += strstr(&buffer[count], "\n") - &buffer[count];
						count++;
					}
					if(strstr(&buffer[count], "\n") - &buffer[count] <= 2) {
						count += strstr(&buffer[count], "\n") - &buffer[count];
						count++;
					}
					Com_Printf("patch: %.*s\n", (int)(strstr(&buffer[count], "\n") - &buffer[count]), &buffer[count]);
					count += strstr(&buffer[count], "\n") - &buffer[count];
					count++;
					count += strstr(&buffer[count], "\n") - &buffer[count];
				}
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
				Q_strncpyz(&output[offset], va("// brush %i\n", brushC), 13);
				offset += strlen(&output[offset]);
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
				if(isWorldspawn) {
					Q_strncpyz(&output[offset+1], &buffer[tokenStartPos], count - tokenStartPos + 1);
					output[offset] = '"';
					output[offset+(count - tokenStartPos)+1] = '"';
					output[offset+(count - tokenStartPos)+2] = ' ';
					offset += strlen(&output[offset]);
				}
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
				if(isWorldspawn) {
					Q_strncpyz(&output[offset+1], &buffer[tokenStartPos], count - tokenStartPos + 1);
					output[offset] = '"';
					output[offset+(count - tokenStartPos)+1] = '"';
					output[offset+(count - tokenStartPos)+2] = ' ';
					offset += strlen(&output[offset]);
				}
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
				isValue = qfalse;
			} else if (!isKey && !isValue && buffer[count] == '\n') {
				// ignore whitespace in between
				if(output[offset] != '\n') {
					output[offset] = '\n';
					offset++;
				}
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
			} else if ((buffer[count] >= '0' && buffer[count] <= '9')
				|| (isNumeric && (buffer[count] == '-' || buffer[count] == '.'))) {
				if(!isNumeric)
					tokenStartPos = count;
				isNumeric = qtrue;
			} else if (Q_stricmpn(&buffer[count], "patchdef2", 9) == 0) {
				count += 9;
				isPatch = qtrue;
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


char *SV_MakeWall( int p1[3], int p2[3] ) {
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


char *SV_MakeBox( vec3_t min, vec3_t max ) {
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
    int h = CM_InlineModel( 0 );
#endif
		CM_ModelBounds( h, vs[0], vs[1] );
	}

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
char *SV_MakeCube(
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

#endif
