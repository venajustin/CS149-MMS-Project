#include <stdio.h>
#include <string.h>
#include "mms.h"

int main(int argc, char **argv) {


    printf("Initializing shared memory...\n");
    if (shared_mem_init(argv[0]) == 1) {
        printf("Error initializing shared memory, make sure manager is running\n");
        return 1;
    }
    
    int err;

    char *regions[10];

    printf("allocating 3 regions of 20 bytes\n");
    
    for (int i = 0 ; i < 3; i++) {
        regions[i] = mms_malloc(10, &err);
         if (err !=0 )
            printf("error malloc");
        err = mms_memset(regions[i], '0' + i, 10);
   } 

    printf("Printing region 2:\n    ");
    err = mms_print(regions[2], 10);


    printf("deleting second region\n");
    err = mms_free(regions[1]);
    if (err != 0) {
        printf("error\n");
    }

    printf("allocating another region of size 10\n");
    regions[4] = mms_malloc(10, &err);
     if (err !=0 )
        printf("error malloc");
     err = mms_memset(regions[4], '4', 10);
    return 0;

}
