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
			if ((i == (CWS.focused->position.x + CWS.focused->dimensions.x) && 
					 j >= CWS.focused->position.y && j <= CWS.focused->position.y + CWS.focused->dimensions.y) || 
					(j == (CWS.focused->position.y + CWS.focused->dimensions.y) && 
					 i >= CWS.focused->position.x && i <= CWS.focused->position.x + CWS.focused->dimensions.y)) {
				CWS.grid[i][j] += 1;
			}
		}
	}
	XResizeWindow(dpy, CWS.focused->id, ((CWS.focused->dimensions.x)+1) * CELLWIDTH - 2 * BORDERTHICKNESS,
			((CWS.focused->dimensions.y)+1) * CELLHEIGHT - 2 * BORDERTHICKNESS);
	CWS.focused->dimensions.x = CWS.focused->dimensions.x + 1;
	CWS.focused->dimensions.y = CWS.focused->dimensions.y + 1;
}

void decrementFocusedSize() {

	if (CWS.focused->dimensions.x == 1 || CWS.focused->dimensions.y == 1) return;

	//find areas of grid where newly created space will be and decrement them
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {
			if ((i == (CWS.focused->position.x + CWS.focused->dimensions.x - 1) && 
					 j >= CWS.focused->position.y && j <= (CWS.focused->position.y + CWS.focused->dimensions.y - 1)) ||
					(j == (CWS.focused->position.y + CWS.focused->dimensions.y - 1) && 
					 i >= CWS.focused->position.x && i <= (CWS.focused->position.x + CWS.focused->dimensions.y - 1))) {
				CWS.grid[i][j] -= 1;
			}
		}
	}
	XResizeWindow(dpy, CWS.focused->id, ((CWS.focused->dimensions.x)-1)*CELLWIDTH - 2*BORDERTHICKNESS,
			((CWS.focused->dimensions.y)-1) * CELLHEIGHT - 2 * BORDERTHICKNESS);

	CWS.focused->dimensions.x = CWS.focused->dimensions.x - 1;
	CWS.focused->dimensions.y = CWS.focused->dimensions.y - 1;
}

void doubleFocusedSize() {
	//increment all cells of grid newly occupied by doubling
	for (int i=0; i<GRIDWIDTH; i++) {
		for (int j=0; j<GRIDHEIGHT; j++) {

			if ((i >= (CWS.focused->position.x + CWS.focused->dimensions.x) && i < (CWS.focused->position.x + CWS.focused->dimensions.x*2) && 
						j >= CWS.focused->position.y && j < CWS.focused->position.y + CWS.focused->dimensions.y*2) ||
				(j >= (CWS.focused->position.y + CWS.focused->dimensions.y) && j < (CWS.focused->position.y + CWS.focused->dimensions.y*2) && 
						i >= CWS.focused->position.x && i < CWS.focused->position.x + CWS.focused->dimensions.x*2)) {
				CWS.grid[i][j] += 1;
			}
		}
	}

	XResizeWindow(dpy, CWS.focused->id, ((CWS.focused->dimensions.x)*2)*CELLWIDTH - 2*BORDERTHICKNESS,
			((CWS.focused->dimensions.y)*2) * CELLHEIGHT - 2 * BORDERTHICKNESS);
	CWS.focused->dimensions.x = CWS.focused->dimensions.x * 2;
	CWS.focused->dimensions.y = CWS.focused->dimensions.y * 2;
}

void halveFocusedSize() {

	//if (focused->width == 1 || focused->height == 1) return;

	printf("haha yes\n");
}
