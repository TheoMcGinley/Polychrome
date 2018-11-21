// all hiding/reshowing functions can be found below
#include "polychrome.h" 
#include <X11/Xmd.h> //needed for CARD32
#include <stdio.h>
#include <stdlib.h>

static void setWindowState(Window, int);

// A structure to represent a stack 
static struct StackNode {
    Window hiddenWindow;
    struct StackNode* next;
};
struct StackNode;
typedef struct StackNode StackNode;
static StackNode *root = NULL;

static StackNode* newNode(Window win) {
    StackNode* stackNode =
              (StackNode*) malloc(sizeof(struct StackNode));
    stackNode->hiddenWindow = win;
    stackNode->next = NULL;
    return stackNode;
}
 
static int isEmpty(struct StackNode *root) {
    return !root;
}
 
static void push(StackNode** root, Window win) {
    StackNode* stackNode = newNode(win);
    stackNode->next = *root;
    *root = stackNode;
}
 
static Window pop(struct StackNode** root) {
    if (isEmpty(*root))
        return 0;
    StackNode* temp = *root;
    *root = (*root)->next;
    int popped = temp->hiddenWindow;
	// TODO do we want to free temp?
    // free(temp); 
    return popped;
}
 
static int peek(StackNode* root) {
    if (isEmpty(root))
        return 0;
    return root->hiddenWindow;
}

// SHAMELESSLY STOLEN FROM AEWM
//TODO push window to stack
//TODO add these functions to polychrome
void hideFocusedWindow() {
	// don't hide root window!
	if (focused == NULL) {
		return;
	}
	hide(focused->id);
}

void hide(Window win) {
	printf("Hiding window: %lx\n", win);
	// removeWindow(win);
	XUnmapWindow(dpy, win);
    setWindowState(win, IconicState);
	push(&root, win);
}

void showNextHidden() {
	if (isEmpty(root)) {
		return;
	}
	Window win = pop(&root);
	printf("Showing window: %lx\n", win);
	XMapWindow(dpy, win);
	setWindowState(win, NormalState);
}

static void setWindowState(Window win, int state) {
    CARD32 data[2];

    data[0] = state;
    data[1] = None; /* Icon? We don't need no steenking icon. */

    XChangeProperty(dpy, win, wm_state, wm_state,
        32, PropModeReplace, (unsigned char *)data, 2);
}
