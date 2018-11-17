#include "polychrome.h"
#include "limits.h"

// add a penalty of 100 for each cell that the new window would overlap with
// existing windows - ensures minimum overlap
static double calcBlockingScore(IntTuple position, IntTuple dimensions) {
	double score = 0;
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ( i >= position.x && i < (position.x + dimensions.x) && 
			  	 j >= position.y && j < (position.y + dimensions.y)) {
					score += (grid[i][j])*100;
			}
		}
	}
	return score;
}

// determines if putting the window in the given position would result in the
// window being at least partially offscreen
static int isOffscreen(IntTuple position, IntTuple dimensions) {
	if ((position.x + dimensions.x) > GRIDWIDTH || 
			(position.y + dimensions.y) > GRIDHEIGHT)
		return 1;	
	return 0;
}

// calcPositionScores prefers windows closer to the centre of the screen - 
// Manhattan distance away from the centre is the penalty
static double calcPositionalScore(IntTuple position, IntTuple dimensions) {
	int perfectX = (GRIDWIDTH/2)  - (dimensions.x/2);
	int perfectY = (GRIDHEIGHT/2) - (dimensions.y/2);
	return abs(perfectX - position.x) + abs(perfectY - position.y);
}

/* calculateScore prioritises:
 * 1) not putting a window offscreen
 * 2) not overlapping windows
 * 3) putting the window as close to the centre as possible
 */
static double calculateScore(IntTuple position, IntTuple dimensions) {
	if (isOffscreen(position, dimensions)) 
		return INT_MAX;
	return calcBlockingScore(position, dimensions) + 
	       calcPositionalScore(position, dimensions);
}

// given the dimensions of the window to add, find the best place to put it
IntTuple findBestPosition (IntTuple dimensions) {
	IntTuple bestPosition, tmpPosition;
	double bestScore, tmpScore;

	bestPosition.x = 0;
	bestPosition.y = 0;
	bestScore = tmpScore = calculateScore(bestPosition, dimensions);

	// lower scores are better
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int k=0; k<GRIDHEIGHT; k++) {
			tmpPosition.x = i;
			tmpPosition.y = k;
			tmpScore = calculateScore(tmpPosition, dimensions);
			if (tmpScore < bestScore) {
				bestScore = tmpScore;
				bestPosition.x = i;
				bestPosition.y = k;
			}
		}
	}
	printf("bestscore: %f\n", bestScore);
	return bestPosition;
}
