#include "polychrome.h" 
#include <X11/Xmd.h>

void startApp(const char *command) {
	if (fork() == 0) {
		execl(_PATH_BSHELL, _PATH_BSHELL, "-c", command, NULL);
	}
}

// the formula is equivalent to (256*256*r + 256*g + b)
int colorToPixelValue(int color) {
	switch (color) {
		case 0: 
			return 16502449; // "a" for "apricot" (251, 206, 177)
			// return 10053324; // "a" for "amethyst" (153, 102, 204)
		case 1: 
			return 16041008; // "s" for "saffron" (244, 196, 48)
			// return 1004218;  // "s" for "sapphire" (15, 82, 186) 
		case 2: 
			return 1401021;  // "d" for "denim" (21, 96, 189)
			// return 13362160; // "d" for "diamond" (203, 227, 240)
		case 3: 
			return 16140938; // "f" for "french rose" (246, 74, 138)
			// return 11674146; // "f" for "firebrick" (178, 34, 34) (sorry)
	}
	return 0;
}

//updateType is either ADD or REMOVE (1 or -1 respectively)
void updateGrid(IntTuple position, IntTuple clientDimensions, int updateType, int workspaceToUpdate) {
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ( i >= position.x && i < (position.x + clientDimensions.x) && 
			  	 j >= position.y && j < (position.y + clientDimensions.y)) {
					workspace[workspaceToUpdate].grid[i][j] += updateType;
			}
		}
	}
}


//find the least used colour for the current workspace
int rarestColour() {
	int minValue = CWS.colorTracker[0];
	int minColor = 0;
	for (int i=1; i<NUMCOLORS; i++) {
		if (CWS.colorTracker[i] < minValue) {
			minValue = CWS.colorTracker[i];
			minColor = i;
		}
	}
	return minColor;
}

/* it is in situations such as this that a structure such as a hashmap would
 * have been much superior to an array of linkedlists, being O(1) rather than
 * O(n). However, as n (being the number of windows) is never significantly
 * large (>1000), the performance boost gained by reimplenting the whole window
 * manager with e.g. a hash map is minimal compared to the developer time taken to 
 * refactor */
int windowExists(Window w) {
	Client *c;

	for (int i=0; i<NUMCOLORS; i++) {
		c = &CWS.clientList[i];

		//if list empty, go to next list 
		if (c->next == NULL) {
			continue;
		}

		while (c->next != NULL) {

			if (c->next->id == w) {
				return 1;
			}
			c = c->next;
		}
	}
	return 0;
}


//"handle" here means ignore
int handleXerror(Display *dpy, XErrorEvent *e) {
	printf("???\n");
	return 0;
}

