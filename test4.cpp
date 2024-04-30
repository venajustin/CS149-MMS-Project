#include <stdio.h>
#include <string.h>
#include "mms.h"

int main(int argc, char **argv) {


    printf("Initializing shared memory...\n");
    if (mms_init() == NULL) {
        printf("Error initializing shared memory, make sure manager is running\n");
        return 1;
    }
    
    int err;

    char *regions[10];

    printf("allocating a region of size 10\n");
    regions[0] = mms_malloc(10, &err);
     if (err !=0 )
        printf("error malloc");
     err = mms_memset(regions[0], '4', 10);

     printf("copying from -16 before region into region. This will throw an error unless there is another buffer located before this\n");
    err = mms_memcpy(regions[0], regions[0] - 16, 5);
    if (err !=0 ){
        printf("ERROR copying\n");
    }


    printf("reading my buffer\n");
    err = mms_print(regions[0], 10);
    if (err != 0) {
        printf("ERROR\n");
    }
    printf("reading the buffer before me\n");
    err = mms_print(regions[0] - 16, 10);
    if (err != 0) {
        printf("ERROR\n");
    }
    return 0;

}
