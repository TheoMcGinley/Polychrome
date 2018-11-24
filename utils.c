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


// shouldBeIgnored determines if the window is a popup, dialogue box or
// any other type of window which should not be tracked by the wm
int shouldBeIgnored(Window win) {
	Atom actualType;
	int actualFormat, status;
	unsigned long nItems, bytesAfter;
	unsigned char *propReturn = NULL;

	// iterate through window properties, see if _NET_WM_WINDOW_TYPE exists
	int nProperties;
	Bool windowProvidesWindowType = False;
	Atom windowType = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", True);
	Atom *windowProperties = XListProperties(dpy, win, &nProperties);
	for (int i=0; i<nProperties; i++) {
		if (windowProperties[i] == windowType) {
			windowProvidesWindowType = True;
		}
	}

	// if the client does not specify its window type, track it by default
	if (!windowProvidesWindowType) {
		return 0;
	}


	// adapted from xprop source code
	status = XGetWindowProperty(dpy, win, windowType,
			0L, sizeof(Atom), False, AnyPropertyType,
			&actualType, &actualFormat, &nItems, &bytesAfter,
			&propReturn);

	// if the query fails, track the window by default
	if (status != Success) {
		XFree(propReturn);
		return 0;
	}

	// clients may specify multiple window types - if any of them state
	// that the window is a normal window then track as normal, else ignore
	int ignoreWindow = 1;
	Atom prop;
	Atom NormalWindow = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", True);
	char *name;
	for (int i=0; i<nItems; i++) {
		prop = ((Atom *)propReturn)[i];
		if (prop == NormalWindow) {
			ignoreWindow = 0;
		}
	}

	XFree(propReturn);
	return ignoreWindow;

}


//"handle" here means ignore
int handleXerror(Display *dpy, XErrorEvent *e) {
	printf("???\n");
	return 0;
}

