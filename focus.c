// all functions dealing with focus and changes of focus may be found below
#include "polychrome.h" 

// set the border of the focused window back to its regular color
static void resetFocusedBorder() {
	if (CWS.focused != NULL)
		XSetWindowBorder(dpy, CWS.focused->id, colorToPixelValue(CWS.focused->color));
}

// NoReset is required when focusing a new window after a window is deleted
// as otherwise would be calling XSetWindowBorder on a non-existent window
static void focusClientNoReset(Client *c) {
	XRaiseWindow(dpy, c->id);
	XSetInputFocus(dpy, c->id, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, c->id, FOCUSCOLOR);
	CWS.focused = c;
}

// focus the specified client, setting its border to the focus colour
void focusClient(Client *c) {
	resetFocusedBorder();
	focusClientNoReset(c);
}


// focus a window of the specified colour
// if the current focused window is of the same colour, focus the next window
// in the list of the same colour
void focusColor(int color) {

	//if no windows of given colour exist, don't do anything
	Client *c = &CWS.clientList[color];
	if (c->next == NULL) {
		return;
	}

	// reset the border colour of the old focused window before starting
	resetFocusedBorder();

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


	c = c->next;
	Client *firstclient = c;
	while (c->next != NULL) {
		//scenario 1)
		if (c->id == CWS.focused->id) {
			c = c->next;
			focusClient(c);
			return;
		}
		c = c->next;
	}

	//scenario 2)
	focusClient(firstclient);
}


// cycle through lists until an unfocused window is found
void focusUnfocusedClient() {
	Client *c;
	for (int i=0; i<NUMCOLORS; i++) {
		c = &CWS.clientList[i];
		if (c->next != NULL && c->next->id != CWS.focused->id) {
			c = c->next;
			focusClientNoReset(c);
			return;
		}
	}

	CWS.focused = NULL;
}

