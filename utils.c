#include "polychrome.h" 
#include <X11/Xmd.h>

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

//find areas of grid where new client will be and icnrement them
//XResizeWindow(dpy, focused.id, width*2, height*2)

void double_focused_size() {

	//increment all cells of grid newly occupied by doubling
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			//TODO check edge conditions here
			if (i > (focused->pos.x + focused->width) && i <= (focused->pos.x + (focused->width)*2)
			&&  j > (focused->pos.y + focused->height) && j <= (focused->pos.y + (focused->height)*2)) { 
				grid[i][j] += 1;
			}
		}
	}

	XResizeWindow(dpy, focused->id, (focused->width)*2, (focused->height)*2);
	focused->width = focused->width * 2;
	focused->height = focused->height * 2;
}

void reset_focused_border() {
	//if (focused->id != UNDEFINED)
	if (focused != NULL)
		XSetWindowBorder(dpy, focused->id, color_to_pixel_value(focused->color));
}

//TODO make sure focused = c; really works. otherwise, set each value
//indiviudlaly
void focus_client(Client *c) {
	XRaiseWindow(dpy, c->id);
	XSetInputFocus(dpy, c->id, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, c->id, FOCUSCOLOR);
	focused = c;
}

void focus_color(int color) {

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

	reset_focused_border();

	c = c->next;
	Client *firstclient = c;
	while (c->next != NULL) {
		//scenario 1)
		if (c->id == focused->id) {
			c = c->next;
			focus_client(c);
			return;
		}
		c = c->next;
	}

	//scenario 2)
	focus_client(firstclient);
}

//if client acknowledges ICCCM's WM_DELETE_WINDOW, close it nicely, else KILL
void destroy_focused_client() {
	//please don't try and kill the root window
	//if (focused->id == UNDEFINED) return;
	if (focused == NULL) return;

    int i, n, found = 0;
    Atom *protocols;

    if (XGetWMProtocols(dpy, focused->id, &protocols, &n)) {
        for (i=0; i<n; i++) if (protocols[i] == wm_delete) found++;
        XFree(protocols);
    }
    if (found){
		//from: https://nachtimwald.com/2009/11/08/sending-wm_delete_window-client-messages/
		XEvent ev;
		ev.xclient.type = ClientMessage;
		ev.xclient.window = focused->id;
		ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, focused->id, False, NoEventMask, &ev);	
	} else {
		XKillClient(dpy, focused->id);
	}
}

Client*  add_to_clientlist(Window w, struct Position wpos, int wwidth, int wheight, int wcolor) {
	Client *c = &clientlist[wcolor];
	while (c->next != NULL) {
		c = c->next;
	}

	Client *newclient = malloc(sizeof(*newclient));
	newclient->id = w;
	newclient->pos = wpos;
	newclient->width = wwidth;
	newclient->height = wheight;
	newclient->color = wcolor;
	newclient->next = NULL;
	c->next = newclient;
	return newclient;
}

/* it is in situations such as this that a structure such as a hashmap would
 * have been much superior to an array of linkedlists, being O(1) rather than
 * O(n). However, as n (being the number of windows) is never significantly
 * large (>1000), the performance boost gained by reimplenting the whole window
 * manager with e.g. a hash map is minimal compared to the developer time taken to 
 * refactor */
int window_exists(Window w) {
	Client *c;

	for (int i=0; i<NUMCOLORS; i++) {
		c = &clientlist[i];

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

/* cycle through windowlists until a unfocused window is found
 * note that the parameter given is the 
*/

void focus_new_client() {
	Client *c;
	for (int i=0; i<NUMCOLORS; i++) {
		c = &clientlist[i];
		if (c->next != NULL && c->next->id != focused->id) {
			c = c->next;
			focus_client(c);
			return;
		}
	}
	focused = NULL;
	/*focused->id = UNDEFINED;
	focused->color = UNDEFINED;*/
}

//"handle" here means ignore
int handle_xerror(Display *dpy, XErrorEvent *e) {
	printf("???\n");
	return 0;
}


// SHAMELESSLY STOLEN FROM AEWM
void set_wm_state(Window win, int state)
{
    CARD32 data[2];

    data[0] = state;
    data[1] = None; /* Icon? We don't need no steenking icon. */

    XChangeProperty(dpy, win, wm_state, wm_state,
        32, PropModeReplace, (unsigned char *)data, 2);
}
