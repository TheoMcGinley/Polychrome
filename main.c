#include "polychrome.h"


Display * dpy;
XWindowAttributes attr;
XButtonEvent pointerorigin;

int colortracker[NUMCOLORS];
WindowNode windowlist[NUMCOLORS];
int grid[GRIDSIZE][GRIDSIZE];
int nextsize;
int nextorientation;

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

	//start_app("feh --bg-tile -z /home/theo/pix/backgrounds/tiling/");

	//XSetErrorHandler(handle_xerror);

	//initialise the linked lists and colortracker
	for (int i=0;i<NUMCOLORS;i++) {
		colortracker[i] = 0;
		windowlist[i].id = UNDEFINED;
		windowlist[i].next = NULL;
	}

	for (int i=0; i<GRIDSIZE; i++) {
		for (int j=0; j<GRIDSIZE; j++) {
			grid[i][j] = 0;
		}
	}

	focused.id = UNDEFINED;
	focused.color = UNDEFINED;

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
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("r")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("p")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    pointerorigin.subwindow = None;

	printf("init over\n");
	return 1;
}

