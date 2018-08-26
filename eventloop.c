#include "polychrome.h"

static void handle_button_press(XButtonEvent *);
static void handle_button_release(XButtonEvent *);
static void handle_key_press(XKeyEvent *);
static void handle_unmap_event(XUnmapEvent *);
static void handle_window_destruction(XDestroyWindowEvent *);
static void handle_motion(XMotionEvent *);
static void handle_window_map(XMapEvent *);
static void handle_client_message(XClientMessageEvent *);
static void remove_window(Window);

void event_loop() {
	XEvent ev;
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
            case UnmapNotify:
                handle_unmap_event(&ev.xunmap); break;
			case DestroyNotify:
				handle_window_destruction(&ev.xdestroywindow); break;
			case MotionNotify:
				handle_motion(&ev.xmotion); break;
            case ClientMessage:
                handle_client_message(&ev.xclient); break;
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

static void handle_button_press(XButtonEvent *e) {
	if (e->subwindow != None) {
		XGetWindowAttributes(dpy, e->subwindow, &attr);
		pointerorigin = *e;
	}
}

static void handle_button_release(XButtonEvent *e) {
	pointerorigin.subwindow = None;
}

static void print_status() {
	printf("focused.id: %lx,\n", focused.id);
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
			//start_app("scrot -u ~/pix/scrots/%Y-%m-%d-%T-screenshot.png");
			print_status();
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

static void handle_window_map(XMapEvent *e) {
	
	//don't retrack an existing window
	if (window_exists(e->window)) {
		//set_wm_state(e->window, NormalState)
		return;
	}

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
	reset_focused_border();

	int givenwidth = 4;
	int givenheight = 4;
	/* find the best position for the window using the scoring system, then move
	it there, accounting for border thickness */
	struct Position pos = find_best_position(givenwidth, givenheight);

	XMoveResizeWindow(dpy, e->window, (pos.x*CELLWIDTH)+BORDERTHICKNESS,
			(pos.y*CELLHEIGHT)+BORDERTHICKNESS, (givenwidth*CELLWIDTH)-(2*BORDERTHICKNESS),
			(givenheight*CELLHEIGHT)-(2*BORDERTHICKNESS));
	printf("x: %d, y: %d\n", pos.x, pos.y);

	//populate_grid
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
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
			printf("%d ", grid[j][i]);
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

//removes deleted window from linkedlist and colortracker
static void handle_window_destruction(XDestroyWindowEvent *e) {
	if (window_exists(e->window))
		remove_window(e->window);
}

static void handle_unmap_event(XUnmapEvent *e) {
	if (window_exists(e->window))
		remove_window(e->window);
}

//problem: focus_new_window() focuses same window if the window to remove is
//the first
static void remove_window(Window win) {


	printf("WIN: %lx\n", win);
	/*if (win == root)
		return;*/

	if (win == focused.id) {
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
			if (wn->next->id == win) {
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

		for (int i=0; i<GRIDWIDTH; i++) {
			for (int j=0; j<GRIDHEIGHT; j++) {
				if ( i >= wntofree->pos.x && i < (wntofree->pos.x + wntofree->width) && 
					 j >= wntofree->pos.y && j < (wntofree->pos.y + wntofree->height)) {
						grid[i][j] -= 1;
				}
			}
		}
		free(wntofree);
	}

}

// All that is required by ICCM is iconify (hide)
static void handle_client_message(XClientMessageEvent *e)
{
	printf("PRETENDING TO HIDE\n");
    /*client_t *c = find_client(e->window, MATCH_WINDOW);

    if (c && e->message_type == wm_change_state &&
        e->format == 32 && e->data.l[0] == IconicState) hide(c);*/
}


