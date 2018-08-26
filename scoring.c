#include "polychrome.h"
#include "limits.h"

static double calc_blocking_score(int grid[GRIDWIDTH][GRIDHEIGHT], int gridx, int gridy, int windowwidth, int windowheight) {
	double score = 0;
	//for each cell
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			//2 checks:
			// 	1) i is between (inclusive) gridx and (gridx+windowwidth)
			// 	2) j is between (inclusive) gridy and (gridy+windowheight)
			//TODO check edge conditions here
			if ( i >= gridx && i < (gridx+windowwidth) && 
			  	 j >= gridy && j < (gridy+windowheight)) {
					score += (grid[i][j])*100;
			}
		}
	}
	return score;
}

static int is_offscreen(int gridx, int gridy, int windowwidth, int windowheight) {
	if ((gridx + windowwidth) > GRIDWIDTH || (gridy + windowheight) > GRIDHEIGHT)
		return 1;	
	return 0;
}

static double calc_positional_score(int gridx, int gridy, int windowwidth, int windowheight) {
	//find perfect spot given window dimensions
	int perfectx = (GRIDWIDTH/2)-(windowwidth/2);
	int perfecty = (GRIDHEIGHT/2)-(windowheight/2);

	//manhattan distance away from perfect spot is score
	return abs(perfectx-gridx) + abs(perfecty-gridy);
}

//windowwidth, windowheight uses cells as its unit
double calculate_score(int grid[GRIDWIDTH][GRIDHEIGHT], int gridx, int gridy, int windowwidth, int windowheight) {
	if (is_offscreen(gridx, gridy, windowwidth, windowheight)) 
		return INT_MAX;
	return calc_blocking_score(grid, gridx, gridy, windowwidth, windowheight) +
	       calc_positional_score(gridx, gridy, windowwidth, windowheight);
}

struct Position find_best_position (int windowwidth, int windowheight) {

	struct Position bestposition;
	bestposition.x = 0;
	bestposition.y = 0;
	double bestscore, tmpscore;
	//bestscore = tmpscore = calculate_score(0, 0, windowwidth, windowheight);
	bestscore = tmpscore = calculate_score(grid, 0, 0, windowwidth, windowheight);

	for (int i=0; i<GRIDWIDTH; i++) {
		for (int k=0; k<GRIDHEIGHT; k++) {
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


