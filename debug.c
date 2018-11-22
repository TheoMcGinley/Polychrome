#include "polychrome.h" 
#include <X11/Xmd.h>

void printGrid() {
	for (int i=0; i<16; i++) {
		for (int j=0; j<16; j++) {
			printf("%d ", workspace[currentWorkspace].grid[j][i]);
		}
		printf("\n");
	}
}
