#include "polychrome.h" 

void start_app(const char *command) {
	if (fork() == 0) {
		execl(_PATH_BSHELL, _PATH_BSHELL, "-c", command, NULL);
	}
}

int color_to_pixel_value(int color) {
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

void reset_focused_border(){
	XSetWindowBorder(dpy, focused.id, color_to_pixel_value(focused.color));
}

void focus_window(Window w, int color) {
	XRaiseWindow(dpy, w);
	XSetInputFocus(dpy, w, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, w, FOCUSCOLOR);
	focused.id = w;
	focused.color = color;
}

void focus_color(int color) {

	//if no windows of given colour exist, don't do anything
	WindowNode *wn = &windowlist[color];
	if (wn->next == NULL) {
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

	reset_focused_border();

	wn = wn->next;
	Window firstwindow = wn->id;
	while (wn->next != NULL) {
		//scenario 1)
		if (wn->id == focused.id) {
			wn = wn->next;
			focus_window(wn->id, color);
			return;
		}
		wn = wn->next;
	}

	//scenario 2)
	focus_window(firstwindow, color);
}

void destroy_active_window() {
	XEvent ev;
	 
	//memset(&ev, 0, sizeof (ev));
	 
	//assumes ICCCM compliance
	//from: https://nachtimwald.com/2009/11/08/sending-wm_delete_window-client-messages/
	ev.xclient.type = ClientMessage;
	ev.xclient.window = focused.id;
	ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, focused.id, False, NoEventMask, &ev);	
}

void add_to_windowlist(Window w, struct Position wpos, int wwidth, int wheight, int windowcolor) {
	WindowNode *wn = &windowlist[windowcolor];
	while (wn->next != NULL) {
		wn = wn->next;
	}

	WindowNode *newnode = malloc(sizeof(*newnode));
	newnode->id = w;
	newnode->pos = wpos;
	newnode->width = wwidth;
	newnode->height = wheight;
	newnode->next = NULL;
	wn->next = newnode;
}

/* it is in situations such as this that a structure such as a hashmap would
 * have been much superior to an array of linkedlists, being O(1) rather than
 * O(n). However, as n (being the number of windows) is never significantly
 * large (>1000), the performance boost gained by reimplenting the whole window
 * manager with e.g. a hash map is minimal compared to the developer time taken to 
 * refactor */
int window_exists(Window w) {
	WindowNode *wn;

	for (int i=0; i<NUMCOLORS; i++) {
		wn = &windowlist[i];

		//if list empty, go to next list 
		if (wn->next == NULL) {
			continue;
		}

		while (wn->next != NULL) {

			if (wn->next->id == w) {
				return 1;
			}
			wn = wn->next;
		}
	}
	return 0;
}

/* cycle through windowlists until a window is found
 * the window_exists is needed as some programs create invisible windows then delete them
 * resulting in a window of color 0 being selected, messing up handle_map,
 * which attempts to colour the previously selected window back to its
 * original colour when a new window is mapped
*/

void focus_new_window() {
	WindowNode *wn;
	for (int i=0; i<NUMCOLORS; i++) {
		wn = &windowlist[i];
		if (wn->next != NULL) {
			wn = wn->next;
			focus_window(wn->id, i);
			return;
		}
	}
}

//???
int handle_xerror(Display *dpy, XErrorEvent *e) {
	printf("???\n");
	return 0;
}


