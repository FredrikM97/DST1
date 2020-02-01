#include "project.h"

void main(){
	init();

	while(1){ // Main loop
		
		execute();
		
/*******************************************************************************
*						Interupt related stuff after this point							 *
*******************************************************************************/
		
		handle();
	}
}

