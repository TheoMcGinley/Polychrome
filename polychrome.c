//TODO consider adding a variable for focusedwindow - easier to keep track of
//logical focus
//TODO fix improper colouring if pkill used 
//TODO fix shell_command (doesn't like some commands e.g. spaces)
//TODO proper error handling 
/*TODO force close if WM_DELETE_WINDOW doesn't go through
	see line 295 of https://github.com/i3/i3/blob/next/src/x.c */
//TODO use hints to determine window size:
//http://neuron-ai.tuke.sk/hudecm/Tutorials/C/special/xlib-programming/xlib-programming-2.html
//TODO fix shell, then make keybinds for:
	//mpc toggle
	//mpc next
	//nextalbum
	//firefox
//TODO rename .nix expressions from theowm -> polychrome

#include <X11/Xlib.h>
#include <stdio.h> //printf
#include <unistd.h> //NULL, exit, fork, sleep
#include <stdlib.h> //NULL, malloc, free, exit, system

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define NUMCOLORS 4
#define NUMBUCKETS 20
#define FOCUSCOLOR 16777111 //soft yellow
				 //16777215 // white

struct WindowNode {
	Window id;
	struct WindowNode *next;
};
struct WindowNode;
typedef struct WindowNode WindowNode;

//Polychrome Window
struct PWindow {
	Window id;
	int color;
} focusedwindow;

int handle_xerror(Display *, XErrorEvent *);

static int initialise();
static void handle_button_press(XButtonEvent *);
static void handle_button_release(XButtonEvent *);
static void handle_key_press(XKeyEvent *);
static void handle_window_creation(XCreateWindowEvent *);
static void handle_window_destruction(XDestroyWindowEvent *);
static void handle_motion(XMotionEvent *);
static void handle_window_map(XMapEvent *);


static Display * dpy;
static XWindowAttributes attr;
static XButtonEvent pointerorigin;

static int colortracker[NUMCOLORS];
static WindowNode windowlist[NUMCOLORS];
static int focusedcolor = 0;
//static Window focusedwindow; //XGetInputFocus exists, but 


void spawn_terminal() {
	pid_t pid = fork();
	if (pid == 0) {
		setsid();
		execlp("/bin/sh", "sh", "-C", "xterm", NULL);
		exit(1);
	}
}

void shell_command(char * command) {
	pid_t pid = fork();
	if (pid == 0) {
		setsid();
		printf("%s", command);
		execlp("/bin/sh", "sh", "-C", command, NULL);
		exit(1);
	}
}

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
			case CreateNotify:
				handle_window_creation(&ev.xcreatewindow); break;
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

	Window focusedwindow;
	int revert_to_return;
	XGetInputFocus(dpy, &focusedwindow, &revert_to_return);
	printf("initial focused window: %lx\n", focusedwindow);

	//colour focused window back to normal
	XSetWindowBorder(dpy, focusedwindow, color_to_pixel_value(focusedcolor));
	//as window is guarenteed to be found below, set focusedcolor to new color
	focusedcolor = color;

	wn = wn->next;
	Window firstwindow = wn->id;
	while (wn->next != NULL) {
		//scenario 1)
		if (wn->id == focusedwindow) {
			wn = wn->next;
			XRaiseWindow(dpy, wn->id);
			XSetInputFocus(dpy, wn->id, RevertToParent, CurrentTime);
			XSetWindowBorder(dpy, wn->id, FOCUSCOLOR);

			/* flash mode 
			XFlush(dpy); //ensure all events processed before sleeping
			sleep(0.7*1000);
			XSetWindowBorder(dpy, wn->id, color_to_pixel_value(color)); */
			return;
		}
		wn = wn->next;
	}

	//scenario 2)
	XRaiseWindow(dpy, firstwindow);
	XSetInputFocus(dpy, firstwindow, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, firstwindow, FOCUSCOLOR);
	/* flash mode
	XFlush(dpy); //ensure all events processed before sleeping
	sleep(0.7*1000);
	XSetWindowBorder(dpy, firstwindow, color_to_pixel_value(color)); */
}


static void destroy_active_window() {
	Window focusedwindow;
	int revert_to_return;
	XGetInputFocus(dpy, &focusedwindow, &revert_to_return);
	//XKillClient(dpy, focusedwindow);
	//XDestroyWindow(dpy, focusedwindow);

	XEvent ev;
	 
	//memset(&ev, 0, sizeof (ev));
	 
	//assumes ICCCM compliance
	//from: https://nachtimwald.com/2009/11/08/sending-wm_delete_window-client-messages/
	ev.xclient.type = ClientMessage;
	ev.xclient.window = focusedwindow;
	ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, focusedwindow, False, NoEventMask, &ev);	
}


//use xev to find correct keysyms
static void handle_key_press(XKeyEvent *e) {
	//if(ev.xkey.subwindow != None) { 
	switch(e->keycode) {
		case 36: // enter
			//spawn_terminal(); 
			//system("xterm");
			shell_command("xterm");
			break;
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
			break;
		case 52: // "z"
			printf("z");
			//system("mpc toggle");
			shell_command("mpc\ toggle");
			break;
		case 53: // "x"
			printf("x\n");
			//shell_command("mpc next");
			shell_command("qutebrowser");
			//system("qutebrowser");
			break;
		case 54: // "c"
			printf("c\n");
			shell_command("~/bin/nextalbum");
			break;
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


static void handle_window_map(XMapEvent *e) {
	
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

	XSetWindowBorderWidth(e->display, e->window, 10);
	//XSetWindowBorder(e->display, e->window, color_to_pixel_value(mincolor));

	//focus new window, setting border of old focus back to regular color
	Window focusedwindow;
	int revert_to_return;
	XGetInputFocus(dpy, &focusedwindow, &revert_to_return);
	printf("old focused window id: %lx\n", focusedwindow);
	XSetWindowBorder(dpy, focusedwindow, color_to_pixel_value(focusedcolor));

	XSetInputFocus(dpy, e->window, RevertToParent, CurrentTime);
	XSetWindowBorder(dpy, e->window, FOCUSCOLOR);
	XRaiseWindow(dpy, e->window);

	/* Flash mode
	 XFlush(dpy); //ensure all events processed before sleeping
	sleep(0.7);
	XSetWindowBorder(dpy, e->window, color_to_pixel_value(mincolor));*/

	focusedcolor = mincolor;
}



static void handle_window_creation(XCreateWindowEvent *e) {
	
	/*//find rarest color
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
	//XMapWindow(dpy, e->window);
	XRaiseWindow(dpy, e->window);
	XSetInputFocus(dpy, e->window, RevertToParent, CurrentTime);
	*/
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
			XRaiseWindow(dpy, wn->id);
			XSetInputFocus(dpy, wn->id, RevertToParent, CurrentTime);
			XSetWindowBorder(dpy, wn->id, FOCUSCOLOR);
			focusedcolor = i;
			return;
		}
	}
}

//removes deleted window from linkedlist and colortracker
static void handle_window_destruction(XDestroyWindowEvent *e) {
	printf("destruction window id: %lx\n", e->window);

	/* CURRENT: if the window had been mapped, a new window needs to be selected
	 * DESIRED: if the window was the focused window when it was destroyed, a
	 * new window needs to be selected 
	 * behaviour between CURRENT and DESIRED is mostly the same as most of the
	 * time when a window is killed it is also the current focused (killed by
	 * the program finishing/ user pressing mod+q)
	 * however, CURRENT changes the focus when kill/pkill/xkill is used, which
	 * is an unintended behaviour */
	//this shouldn't work - most of the time, the focused window is the window
	//being killed
	if (window_exists(e->window)) {
		 Window focusedwindow;
		int revert_to_return;
		XGetInputFocus(dpy, &focusedwindow, &revert_to_return);
		printf("old focused window id: %lx\n", focusedwindow);
		/*if (focusedwindow == ROOTID) {
			printf("YASSS");
		}*/
		//XSetWindowBorder(dpy, focusedwindow, color_to_pixel_value(focusedcolor));
		focus_new_window();
	}


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
				//remove window from colortracker
				colortracker[i] = colortracker[i] - 1;
				break;
			}
			wn = wn->next;
		}
	}
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
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("z")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("x")), Mod1Mask,
            DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("c")), Mod1Mask,
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
	printf("???\n");
	return 0;
}

//future TODO: replace Mod1Mask (alt) with Mod4Mask (meta)
