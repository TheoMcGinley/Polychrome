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
	printf("focused.id: %lx,\n", focused->id);
}

//use xev to find correct keysyms
static void handle_key_press(XKeyEvent *e) {
	//SHIFT + MOD
	if ((e->state & (ShiftMask|Mod1Mask)) == (ShiftMask|Mod1Mask)) {
		switch(e->keycode) {
			case 42: // Shift+g
				halve_focused_size();
				return;
			case 43: // shift+h
				double_focused_size();
				return;
			case 65: //shift+space
				show_active_window();
		}
	}
	
	//MOD 

	if ((e->state & (Mod1Mask)) == (Mod1Mask)) {
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
			case 42: // "g"
				decrement_focused_size();
				break;
			case 43: // "h"
				increment_focused_size();
				break;
			case 24: // "q"
				destroy_focused_client();
				break;
			case 25: // "w"
				set_new_window_dimensions(WIDE);
				break;
			case 26: // "w"
				set_new_window_dimensions(EXTRA);
				break;
			case 27: // "r"
				//start_app("java -jar ~/vlt/RuneLite.jar");
				set_new_window_dimensions(REGULAR);
				break;
			case 33: // "p"
				//start_app("scrot -u ~/pix/scrots/%Y-%m-%d-%T-screenshot.png");
				set_new_window_dimensions(PORTRAIT);
				//print_status();
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
			case 65:
				hide_active_window();
				break;
		}
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

	int clientwidth, clientheight;
	switch (newdimensions) {
		case REGULAR:
			clientwidth = 4;
			clientheight = 4;
			break;
		case PORTRAIT:
			clientwidth = 4;
			clientheight = 8;
			break;
		case WIDE:
			clientwidth = 8;
			clientheight = 4;
			break;
		case EXTRA:
			clientwidth = 8;
			clientheight = 8;
			break;
	}
	/* find the best position for the window using the scoring system, then move
	it there, accounting for border thickness */
	struct Position pos = find_best_position(clientwidth, clientheight);

	XMoveResizeWindow(dpy, e->window, (pos.x*CELLWIDTH)+BORDERTHICKNESS,
			(pos.y*CELLHEIGHT)+BORDERTHICKNESS, (clientwidth*CELLWIDTH)-(2*BORDERTHICKNESS),
			(clientheight*CELLHEIGHT)-(2*BORDERTHICKNESS));
	printf("x: %d, y: %d\n", pos.x, pos.y);

	//populate_grid
	//TODO fix this?? looks wrong??
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ( i >= pos.x && i < (pos.x+clientwidth) && 
			  	 j >= pos.y && j < (pos.y+clientheight)) {
					grid[i][j] += 1;
			}
		}
	}

	//add window to relevant linked list
	Client *c = add_to_clientlist(e->window, pos, clientwidth, clientheight, mincolor);

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

	if (win == focused->id) {
		focus_new_client();
	}

	int windowfound = 0;
	Client *c;
	Client *clienttofree = NULL;

	//find and remove client from linked lists
	for (int i=0; i<NUMCOLORS && !windowfound; i++) {
		c = &clientlist[i];

		//if list empty, go to next list 
		if (c->next == NULL) {
			continue;
		}

		while (c->next != NULL) {
			if (c->next->id == win) {
				clienttofree = c->next;
				windowfound = 1;
				/*if the node to delete has a next node, set current next to
				that node, else node to delete is last in list so can set current
				next to NULL */
				if (c->next->next != NULL) {
					c->next = c->next->next;
				} else {
					c->next = NULL;
				}
				//remove window from colortracker
				colortracker[i] = colortracker[i] - 1;
				break;
			}
			c = c->next;
		}
	}

	if (clienttofree != NULL) {

		for (int i=0; i<GRIDWIDTH; i++) {
			for (int j=0; j<GRIDHEIGHT; j++) {
				if ( i >= clienttofree->pos.x && i < (clienttofree->pos.x + clienttofree->width) && 
					 j >= clienttofree->pos.y && j < (clienttofree->pos.y + clienttofree->height)) {
						grid[i][j] -= 1;
				}
			}
		}
		free(clienttofree);
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


