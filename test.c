#include <stdio.h>
#include "mms.h"

int main() {
    printf("Initializing shared memory...\n");
    if (shared_mem_init() == 1) {
        printf("Error initializing shared memory, make sure manager is running\n");
        return 1;
    }

    int err_code;
    printf("%d\n", mms_malloc(0, &err_code));


}




