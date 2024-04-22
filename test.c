#include <stdio.h>
#include "mms.h"

int main() {
    printf("Initializing shared memory...\n");
    if (shared_mem_init() == 1) {
        printf("Error initializing shared memory, make sure manager is running\n");
        return 1;
    }

    set_mem(222);
	char readmem;
    printf("Reading first byte\n");
	readmem = first_byte();
	printf("First Char %d\n", readmem);
	return 0;


}




