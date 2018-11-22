#include "polychrome.h"

// globals defining window manager state
Display * dpy;

XWindowAttributes attr;
XButtonEvent pointerOrigin;

Workspace workspace[NUMWORKSPACES];
int currentWorkspace;

enum NewWindowDimensions newDimensions;

Atom wm_state;
Atom wm_change_state;
Atom wm_protos;
Atom wm_delete;


static int init(void);

int main(void) {

	//if initialisation failed, exit with error
	if(!init()) {
		return 1;	
	}

	handleEvents();
	return 0;
}

// Look for windows that already exist 
static void scanWins() {
    unsigned int nwins, i;
    Window dummyw1, dummyw2, *wins;
    XWindowAttributes attr;

    XQueryTree(dpy, DefaultRootWindow(dpy), &dummyw1, &dummyw2, &wins, &nwins);
    for (i = 0; i < nwins; i++) {
        XGetWindowAttributes(dpy, wins[i], &attr);
        if (!attr.override_redirect && attr.map_state == IsViewable)
            XMapWindow(dpy, wins[i]);
    }
    XFree(wins);
}



static int init() {

    if(!(dpy = XOpenDisplay(0x0))) {
		return 0;
	}

	//ICCCM, taken from aewm
    wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wm_state = XInternAtom(dpy, "WM_STATE", False);
    wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);

	currentWorkspace = 0;

	//for each workspace, initialise the linked lists and colortracker
	for (int j=0;j<NUMWORKSPACES;j++) {
		for (int i=0;i<NUMCOLORS;i++) {
			workspace[j].colorTracker[i] = 0;
			workspace[j].clientList[i].id = UNDEFINED;
			workspace[j].clientList[i].next = NULL;
		}
	}

	//for each workspace, initialise the grid
	for (int k=0;k<NUMWORKSPACES;k++) {
		for (int i=0; i<GRIDWIDTH; i++) {
			for (int j=0; j<GRIDHEIGHT; j++) {
				workspace[k].grid[i][j] = 0;
			}
		}
	}

	for (int i=0;i<NUMWORKSPACES;i++) {
		workspace[i].focused = NULL;
	}

	newDimensions = REGULAR;

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

	// hide/show
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("i")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("o")), Mod1Mask,
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

	// workspaces 
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("j")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("k")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("l")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym(";")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    pointerOrigin.subwindow = None;

	scanWins();
	return 1;
}

