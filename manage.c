#include "polychrome.h"

void addNewWindow(XMapEvent *e) {

	// find rarest color for the workspace and update the color tracker
	int borderColor = rarestColour();
	CWS.colorTracker[borderColor] = CWS.colorTracker[borderColor] + 1;

	XSetWindowBorderWidth(e->display, e->window, BORDERTHICKNESS);

	IntTuple clientDimensions = getNewWindowDimensions();

	/* find the best position for the window using the scoring system, then move
	it there, accounting for border thickness */
	IntTuple clientPosition = findBestPosition(clientDimensions);

	XMoveResizeWindow(dpy, e->window,
			(clientPosition.x * CELLWIDTH)  + BORDERTHICKNESS,
			(clientPosition.y * CELLHEIGHT) + BORDERTHICKNESS,
			(clientDimensions.x * CELLWIDTH)  - (2 * BORDERTHICKNESS), 
			(clientDimensions.y * CELLHEIGHT) - (2 * BORDERTHICKNESS));


	//add window to relevant color's linked list
	Client *c = addToClientList(e->window, clientPosition, clientDimensions, borderColor);

	updateGrid(clientPosition, clientDimensions, ADD, currentWorkspace);
	printGrid();
	focusClient(c);

	//TODO make this a setting
	newDimensions = REGULAR;
}

// if client acknowledges ICCCM's WM_DELETE_WINDOW, close it nicely, else KILL
// Calling this will in turn call an unmap and/or window destruction, removing
// it from the grid
void destroyFocusedClient() {
	//please don't try and kill the root window
	if (CWS.focused == NULL) return;

    int i, n, found = 0;
    Atom *protocols;

    if (XGetWMProtocols(dpy, CWS.focused->id, &protocols, &n)) {
        for (i=0; i<n; i++) if (protocols[i] == wm_delete) found++;
        XFree(protocols);
    }
    if (found) {
		//from: https://nachtimwald.com/2009/11/08/sending-wm_delete_window-client-messages/
		XEvent ev;
		ev.xclient.type = ClientMessage;
		ev.xclient.window = CWS.focused->id;
		ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, CWS.focused->id, False, NoEventMask, &ev);	
	} else {
		XKillClient(dpy, CWS.focused->id);
	}
}


Client*  addToClientList(Window win, IntTuple position, IntTuple dimensions, int color) {

	// go to the end of the relevant clientList
	Client *c = &CWS.clientList[color];
	while (c->next != NULL) {
		c = c->next;
	}

	// create the new client, attach it to the end of the list and return it
	Client *newClient = malloc(sizeof(*newClient));
	newClient->id = win;
	newClient->position = position;
	newClient->dimensions = dimensions;
	newClient->color = color;
	newClient->next = NULL;
	c->next = newClient;
	return newClient;
}


//removes deleted window from linkedlist and colortracker
void removeWindow(Window win) {

	if (win == CWS.focused->id) {
		focusUnfocusedClient();
	}

	int windowFound = 0;
	Client *c;
	Client *removedClient = NULL;
	int workspaceToRemoveFrom = -1;

	//find and remove the client from the relevant linked list
	for (int j=0; j<NUMWORKSPACES; j++) {
		for (int i=0; i<NUMCOLORS && !windowFound; i++) {
			c = &workspace[j].clientList[i];

			//if list empty, go to next list 
			if (c->next == NULL) {
				continue;
			}

			while (c->next != NULL) {
				if (c->next->id == win) {
					removedClient = c->next;
					windowFound = 1;
					workspaceToRemoveFrom = j;
					/*if the node to delete has a next node, set current next to
					that node, else node to delete is last in list so can set current
					next to NULL */
					if (c->next->next != NULL) {
						c->next = c->next->next;
					} else {
						c->next = NULL;
					}

					//remove window from colortracker
					workspace[j].colorTracker[i] = workspace[j].colorTracker[i] - 1;
					break;
				}
				c = c->next;
			}
		}
	}

	if (removedClient != NULL) {
		updateGrid(removedClient->position, removedClient->dimensions, REMOVE, workspaceToRemoveFrom);
		free(removedClient);
	}
}
