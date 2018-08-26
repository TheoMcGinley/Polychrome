#ifndef POLYCHROME_H
#define POLYCHROME_H

#include <X11/Xlib.h>
#include <stdio.h> //printf
#include <unistd.h> //NULL, exit, fork, sleep
#include <stdlib.h> //NULL, malloc, free, exit, system
#include <paths.h>

// DEFINES }}}

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
#define CELLWIDTH 240
#define CELLHEIGHT 135
#define BORDERTHICKNESS 10


// END_DEFINES }}}

// STRUCTS {{{

struct FocusedWindow {
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

enum SpawnConfig {REGULAR, PORTRAIT, WIDE, LARGE};


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
extern WindowNode windowlist[NUMCOLORS];
extern int grid[GRIDWIDTH][GRIDHEIGHT];
extern int nextsize;
extern int nextorientation;

extern Atom wm_state;
extern Atom wm_change_state;
extern Atom wm_protos;
extern Atom wm_delete;



// END_GLOBALS }}}

// FUNCTIONS {{
// events.c
extern void event_loop(void);

// utils.c 
extern void start_app(const char *);
extern int color_to_pixel_value(int);
extern void reset_focused_border(void);
extern void focus_window(Window, int);
extern void focus_new_window(void);
extern void focus_color(int);
extern void destroy_active_window(void);
extern void add_to_windowlist(Window, struct Position, int, int, int);
//unused?
extern int window_exists(Window);
extern int handle_xerror(Display *, XErrorEvent *);

//scoring.c
//extern double calculate_score(int[][], int, int, int, int);
//extern double calculate_score(int**, int, int, int, int);
extern double calculate_score(int[GRIDWIDTH][GRIDHEIGHT], int, int, int, int);
extern struct Position find_best_position (int, int);
// END_FUNCTIONS }}
#endif // POLYCHROME_H
