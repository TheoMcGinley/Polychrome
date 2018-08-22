#include "polychrome.h"

//optionally give it a grid?
static double calc_blocking_score(int grid[GRIDSIZE][GRIDSIZE], int gridx, int gridy, int windowwidth, int windowheight) {
	double score = 0;
	//for each cell
	for (int i=0; i<GRIDSIZE; i++) {
		for (int j=0; j<GRIDSIZE; j++) {
			//2 checks:
			// 	1) i is between (inclusive) gridx and (gridx+windowwidth)
			// 	2) j is between (inclusive gridy and (gridy+windowheight)
			//TODO check edge conditions here
			if ( i >= gridx && i < (gridx+windowwidth) && 
			  	 j >= gridy && j < (gridy+windowheight)) {
					score += (grid[i][j])*1000;
			}
		}
	}
	return score;
}

//assuming GRIDSIZE=16
//assert(15, 15, 1, 1) = 0
//assert(16, 16, 1, 1) = 4000
static double calc_offscreen_score(int gridx, int gridy, int windowwidth, int windowheight) {
	double score = 0;
	if ((gridx + windowwidth) > GRIDSIZE) {
		score += windowheight*4000;
	}
	if ((gridy + windowheight) > GRIDSIZE) {
		score += windowwidth*4000;
	}
	return score;
}

static double calc_positional_score(int gridx, int gridy) {
	int x = (GRIDSIZE/2) - 1;
	int y = (GRIDSIZE/2) - 1;
	int direction = 0;
	int chainsize = 1;
	double score = 0;

	//SPIRAL MAGIC
	for (int k=1; k<=(GRIDSIZE-1); k++) {
        for (int j=0; j<(k<(GRIDSIZE-1)?2:3); j++) {
            for (int i=0; i<chainsize; i++) {
				if (x==gridx && y==gridy) {
					return score;
				} else {
					score += 1;
				}
				//do thing on [x][y]

                switch (direction) {
                    case 0: y = y + 1; break;
                    case 1: x = x + 1; break;
                    case 2: y = y - 1; break;
                    case 3: x = x - 1; break;
                }
            }
            direction = (direction+1)%4;
        }
        chainsize += 1;
    }
	//should never reach here
	return score;
}

//windowwidth, windowheight uses cells as its unit
double calculate_score(int grid[GRIDSIZE][GRIDSIZE], int gridx, int gridy, int windowwidth, int windowheight) {
	return calc_offscreen_score(gridx, gridy, windowwidth, windowheight) + 
	       calc_blocking_score(grid, gridx, gridy, windowwidth, windowheight) +
	       calc_positional_score(gridx, gridy);
}

struct Position find_best_position (int windowwidth, int windowheight) {

	struct Position bestposition;
	double bestscore, tmpscore;
	//bestscore = tmpscore = calculate_score(0, 0, windowwidth, windowheight);
	bestscore = tmpscore = calculate_score(grid, 0, 0, windowwidth, windowheight);

	for (int i=0; i<GRIDSIZE; i++) {
		for (int k=0; k<GRIDSIZE; k++) {
			//tmpscore = calculate_score(i, k, windowwidth, windowheight);
			tmpscore = calculate_score(grid, i, k, windowwidth, windowheight);
			if (tmpscore < bestscore) {
				bestscore = tmpscore;
				bestposition.x = i;
				bestposition.y = k;
			}
		}
	}
	printf("bestscore: %f\n", bestscore);
	return bestposition;
}


