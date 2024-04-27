#include <stdio.h>
#include <string.h>
#include "mms.h"

int main() {
    printf("Initializing shared memory...\n");
    if (shared_mem_init() == 1) {
        printf("Error initializing shared memory, make sure manager is running\n");
        return 1;
    }
    
    int err;
    char* my_ptr_1 = mms_malloc(10, &err);
    if (err == 100) {
        printf("Out of memory\n");
        return 1;
    }
    char* my_ptr_2 = mms_malloc(8, &err);
    if (err == 100) {
        printf("Out of memory\n");
        return 1;
    }
    char* my_ptr_3 = mms_malloc(8, &err);
    if (err == 100) {
        printf("Out of memory\n");
        return 1;
    }



    for (char i = 0; i < 10; i++) {
        my_ptr_1[i] = 0xA0 + i;
    }
    for (char i = 0; i < 8; i++) {
        my_ptr_2[i] = 0xB0 +i;
    }
    for (char i = 0; i < 8; i++) {
        my_ptr_3[i] = 0xC0 + i;
    }

    err = mms_memset(my_ptr_1, 'H', 10);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    err = mms_memset(my_ptr_2 + 4, 'B', 3);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    err = mms_memset(my_ptr_1 - 3, 'E', 10);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    err = mms_memset(my_ptr_1 + 7, 'E', 10);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


}




