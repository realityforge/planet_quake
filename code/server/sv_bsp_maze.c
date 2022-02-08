#include "server.h"

#ifdef USE_MEMORY_MAPS

int SV_MakeMaze( void ) {
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

#endif
