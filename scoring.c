#include "polychrome.h"
#include "limits.h"

static double calc_blocking_score(int windowx, int windowy, int windowwidth, int windowheight) {
	double score = 0;
	//for each cell
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			//2 checks:
			// 	1) i is between (inclusive) windowx and (windowx+windowwidth)
			// 	2) j is between (inclusive) windowy and (windowy+windowheight)
			//TODO check edge conditions here
			if ( i >= windowx && i < (windowx+windowwidth) && 
			  	 j >= windowy && j < (windowy+windowheight)) {
					score += (grid[i][j])*100;
			}
		}
	}
	return score;
}

static int is_offscreen(int windowx, int windowy, int windowwidth, int windowheight) {
	if ((windowx + windowwidth) > GRIDWIDTH || (windowy + windowheight) > GRIDHEIGHT)
		return 1;	
	return 0;
}

static double calc_positional_score(int windowx, int windowy, int windowwidth, int windowheight) {
	//find perfect spot given window dimensions
	int perfectx = (GRIDWIDTH/2)-(windowwidth/2);
	int perfecty = (GRIDHEIGHT/2)-(windowheight/2);

	//manhattan distance away from perfect spot is score
	return abs(perfectx-windowx) + abs(perfecty-windowy);
}

//windowwidth, windowheight uses cells as its unit
double calculate_score(int windowx, int windowy, int windowwidth, int windowheight) {
	if (is_offscreen(windowx, windowy, windowwidth, windowheight)) 
		return INT_MAX;
	return calc_blocking_score(windowx, windowy, windowwidth, windowheight) +
	       calc_positional_score(windowx, windowy, windowwidth, windowheight);
}

struct Position find_best_position (int windowwidth, int windowheight) {
	struct Position bestposition;
	bestposition.x = 0;
	bestposition.y = 0;
	double bestscore, tmpscore;
	//bestscore = tmpscore = calculate_score(0, 0, windowwidth, windowheight);
	bestscore = tmpscore = calculate_score(0, 0, windowwidth, windowheight);

	for (int i=0; i<GRIDWIDTH; i++) {
		for (int k=0; k<GRIDHEIGHT; k++) {
			//tmpscore = calculate_score(i, k, windowwidth, windowheight);
			tmpscore = calculate_score(i, k, windowwidth, windowheight);
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
