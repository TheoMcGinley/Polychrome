// all functions dealing with focus and changes of focus may be found below
#include "polychrome.h" 
void resetFocusedBorder() {
	if (focused != NULL)
		XSetWindowBorder(dpy, focused->id, colorToPixelValue(focused->color));
}

void focusClient(Client *c) {
	XRaiseWindow(dpy, c->id);
	XSetInputFocus(dpy, c->id, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, c->id, FOCUSCOLOR);
	focused = c;
}

void focusColor(int color) {

	//if no windows of given colour exist, don't do anything
	Client *c = &clientlist[color];
	if (c->next == NULL) {
		return;
	}

	/*2 scenarios: 
	 * 	1) trying to focus new window of same colour as current
	 * 	2) trying to focus window of different color
	 *
	 * 	for 1):
	 * 	the current focused window will be found somewhere in the list. 
	 * 	Focus the next window in the list (or first if at end of list)
	 *
	 * 	for 2):
	 * 	the current focused window will not be found in the list. Focus the
	 * 	first window in the list
	 *
	 * 	potential optimisation:
	 * 	remember current border colour to skip scenario 2) completely -
	 * 	if selecting a different colour to current, just select first in list
	 * 	and return
	*/

	resetFocusedBorder();

	c = c->next;
	Client *firstclient = c;
	while (c->next != NULL) {
		//scenario 1)
		if (c->id == focused->id) {
			c = c->next;
			focusClient(c);
			return;
		}
		c = c->next;
	}

	//scenario 2)
	focusClient(firstclient);
}


// cycle through windowlists until an unfocused window is found
void focusNewClient() {
	Client *c;
	for (int i=0; i<NUMCOLORS; i++) {
		c = &clientlist[i];
		if (c->next != NULL && c->next->id != focused->id) {
			c = c->next;
			focusClient(c);
			return;
		}
	}
	focused = NULL;
}


