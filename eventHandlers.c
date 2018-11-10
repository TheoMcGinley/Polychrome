#include "polychrome.h"

static void handleButtonPress(XButtonEvent *);
static void handleButtonRelease(XButtonEvent *);
static void handleKeyPress(XKeyEvent *);
static void handleUnmapEvent(XUnmapEvent *);
static void handleWindowDestruction(XDestroyWindowEvent *);
static void handleMotion(XMotionEvent *);
static void handleWindowMap(XMapEvent *);
static void handleClientMessage(XClientMessageEvent *);
static void removeWindow(Window);

void handleEvents() {
	XEvent ev;
    for(;;) {

        XNextEvent(dpy, &ev);

		switch (ev.type) {
            case ButtonPress:
                handleButtonPress(&ev.xbutton); break;
            case ButtonRelease:
                handleButtonRelease(&ev.xbutton); break;
			case KeyPress:
				handleKeyPress(&ev.xkey); break;
			case MapNotify:
				handleWindowMap(&ev.xmap); break;
            case UnmapNotify:
                handleUnmapEvent(&ev.xunmap); break;
			case DestroyNotify:
				handleWindowDestruction(&ev.xdestroywindow); break;
			case MotionNotify:
				handleMotion(&ev.xmotion); break;
            case ClientMessage:
                handleClientMessage(&ev.xclient); break;
            case ConfigureRequest:
                printf("ConfigureRequest received\n"); break;
			case Expose:
				printf("Expose received\n"); break;

			default:
				//printf("some janky event detected: %d\n", ev.type);
				break;
		}

    }
}

static void handleButtonPress(XButtonEvent *e) {
	if (e->subwindow != None) {
		XGetWindowAttributes(dpy, e->subwindow, &attr);
		pointerorigin = *e;
	}
}

static void handleButtonRelease(XButtonEvent *e) {
	pointerorigin.subwindow = None;
}

static void printStatus() {
	printf("focused.id: %lx,\n", focused->id);
}

//use xev to find correct keysyms
static void handleKeyPress(XKeyEvent *e) {
	//SHIFT + MOD
	if ((e->state & (ShiftMask|Mod1Mask)) == (ShiftMask|Mod1Mask)) {
		switch(e->keycode) {
			case 42: // Shift+g
				halveFocusedSize();
				return;
			case 43: // shift+h
				doubleFocusedSize();
				return;
			case 65: //shift+space
				showNextHidden();
		}
	}
	
	//MOD 

	if ((e->state & (Mod1Mask)) == (Mod1Mask)) {
		switch(e->keycode) {
			case 36: // enter
				startApp("urxvt");
				break;
			case 38: // "a"
				focusColor(0);
				break;
			case 39: // "s"
				focusColor(1);
				break;
			case 40: // "d"
				focusColor(2);
				break;
			case 41: // "f"
				focusColor(3);
				break;
			case 42: // "g"
				decrementFocusedSize();
				break;
			case 43: // "h"
				incrementFocusedSize();
				break;
			case 24: // "q"
				destroyFocusedClient();
				break;
			case 25: // "w"
				setNewWindowDimensions(WIDE);
				break;
			case 26: // "w"
				setNewWindowDimensions(EXTRA);
				break;
			case 27: // "r"
				//start_app("java -jar ~/vlt/RuneLite.jar");
				setNewWindowDimensions(REGULAR);
				break;
			case 33: // "p"
				//start_app("scrot -u ~/pix/scrots/%Y-%m-%d-%T-screenshot.png");
				setNewWindowDimensions(PORTRAIT);
				//print_status();
				break;
			case 52: // "z"
				startApp("mpc toggle -q");
				break;
			case 53: // "x"
				startApp("mpc next -q");
				break;
			case 54: // "c"
				startApp("nextalbum");
				break;
			case 56: // "b"
				startApp("firefox");
				break;
			case 65:
				hideFocusedWindow();
				break;
		}
	}
}

static void handleWindowMap(XMapEvent *e) {
	
	//don't retrack an existing window
	if (windowExists(e->window)) {
		//set_wm_state(e->window, NormalState)
		return;
	}

	addNewWindow(XMapEvent *e);

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

	//set border of old focus back to regular color
	resetFocusedBorder();

	// clientDimensions.x refers to width, clientDimensions.y refers to height
	struct Position clientDimensions = getNewWindowDimensions();

	/* find the best position for the window using the scoring system, then move
	it there, accounting for border thickness */
	struct Position pos = findBestPosition(clientDimensions.x, clientDimensions.y);
	XMoveResizeWindow(dpy, e->window, (pos.x*CELLWIDTH)+BORDERTHICKNESS,
			(pos.y*CELLHEIGHT)+BORDERTHICKNESS, (clientDimensions.x*CELLWIDTH)-(2*BORDERTHICKNESS),
			(clientDimensions.y*CELLHEIGHT)-(2*BORDERTHICKNESS));
	printf("x: %d, y: %d\n", pos.x, pos.y);

	//populate_grid
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ( i >= pos.x && i < (pos.x+clientwidth) && 
			  	 j >= pos.y && j < (pos.y+clientheight)) {
					grid[i][j] += 1;
			}
		}
	}

	//add window to relevant linked list
	Client *c = addToClientlist(e->window, pos, clientwidth, clientheight, mincolor);

	//print grid
	for (int i=0; i<16; i++) {
		for (int j=0; j<16; j++) {
			printf("%d ", grid[j][i]);
		}
		printf("\n");
	}

	focus_client(c);
	//TODO make this a setting
	newdimensions = REGULAR;
}

/* we only get motion events when a button is being pressed,
 * but we still have to check that the drag started on a window */
static void handleMotion(XMotionEvent *e) {
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

static void handleWindowDestruction(XDestroyWindowEvent *e) {
	if (windowExists(e->window))
		removeWindow(e->window);
}

static void handleUnmapEvent(XUnmapEvent *e) {
	if (windowExists(e->window))
		removeWindow(e->window);
}

// All that is required by ICCM is iconify (hide)
static void handleClientMessage(XClientMessageEvent *e) {
	printf("PRETENDING TO HIDE\n");
    /*client_t *c = find_client(e->window, MATCH_WINDOW);*/

    if (e->message_type == wm_change_state && 
			e->format == 32 && e->data.l[0] == IconicState) {
		hide(e->window);
	}
}


