// all workspace-related functions can be found below
#include "polychrome.h" 

void switchToWorkspace(int workspaceNum) {
	// don't do anything if switching to same workspace
	if (workspaceNum == currentWorkspace) {
		return;
	}

	Client *c;

	// hide all windows in current workspace
	for (int i=0; i<NUMCOLORS; i++) {
		c = &CWS.clientList[i];

		while (c->next != NULL) {
			unmap(c->next->id);
			c = c->next;
		}
	}

	currentWorkspace = workspaceNum;

	// show all windows in new workspace
	for (int i=0; i<NUMCOLORS; i++) {
		c = &CWS.clientList[i];

		while (c->next != NULL) {
			map(c->next->id);
			c = c->next;
		}
	}
}
