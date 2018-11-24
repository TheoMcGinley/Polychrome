#ifndef POLYCHROME_H
#define POLYCHROME_H

// INCLUDES {{{
#include <X11/Xlib.h>
#include <stdio.h>  // used for: printf
#include <unistd.h> // used for: NULL, exit, fork, sleep
#include <stdlib.h> // used for: NULL, malloc, free, exit, system
#include <paths.h>  // used for: ???
// END_INCLUDES }}}

// DEFINES {{{

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define NUMCOLORS 4
#define NUMBUCKETS 20
#define FOCUSCOLOR 16777111 //soft yellow
				 //16777215 // white
#define UNDEFINED -1
#define GRIDWIDTH 16 
#define GRIDHEIGHT 16 
//#define CELLWIDTH (WidthOfScreen()/GRIDSIZE) //TODO make these work
//#define CELLHEIGHT (HeightOfScreen()/GRIDSIZE)
#define CELLWIDTH 240 //for 4k
#define CELLHEIGHT 135 //for 4k
// #define CELLWIDTH 48 //for notebook
// #define CELLHEIGHT 27 //for notebook

#define NUMWORKSPACES 4 

#define CWS workspace[currentWorkspace]
#define BORDERTHICKNESS 10

// used when populating/depopulating the grid
#define ADD 1
#define REMOVE -1

// definitions for hiding - can't find where to import them from
#define WithdrawnState 0
#define NormalState 1
#define IconicState 3



// END_DEFINES }}}

// STRUCTS {{{

// used for positions and dimensions
typedef struct IntTuple {
	int x;
	int y;
} IntTuple;

struct Client {
	Window id;
	IntTuple position;
	IntTuple dimensions;
	int color;
	struct Client *next;
};

struct Client;
typedef struct Client Client;

typedef struct Workspace {
	int colorTracker[NUMCOLORS];
	Client clientList[NUMCOLORS];
	Client *focused;
	int grid[GRIDWIDTH][GRIDHEIGHT];
} Workspace;

//use QUEENLY (portrait) so that it's QWER?
enum NewWindowDimensions {REGULAR, PORTRAIT, WIDE, EXTRA};


/*
struct workspacestate {
	colortracker[NUMCOLORS];
	windowlist[NUMCOLORS];
	grid[CELLWIDTH][CELLHEIGHT];
}*/

// END_STRUCTS }}}

// GLOBALS {{{

extern Display * dpy;
extern XWindowAttributes attr;
extern XButtonEvent pointerOrigin;

extern Workspace workspace[NUMWORKSPACES];
extern int currentWorkspace;

extern enum NewWindowDimensions newDimensions;

extern Atom wm_state;
extern Atom wm_change_state;
extern Atom wm_protos;
extern Atom wm_delete;

// END_GLOBALS }}}

// FUNCTIONS {{{

// debug.c
extern void printGrid();

// eventHandlers.c
extern void handleEvents();

// focus.c
extern void focusClient(Client*);
extern void focusColor(int);
extern void focusUnfocusedClient();
extern void showNextHidden();

// hide.c
extern void map(Window);
extern void unmap(Window);
extern void hideFocusedWindow();
extern void hide(Window);
extern void showNextHidden();

// manage.c
extern void addNewWindow(XMapEvent *);
extern void destroyFocusedClient();
extern Client* addToClientList(Window, IntTuple, IntTuple, int);
extern void removeWindow(Window);

//scoring.c
extern IntTuple findBestPosition (IntTuple);

// size.c
extern void setNewWindowDimensions(int);
extern IntTuple getNewWindowDimensions();
extern void incrementFocusedSize(void);
extern void decrementFocusedSize(void);
extern void halveFocusedSize(void);
extern void doubleFocusedSize(void);

// utils.c 
extern void startApp(const char *);
extern int colorToPixelValue(int);
extern int windowExists(Window);
extern int rarestColour();
extern void updateGrid(IntTuple, IntTuple, int, int);
extern int shouldBeIgnored(Window);

// workspaces.c
extern void switchToWorkspace(int);

// END_FUNCTIONS }}}
#endif // POLYCHROME_H
