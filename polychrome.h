#ifndef POLYCHROME_H
#define POLYCHROME_H

// INCLUDES {{{
#include <X11/Xlib.h>
#include <stdio.h> //printf
#include <unistd.h> //NULL, exit, fork, sleep
#include <stdlib.h> //NULL, malloc, free, exit, system
#include <paths.h>
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
//#define cellwidth 240 //for 4k
//#define cellheight 135 //for 4k
#define CELLWIDTH 48 //for notebook
#define CELLHEIGHT 27 //for notebook

#define BORDERTHICKNESS 10


// END_DEFINES }}}

// STRUCTS {{{

struct Position {
	int x;
	int y;
};

struct Client {
	Window id;
	struct Position pos;
	int width;
	int height;
	int color;
	struct Client *next;
};

struct Client;
typedef struct Client Client;

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
//extern Window root;
extern XWindowAttributes attr;
extern XButtonEvent pointerorigin;
extern int colortracker[NUMCOLORS];
extern Client clientlist[NUMCOLORS];
extern Client *focused;
extern int grid[GRIDWIDTH][GRIDHEIGHT];
//extern int nextsize;
//extern int nextorientation;
extern enum NewWindowDimensions newdimensions;

extern Atom wm_state;
extern Atom wm_change_state;
extern Atom wm_protos;
extern Atom wm_delete;



// END_GLOBALS }}}

// FUNCTIONS {{{
// events.c
extern void eventLoop(void);

// hide.c
extern void hideFocusedWindow();
extern void hide(Window);
extern void showNextHidden();
extern void setWindowState(Window win, int state);

// utils.c 
extern void startApp(const char *);
extern int colorToPixelValue(int);
extern int windowExists(Window);

// size.c
extern void setNewWindowDimensions(int);
extern struct Position getNewWindowDimension();
extern void incrementFocusedSize(void);
extern void decrementFocusedSize(void);
extern void halveFocusedSize(void);
extern void doubleFocusedSize(void);

// focus.c
extern void resetFocusedBorder(void);
extern void focusClient(Client*);
extern void focusNewClient(void);
extern void focusColor(int);

// manage.c
extern void destroyFocusedClient(void);
extern Client* addToClientlist(Window, struct Position, int, int, int);

//unused?
//extern int handleXerror(Display *, XErrorEvent *);

//scoring.c
extern double calculateScore(int, int, int, int);
extern struct Position findBestPosition (int, int);
// END_FUNCTIONS }}}
#endif // POLYCHROME_H
