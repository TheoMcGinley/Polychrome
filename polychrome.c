//TODO add position, width and height to windownode
//TODO include position, width and height when adding to widnownode
#include <X11/Xlib.h>
#include <stdio.h> //printf
#include <unistd.h> //NULL, exit, fork, sleep
#include <stdlib.h> //NULL, malloc, free, exit, system
#include <paths.h>
#include "scoring.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define NUMCOLORS 4
#define NUMBUCKETS 20
#define FOCUSCOLOR 16777111 //soft yellow
				 //16777215 // white
#define UNDEFINED -1
#define GRIDSIZE 16 //RESTRICTIONS: must be even, and at least 4
//#define CELLWIDTH (WidthOfScreen()/GRIDSIZE) //TODO make these work
//#define CELLHEIGHT (HeightOfScreen()/GRIDSIZE)
#define CELLWIDTH 240
#define CELLHEIGHT 135
#define BORDERTHICKNESS 10

//may need a "trackedbygrid" var to indicate whether the window should be
//tracked or not e.g. for pop-ups, steam chat etc
//Polychrome Window - a better alternative than XGetInputFocus
struct PWindow {
	Window id;
	int color;
} focused;

struct Position {
	int x;
	int y;
};

struct WindowNode {
	Window id;
	struct Position pos;
	int width;
	int height;
	struct WindowNode *next;
};

struct WindowNode;
typedef struct WindowNode WindowNode;



int handle_xerror(Display *, XErrorEvent *);

static int initialise();
static void handle_button_press(XButtonEvent *);
static void handle_button_release(XButtonEvent *);
static void handle_key_press(XKeyEvent *);
static void handle_window_destruction(XDestroyWindowEvent *);
static void handle_motion(XMotionEvent *);
static void handle_window_map(XMapEvent *);


static Display * dpy;
static XWindowAttributes attr;
static XButtonEvent pointerorigin;

//group these three as a struct desktopstate ?
static int colortracker[NUMCOLORS];
static WindowNode windowlist[NUMCOLORS];
static int grid[GRIDSIZE][GRIDSIZE];

static int nextsize;
static int nextorientation;

/*
//optionally give it a grid?
int calc_blocking_score(int gridx, int gridy, int windowwidth, int windowheight) {
	int score = 0;
	//for each cell
	for (int i=0; i<GRIDSIZE; i++) {
		for (int j=0; j<GRIDSIZE; j++) {
			//2 checks:
			// 	1) i is between (inclusive) gridx and (gridx+windowwidth)
			// 	2) j is between (inclusive gridy and (gridy+windowheight)
			//TODO check edge conditions here
			if ( i >= gridx && i < (gridx+windowwidth) && 
			  	 j >= gridy && j < (gridy+windowheight)) {
					score += (grid[i][j])*1000;
			}
		}
	}
	return score;
}

//assuming GRIDSIZE=16
//assert(15, 15, 1, 1) = 0
//assert(16, 16, 1, 1) = 4000
int calc_offscreen_score(int gridx, int gridy, int windowwidth, int windowheight) {
	int score = 0;
	if ((gridx + windowwidth) > GRIDSIZE) {
		score += windowheight*4000;
	}
	if ((gridy + windowheight) > GRIDSIZE) {
		score += windowwidth*4000;
	}
	return score;
}

int calc_positional_score(int gridx, int gridy) {
	int x = (GRIDSIZE/2) - 1;
	int y = (GRIDSIZE/2) - 1;
	int direction = 0;
	int chainsize = 1;
	int score = 0;

	//SPIRAL MAGIC
	for (int k=1; k<=(GRIDSIZE-1); k++) {
        for (int j=0; j<(k<(GRIDSIZE-1)?2:3); j++) {
            for (int i=0; i<chainsize; i++) {
				if (x==gridx && y==gridy) {
					return score;
				} else {
					score += 1;
				}
				//do thing on [x][y]

                switch (direction) {
                    case 0: y = y + 1; break;
                    case 1: x = x + 1; break;
                    case 2: y = y - 1; break;
                    case 3: x = x - 1; break;
                }
            }
            direction = (direction+1)%4;
        }
        chainsize += 1;
    }
	//should never reach here
	return score;
}

//windowwidth, windowheight uses cells as its unit
int calculate_score(int gridx, int gridy, int windowwidth, int windowheight) {
	return calc_offscreen_score(gridx, gridy, windowwidth, windowheight) + 
	       calc_blocking_score(gridx, gridy, windowwidth, windowheight) +
	       calc_positional_score(gridx, gridy);
}
*/

struct Position find_best_position (int windowwidth, int windowheight) {

	struct Position bestposition;
	double bestscore, tmpscore;
	//bestscore = tmpscore = calculate_score(0, 0, windowwidth, windowheight);
	bestscore = tmpscore = calculate_score(grid, 0, 0, windowwidth, windowheight);

	for (int i=0; i<GRIDSIZE; i++) {
		for (int k=0; k<GRIDSIZE; k++) {
			//tmpscore = calculate_score(i, k, windowwidth, windowheight);
			tmpscore = calculate_score(grid, i, k, windowwidth, windowheight);
			if (tmpscore < bestscore) {
				bestscore = tmpscore;
				bestposition.x = i;
				bestposition.y = k;
			}
		}
	}
	printf("bestscore: %f\n", bestscore);
	return bestposition;
}


void start_app(const char *command) {
	if (fork() == 0) {
		execl(_PATH_BSHELL, _PATH_BSHELL, "-c", command, NULL);
	}
}

/*void shell_command(char * command) {
	pid_t pid = fork();
	if (pid == 0) {
		setsid();
		printf("%s", command);
		execlp("/bin/sh", "sh", "-C", command, NULL);
		exit(1);
	}
}*/

int main(void) {
    XEvent ev;

	//if initialisation failed, exit with error
	if(!initialise()) {
		return 1;	
	}

    for(;;)
    {

        XNextEvent(dpy, &ev);

		//FROM AEWM
		switch (ev.type) {
            case ButtonPress:
                handle_button_press(&ev.xbutton); break;
            case ButtonRelease:
                handle_button_release(&ev.xbutton); break;
			case KeyPress:
				handle_key_press(&ev.xkey); break;
			case MapNotify:
				handle_window_map(&ev.xmap); break;
			case DestroyNotify:
				handle_window_destruction(&ev.xdestroywindow); break;
			case MotionNotify:
				handle_motion(&ev.xmotion); break;
			default:
				//printf("some janky event detected: %d\n", ev.type);
				break;
		}

    }
}

static void handle_button_press(XButtonEvent *e) {
	if (e->subwindow != None) {
		XGetWindowAttributes(dpy, e->subwindow, &attr);
		pointerorigin = *e;
	}
}

static void handle_button_release(XButtonEvent *e) {
	pointerorigin.subwindow = None;
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

static void reset_focused_border(){
	XSetWindowBorder(dpy, focused.id, color_to_pixel_value(focused.color));
}

static void focus_window(Window w, int color) {
	XRaiseWindow(dpy, w);
	XSetInputFocus(dpy, w, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, w, FOCUSCOLOR);
	focused.id = w;
	focused.color = color;
}

static void focus_color(int color) {

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


static void destroy_active_window() {
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


//use xev to find correct keysyms
static void handle_key_press(XKeyEvent *e) {
	switch(e->keycode) {
		case 36: // enter
			start_app("urxvt");
			break;
		case 38: // "a"
			focus_color(0);
			break;
		case 39: // "s"
			focus_color(1);
			break;
		case 40: // "d"
			focus_color(2);
			break;
		case 41: // "f"
			focus_color(3);
			break;
		case 24: // "q"
			destroy_active_window();
			break;
		case 27: // "r"
			start_app("java -jar ~/vlt/RuneLite.jar");
			break;
		case 33: // "p"
			start_app("scrot -u ~/pix/scrots/%Y-%m-%d-%T-screenshot.png");
			break;
		case 52: // "z"
			start_app("mpc toggle -q");
			break;
		case 53: // "x"
			start_app("mpc next -q");
			break;
		case 54: // "c"
			start_app("nextalbum");
			break;
		case 56: // "b"
			start_app("firefox");
			break;
	}
}

//give the window a border and border colour
static void add_to_windowlist(Window w, struct Position wpos, int wwidth, int wheight, int windowcolor) {
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


static void handle_window_map(XMapEvent *e) {
	
	//find rarest color and update tracker
	int minvalue = colortracker[0];
	int mincolor = 0;
	for (int i=1; i<NUMCOLORS; i++) {
		if (colortracker[i] < minvalue) {
			minvalue = colortracker[i];
			mincolor = i;
		}
	}

	colortracker[mincolor] = colortracker[mincolor] + 1;

	XSetWindowBorderWidth(e->display, e->window, BORDERTHICKNESS);

	//focus new window, setting border of old focus back to regular color
	if (focused.id != UNDEFINED) {
		reset_focused_border();
	}

	//TODO accept givenwidth, givenheight as input from user
	int givenwidth = 4;
	int givenheight = 4;
	struct Position pos = find_best_position(givenwidth, givenheight);
	XMoveResizeWindow(dpy, e->window, (pos.x*CELLWIDTH)+BORDERTHICKNESS,
			(pos.y*CELLHEIGHT)+BORDERTHICKNESS, (givenwidth*CELLWIDTH)-(2*BORDERTHICKNESS),
			(givenheight*CELLHEIGHT)-(2*BORDERTHICKNESS));
	//printf("x: %d, y: %d\n", pos.x*CELLWIDTH, pos.y*CELLHEIGHT);
	printf("x: %d, y: %d\n", pos.x, pos.y);

	//populate_grid
	for (int i=0; i<GRIDSIZE; i++) {
		for (int j=0; j<GRIDSIZE; j++) {
			if ( i >= pos.x && i < (pos.x+givenheight) && 
			  	 j >= pos.y && j < (pos.y+givenheight)) {
					grid[i][j] += 1;
			}
		}
	}

	//add window to relevant linked list
	add_to_windowlist(e->window, pos, givenwidth, givenheight, mincolor);



	//print grid
	for (int i=0; i<16; i++) {
		for (int j=0; j<16; j++) {
			printf("%d ", grid[i][j]);
		}
		printf("\n");
	}


	focus_window(e->window, mincolor);
}



/* we only get motion events when a button is being pressed,
 * but we still have to check that the drag started on a window */
static void handle_motion(XMotionEvent *e) {
	/*
	* here we could "compress" motion notify events by doing:
	 *
	 * while(XCheckTypedEvent(dpy, MotionNotify, &ev));
	 *
	 * if there are 10 of them waiting, it makes no sense to look at
	 * any of them but the most recent.  in some cases -- if the window
	 * is really big or things are just acting slowly in general --
	 * failing to do this can result in a lot of "drag lag," especially
	 * if your wm does a lot of drawing and whatnot that causes it to
	 * lag.
	 *
	 * for window managers with things like desktop switching, it can
	 * also be useful to compress EnterNotify events, so that you don't
	 * get "focus flicker" as windows shuffle around underneath the
	 * pointer.
	 */

	/* now we use the stuff we saved at the beginning of the
	 * move/resize and compare it to the pointer's current position to
	 * determine what the window's new size or position should be.
	 *
	 * if the initial button press was button 1, then we're moving.
	 * otherwise it was 3 and we're resizing.
	 *
	 * we also make sure not to go negative with the window's
	 * dimensions, resulting in "wrapping" which will make our window
	 * something ridiculous like 65000 pixels wide (often accompanied
	 * by lots of swapping and slowdown).
	 *
	 * even worse is if we get "lucky" and hit a width or height of
	 * exactly zero, triggering an X error.  so we specify a minimum
	 * width/height of 1 pixel.
	 *
	*/

	if (pointerorigin.subwindow != None) {
		int xdiff = e->x_root - pointerorigin.x_root;
		int ydiff = e->y_root - pointerorigin.y_root;
		XMoveResizeWindow(dpy, pointerorigin.subwindow,
			attr.x + (pointerorigin.button==1 ? xdiff : 0),
			attr.y + (pointerorigin.button==1 ? ydiff : 0),
			MAX(1, attr.width + (pointerorigin.button==3 ? xdiff : 0)),
			MAX(1, attr.height + (pointerorigin.button==3 ? ydiff : 0)));
	}
}

/* it is in situations such as this that a structure such as a hashmap would
 * have been much superior to an array of linkedlists, being O(1) rather than
 * O(n). However, as n (being the number of windows) is never significantly
 * large (>1000), the performance boost gained by reimplenting the whole window
 * manager with e.g. a hash map is minimal compared to the developer time taken to 
 * refactor */
static int window_exists(Window w) {
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

static void focus_new_window() {
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

//removes deleted window from linkedlist and colortracker
static void handle_window_destruction(XDestroyWindowEvent *e) {
	printf("destruction window id: %lx\n", e->window);

	if (e->window == focused.id) {
		focus_new_window();
	}

	int windowfound = 0;
	WindowNode *wn;
	WindowNode *wntofree = NULL;

	//find and remove window from linked lists
	for (int i=0; i<NUMCOLORS && !windowfound; i++) {
		wn = &windowlist[i];

		//if list empty, go to next list 
		if (wn->next == NULL) {
			continue;
		}

		while (wn->next != NULL) {
			if (wn->next->id == e->window) {
				wntofree = wn->next;
				windowfound = 1;
				/*if the node to delete has a next node, set current next to
				that node, else node to delete is last in list so can set current
				next to NULL */
				if (wn->next->next != NULL) {
					wn->next = wn->next->next;
				} else {
					wn->next = NULL;
				}
				//remove window from colortracker
				colortracker[i] = colortracker[i] - 1;
				break;
			}
			wn = wn->next;
		}
	}

	if (wntofree != NULL) {

		for (int i=0; i<GRIDSIZE; i++) {
			for (int j=0; j<GRIDSIZE; j++) {
				if ( i >= wntofree->pos.x && i < (wntofree->pos.x + wntofree->width) && 
					 j >= wntofree->pos.y && j < (wntofree->pos.y + wntofree->height)) {
						grid[i][j] -= 1;
				}
			}
		}
		free(wntofree);
	}
}



static int initialise() {

    if(!(dpy = XOpenDisplay(0x0))) {
		return 0;
	}

	start_app("feh --bg-tile -z /home/theo/pix/backgrounds/tiling/");

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
	return 1;
}

/* --- UTILS --- */

int handle_xerror(Display *dpy, XErrorEvent *e) {
	printf("???\n");
	return 0;
}

//future TODO: replace Mod1Mask (alt) with Mod4Mask (meta)
