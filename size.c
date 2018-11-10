// all size/resizing functions can be found below
#include "polychrome.h" 

void setNewWindowDimensions(int dimension) {
	switch (dimension) {
		case REGULAR: 	newdimensions = REGULAR; break;
		case PORTRAIT: 	newdimensions = PORTRAIT; break;
		case WIDE: 		newdimensions = WIDE; break;
		case EXTRA: 	newdimensions = EXTRA; break;
	}
}

//TODO set dimensions x and y
struct Position getNewWindowDimension() {
	struct Position dimensions
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

void incrementFocusedSize() {
	//find areas of grid where new client will be and increment them
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ((i == (focused->pos.x + focused->width) && j >= focused->pos.y && j <= focused->pos.y + focused->height)
			|| (j == (focused->pos.y + focused->height) && i >= focused->pos.x && i <= focused->pos.x + focused->height)) {
				grid[i][j] += 1;
			}
		}
	}
	XResizeWindow(dpy, focused->id, ((focused->width)+1)*CELLWIDTH - 2*BORDERTHICKNESS, ((focused->height)+1)*CELLHEIGHT - 2*BORDERTHICKNESS);
	focused->width = focused->width+1;
	focused->height = focused->height+1;
}

//TODO don't allow if it makes it too small
void decrementFocusedSize() {

	if (focused->width == 1 || focused->height == 1) return;

	//find areas of grid where newly created space will be and decrement them
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ((i == (focused->pos.x + focused->width - 1) && j >= focused->pos.y && j <= (focused->pos.y + focused->height - 1))
			|| (j == (focused->pos.y + focused->height - 1) && i >= focused->pos.x && i <= (focused->pos.x + focused->height - 1))) {
				grid[i][j] -= 1;
			}
		}
	}
	XResizeWindow(dpy, focused->id, ((focused->width)-1)*CELLWIDTH - 2*BORDERTHICKNESS, ((focused->height)-1)*CELLHEIGHT - 2*BORDERTHICKNESS);

	focused->width = focused->width-1;
	focused->height = focused->height-1;
}

void doubleFocusedSize() {
	//increment all cells of grid newly occupied by doubling
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {

			if ((i >= (focused->pos.x + focused->width) && i < (focused->pos.x + focused->width*2) && 
						j >= focused->pos.y && j < focused->pos.y + focused->height*2) ||
				(j >= (focused->pos.y + focused->height) && j < (focused->pos.y + focused->height*2) && 
						i >= focused->pos.x && i < focused->pos.x + focused->width*2)) {
				grid[i][j] += 1;
			}


			/*if (i > (focused->pos.x + focused->width) && i <= (focused->pos.x + (focused->width)*2)
			&&  j > (focused->pos.y + focused->height) && j <= (focused->pos.y + (focused->height)*2)) { 
				grid[i][j] += 1;
			}*/
		}
	}

	XResizeWindow(dpy, focused->id, ((focused->width)*2)*CELLWIDTH - 2*BORDERTHICKNESS, ((focused->height)*2)*CELLHEIGHT - 2*BORDERTHICKNESS);
	focused->width = focused->width * 2;
	focused->height = focused->height * 2;
}

void halveFocusedSize() {

	if (focused->width == 1 || focused->height == 1) return;

	printf("haha yes\n");
}




