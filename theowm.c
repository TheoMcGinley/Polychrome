//TODO proper error handling
//https://www.google.com/search?q=xlib+error+handling&ie=utf-8&oe=utf-8&client=firefox-b-ab
//TODO "XDestroyWindow" doesn't actually kill the program:
//https://stackoverflow.com/questions/10792361/how-do-i-gracefully-exit-an-x11-event-loop
//TODO fix children taking on border colour of parent
//TODO replace all case 38: with case XStringToKeySYm("a") etc.
//TODO paint selected window a different colour (pixmap pattern?)
//TODO fix firefox (and other) breakages
//TODO make keybind for:
	//mpc toggle
	//mpc next
	//nextalbum
	//qutebrowser


//#include "theowm.h"

#include <X11/Xlib.h>

// THEO PERSONAL INCLUSION
#include <stdio.h>

//included because included in aemw
#include <unistd.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define NUMCOLORS 4
#define NUMBUCKETS 20

struct WindowNode {
	Window id;
	struct WindowNode *next;
};
struct WindowNode;
typedef struct WindowNode WindowNode;

static int initialise();
int handle_xerror(Display *, XErrorEvent *);
static void handle_button_press(XButtonEvent *);
static void handle_button_release(XButtonEvent *);
static void handle_key_press(XKeyEvent *);
static void handle_window_creation(XCreateWindowEvent *);
static void handle_window_destruction(XDestroyWindowEvent *);
static void handle_motion(XMotionEvent *);

//borrowed from aewm
void spawn_terminal() {
	pid_t pid = fork();
	if (pid == 0) {
		setsid();
		execlp("/bin/sh", "sh", "-C", "xterm", NULL);
		exit(1);
	}
}

static Display * dpy;
static XWindowAttributes attr;
static XButtonEvent pointerorigin;

static int colortracker[NUMCOLORS];
static WindowNode windowlist[NUMCOLORS];


int main(void) {
    XEvent ev;

	//if initialisation failed, exit with error
	if(!initialise()) {
		return 1;	
	}

    for(;;)
    {

        /* this is the most basic way of looping through X events; you can be
         * more flexible by using XPending(), or ConnectionNumber() along with
         * select() (or poll() or whatever floats your boat).
         */

        XNextEvent(dpy, &ev);

		//FROM AEWM
		switch (ev.type) {
            case ButtonPress:
                handle_button_press(&ev.xbutton); break;
            case ButtonRelease:
                handle_button_release(&ev.xbutton); break;
			case KeyPress:
				handle_key_press(&ev.xkey); break;
			case CreateNotify:
				handle_window_creation(&ev.xcreatewindow); break;
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
		/* we "remember" the position of the pointer at the beginning of
		 * our move/resize, and the size/position of the window.  that way,
		 * when the pointer moves, we can compare it to our initial data
		 * and move/resize accordingly.
		 */
		//XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
		//start = ev.xbutton;
}

static void handle_button_release(XButtonEvent *e) {
	pointerorigin.subwindow = None;
	//start.subwindow = None;
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
	*/

	Window focusedwindow;
	int revert_to_return;

	XGetInputFocus(dpy, &focusedwindow, &revert_to_return);
	printf("initial focused window: %lx\n", focusedwindow);

	wn = wn->next;
	Window firstwindow = wn->id;
	while (wn->next != NULL) {
		if (wn->id == focusedwindow) {
			wn = wn->next;
			XSetInputFocus(dpy, wn->id, RevertToParent, CurrentTime);
			XRaiseWindow(dpy, wn->id);
			return;
		}
		wn = wn->next;
	}
	XSetInputFocus(dpy, firstwindow, RevertToParent, CurrentTime);
	XRaiseWindow(dpy, wn->id);
}


static void destroy_active_window() {
	Window focusedwindow;
	int revert_to_return;
	XGetInputFocus(dpy, &focusedwindow, &revert_to_return);
	XDestroyWindow(dpy, focusedwindow);
}


//use xev to find correct keysyms
static void handle_key_press(XKeyEvent *e) {
	//if(ev.xkey.subwindow != None) { 
	switch(e->keycode) {
		case 36: // enter
			spawn_terminal(); break;
		case 38: // "a"
			printf("a\n");
			focus_color(0);
			break;
		case 39: // "s"
			printf("s\n");
			focus_color(1);
			break;
		case 40: // "d"
			printf("d\n");
			focus_color(2);
			break;
		case 41: // "f"
			printf("f\n");
			focus_color(3);
			break;
		case 24: // "q"
			printf("q\n");
			destroy_active_window();
	}

	/*if(e->window != None) { 
		spawn_terminal();
	}*/

        /* this is our keybinding for raising windows.  as i saw someone
         * mention on the ratpoison wiki, it is pretty stupid; however, i
         * wanted to fit some sort of keyboard binding in here somewhere, and
         * this was the best fit for it.
         *
         * i was a little confused about .window vs. .subwindow for a while,
         * but a little RTFMing took care of that.  our passive grabs above
         * grabbed on the root window, so since we're only interested in events
         * for its child windows, we look at .subwindow.  when subwindow ==
         * None, that means that the window the event happened in was the same
         * window that was grabbed on -- in this case, the root window.
         */
}

//give the window a border and border colour
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

static void add_to_windowlist(Window w, int windowcolor) {
	WindowNode *wn = &windowlist[windowcolor];
	while (wn->next != NULL) {
		wn = wn->next;
	}

	WindowNode *newnode = malloc(sizeof(*newnode));
	newnode->id = w;
	//newnode->color = windowcolor;
	newnode->next = NULL;
	wn->next = newnode;
}

static void handle_window_creation(XCreateWindowEvent *e) {
	
	//find rarest color
	int minvalue = colortracker[0];
	int mincolor = 0;
	for (int i=1; i<NUMCOLORS; i++) {
		if (colortracker[i] < minvalue) {
			minvalue = colortracker[i];
			mincolor = i;
		}
	}
	//printf("adding window: %lx, address: %lx\n", e->window, &e->window);


	//update tracker
	colortracker[mincolor] = colortracker[mincolor] + 1;

	//add window to relevant linked list
	printf("created window id: %lx\n", e->window);
	add_to_windowlist(e->window, mincolor);

	//set border 
	XSetWindowBorderWidth(e->display, e->window, 10);
	XSetWindowBorder(e->display, e->window, color_to_pixel_value(mincolor));

	//focus new window
	XMapWindow(dpy, e->window);
	XSetInputFocus(dpy, e->window, RevertToParent, CurrentTime);
}


/* we only get motion events when a button is being pressed,
 * but we still have to check that the drag started on a window */
static void handle_motion(XMotionEvent *e) {
	/*
	if(start.subwindow != None) {
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
		int xdiff = ev.xbutton.x_root - start.x_root;
		int ydiff = ev.xbutton.y_root - start.y_root;
		XMoveResizeWindow(dpy, start.subwindow,
			attr.x + (start.button==1 ? xdiff : 0),
			attr.y + (start.button==1 ? ydiff : 0),
			MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
			MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
	}
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

//removes deleted window from linkedlist and colortracker
static void handle_window_destruction(XDestroyWindowEvent *e) {
	//TODO: fix the "not checking last node" problem properly
	printf("destruction window id: %lx\n", e->window);

	int bordercolor = -1;
	int windowfound = 0;
	WindowNode *wn;

	//find and remove window from linked lists
	for (int i=0; i<NUMCOLORS && !windowfound; i++) {
		wn = &windowlist[i];

		//if list empty, go to next list 
		if (wn->next == NULL) {
			continue;
		}

		while (wn->next != NULL) {
			if (wn->next->id == e->window) {
				WindowNode *wntofree = wn->next;
				bordercolor = i;
				windowfound = 1;
				/*if the node to delete has a next node, set current next to
				that node, else node to delete is last in list so can set current
				next to NULL */
				if (wn->next->next != NULL) {
					wn->next = wn->next->next;
				} else {
					wn->next = NULL;
				}
				free(wntofree);
				break;
			}
			wn = wn->next;
		}
	}

	//remove window from colortracker
	colortracker[bordercolor] = colortracker[bordercolor] - 1;
}



static int initialise() {

    /* we use DefaultRootWindow to get the root window, which is a somewhat
     * naive approach that will only work on the default screen.  most people
     * only have one screen, but not everyone.  if you run multi-head without
     * xinerama then you quite possibly have multiple screens. (i'm not sure
     * about vendor-specific implementations, like nvidia's)
     *
     * many, probably most window managers only handle one screen, so in
     * reality this isn't really *that* naive.
     *
     * if you wanted to get the root window of a specific screen you'd use
     * RootWindow(), but the user can also control which screen is our default:
     * if they set $DISPLAY to ":0.foo", then our default screen number is
     * whatever they specify "foo" as.
     */

    /* you could also include keysym.h and use the XK_F1 constant instead of
     * the call to XStringToKeysym, but this method is more "dynamic."  imagine
     * you have config files which specify key bindings.  instead of parsing
     * the key names and having a huge table or whatever to map strings to XK_*
     * constants, you can just take the user-specified string and hand it off
     * to XStringToKeysym.  XStringToKeysym will give you back the appropriate
     * keysym or tell you if it's an invalid key name.
     *
     * a keysym is basically a platform-independent numeric representation of a
     * key, like "F1", "a", "b", "L", "5", "Shift", etc.  a keycode is a
     * numeric representation of a key on the keyboard sent by the keyboard
     * driver (or something along those lines -- i'm no hardware/driver expert)
     * to X.  so we never want to hard-code keycodes, because they can and will
     * differ between systems.
     */

    if(!(dpy = XOpenDisplay(0x0))) {
		return 0;
	}

	//XSetErrorHandler(handle_xerror);

	//initialise the linked lists and colortracker
	for (int i=0;i<NUMCOLORS;i++) {
		colortracker[i] = 0;
		windowlist[i].id = -1;
		windowlist[i].next = NULL;
	}

	//make all children of root give out notify events
	XSelectInput (dpy, RootWindow(dpy, DefaultScreen(dpy)), SubstructureNotifyMask);    
	XSetInputFocus(dpy, RootWindow(dpy, DefaultScreen(dpy)), RevertToNone, CurrentTime);

	//grab keys needed for the wm
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





    /* XGrabKey and XGrabButton are basically ways of saying "when this
     * combination of modifiers and key/button is pressed, send me the events."
     * so we can safely assume that we'll receive Alt+F1 events, Alt+Button1
     * events, and Alt+Button3 events, but no others.  You can either do
     * individual grabs like these for key/mouse combinations, or you can use
     * XSelectInput with KeyPressMask/ButtonPressMask/etc to catch all events
     * of those types and filter them as you receive them.
     */
    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    pointerorigin.subwindow = None;
	return 1;
}

/* --- UTILS --- */

int handle_xerror(Display *dpy, XErrorEvent *e) {
	return 0;
}

//future TODO ggVGs/Mod1Mask/Mod1Mask/g
//OLD DESTRUCTION CODE
//
/*		while (wn->next != NULL) {
			if (wn->id == e->window) {
				bordercolor = i;
				//printf("window to delete found, bordercolor: %d\n", bordercolor);
				windowfound = 1;
			}
			wn = wn->next;
		}
		//check last node in list -- ugly, but works
		if (wn->id == e->window) {
			bordercolor = i;
			//printf("window to delete found here, bordercolor: %d\n", bordercolor);
			windowfound = 1;
		}
*/
