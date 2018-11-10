//if client acknowledges ICCCM's WM_DELETE_WINDOW, close it nicely, else KILL
// calling this will in turn call an unmap and/or window destruction, removing
// it from the grid
void destroyFocusedClient() {
	//please don't try and kill the root window
	if (focused == NULL) return;

	//TODO found here is always 0??
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

Client*  addToClientList(Window w, struct Position wpos, int wwidth, int wheight, int wcolor) {
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


//removes deleted window from linkedlist and colortracker
static void removeWindow(Window win) {


	printf("WIN: %lx\n", win);

	//TODO surely this could refocus the same window? check this
	if (win == focused->id) {
		focusNewClient();
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

		//de-populate grid
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
