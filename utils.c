#include "polychrome.h" 
#include <X11/Xmd.h>

void startApp(const char *command) {
	if (fork() == 0) {
		execl(_PATH_BSHELL, _PATH_BSHELL, "-c", command, NULL);
	}
}

int colorToPixelValue(int color) {
	switch (color) {
		case 0:
			//return 25600; //equivalent to (0,100,0)
			return 11169350; //equivalent to (170, 110, 70)
		case 1:
			return 5208386;
		case 2: 
			return 16494290; 
		case 3:
			return 4014460;
	}
	return 0;
}

//updateType is either ADD or REMOVE (1 or -1 respectively)
void updateGrid(IntTuple position, IntTuple clientDimensions, int updateType) {
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ( i >= position.x && i < (position.x + clientDimensions.x) && 
			  	 j >= position.y && j < (position.y + clientDimensions.y)) {
					grid[i][j] += updateType;
			}
		}
	}
}


//find the least used colour
int rarestColour() {
	int minValue = colorTracker[0];
	int minColor = 0;
	for (int i=1; i<NUMCOLORS; i++) {
		if (colorTracker[i] < minValue) {
			minValue = colorTracker[i];
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
		c = &clientList[i];

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

