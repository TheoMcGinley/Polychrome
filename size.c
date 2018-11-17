// all size/resizing functions can be found below
#include "polychrome.h" 

void setNewWindowDimensions(int dimension) {
	switch (dimension) {
		case REGULAR: 	newDimensions = REGULAR; break;
		case PORTRAIT: 	newDimensions = PORTRAIT; break;
		case WIDE: 		newDimensions = WIDE; break;
		case EXTRA: 	newDimensions = EXTRA; break;
	}
}

// x refers to width, y refers to height
// TODO ensure this works
IntTuple getNewWindowDimensions() {
	IntTuple dimensions;
	switch (newDimensions) {
		case REGULAR:
			dimensions.x = 4;
			dimensions.y = 4;
			break;
		case PORTRAIT:
			dimensions.x = 4;
			dimensions.y = 8;
			break;
		case WIDE:
			dimensions.x = 8;
			dimensions.y = 4;
			break;
		case EXTRA:
			dimensions.x = 8;
			dimensions.y = 8;
			break;
	}
	return dimensions;

}

void incrementFocusedSize() {
	//find areas of grid where new client will be and increment them
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ((i == (focused->position.x + focused->dimensions.x) && 
					 j >= focused->position.y && j <= focused->position.y + focused->dimensions.y) || 
					(j == (focused->position.y + focused->dimensions.y) && 
					 i >= focused->position.x && i <= focused->position.x + focused->dimensions.y)) {
				grid[i][j] += 1;
			}
		}
	}
	XResizeWindow(dpy, focused->id, ((focused->dimensions.x)+1) * CELLWIDTH - 2 * BORDERTHICKNESS,
			((focused->dimensions.y)+1) * CELLHEIGHT - 2 * BORDERTHICKNESS);
	focused->dimensions.x = focused->dimensions.x + 1;
	focused->dimensions.y = focused->dimensions.y + 1;
}

void decrementFocusedSize() {

	if (focused->dimensions.x == 1 || focused->dimensions.y == 1) return;

	//find areas of grid where newly created space will be and decrement them
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ((i == (focused->position.x + focused->dimensions.x - 1) && 
					 j >= focused->position.y && j <= (focused->position.y + focused->dimensions.y - 1)) ||
					(j == (focused->position.y + focused->dimensions.y - 1) && 
					 i >= focused->position.x && i <= (focused->position.x + focused->dimensions.y - 1))) {
				grid[i][j] -= 1;
			}
		}
	}
	XResizeWindow(dpy, focused->id, ((focused->dimensions.x)-1)*CELLWIDTH - 2*BORDERTHICKNESS,
			((focused->dimensions.y)-1) * CELLHEIGHT - 2 * BORDERTHICKNESS);

	focused->dimensions.x = focused->dimensions.x - 1;
	focused->dimensions.y = focused->dimensions.y - 1;
}

void doubleFocusedSize() {
	//increment all cells of grid newly occupied by doubling
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {

			if ((i >= (focused->position.x + focused->dimensions.x) && i < (focused->position.x + focused->dimensions.x*2) && 
						j >= focused->position.y && j < focused->position.y + focused->dimensions.y*2) ||
				(j >= (focused->position.y + focused->dimensions.y) && j < (focused->position.y + focused->dimensions.y*2) && 
						i >= focused->position.x && i < focused->position.x + focused->dimensions.x*2)) {
				grid[i][j] += 1;
			}
		}
	}

	XResizeWindow(dpy, focused->id, ((focused->dimensions.x)*2)*CELLWIDTH - 2*BORDERTHICKNESS,
			((focused->dimensions.y)*2) * CELLHEIGHT - 2 * BORDERTHICKNESS);
	focused->dimensions.x = focused->dimensions.x * 2;
	focused->dimensions.y = focused->dimensions.y * 2;
}

void halveFocusedSize() {

	//if (focused->width == 1 || focused->height == 1) return;

	printf("haha yes\n");
}




