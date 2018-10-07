#include "polychrome.h"

Display * dpy;

XWindowAttributes attr;
XButtonEvent pointerorigin;

int colortracker[NUMCOLORS];
Client clientlist[NUMCOLORS];
Client *focused;
int grid[GRIDWIDTH][GRIDHEIGHT];

enum NewWindowDimensions newdimensions;

Atom wm_state;
Atom wm_change_state;
Atom wm_protos;
Atom wm_delete;


static int initialise(void);

int main(void) {

	//if initialisation failed, exit with error
	if(!initialise()) {
		return 1;	
	}

	event_loop();
	return 0;
}

static int initialise() {

    if(!(dpy = XOpenDisplay(0x0))) {
		return 0;
	}

	//root = RootWindow(dpy, DefaultScreen(dpy));
	//printf("ROOT: %lx\n", root);
	//start_app("feh --bg-tile -z /home/theo/pix/backgrounds/tiling/");

	//XSetErrorHandler(handle_xerror);

	//ICCCM, taken from aewm
    wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wm_state = XInternAtom(dpy, "WM_STATE", False);
    wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);


	//initialise the linked lists and colortracker
	for (int i=0;i<NUMCOLORS;i++) {
		colortracker[i] = 0;
		clientlist[i].id = UNDEFINED;
		clientlist[i].next = NULL;
	}

	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			grid[i][j] = 0;
		}
	}

	//TODO potentially set other fields of focused?
	focused = NULL;
	newdimensions = REGULAR;
	/*focused.id = UNDEFINED;
	focused.color = UNDEFINED;*/

	//make all children of root give out notify events
	XSelectInput (dpy, RootWindow(dpy, DefaultScreen(dpy)), SubstructureNotifyMask);    
	XSetInputFocus(dpy, RootWindow(dpy, DefaultScreen(dpy)), RevertToNone, CurrentTime);

	//grab keys needed for the wm
    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Return")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("a")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("s")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("d")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("f")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("g")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("h")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("q")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("z")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("x")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("c")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("b")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("p")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
	//orientations
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("r")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("p")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("w")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("e")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);



	//SHIFTS
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("g")), ShiftMask|Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("h")), ShiftMask|Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);


    pointerorigin.subwindow = None;

	printf("init over\n");
	return 1;
}

