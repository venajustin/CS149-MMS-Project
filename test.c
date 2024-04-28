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
    printf("alloc 10 bytes\n");
    char* my_ptr_1 = mms_malloc(10, &err);
    if (err == 100) {
        printf("Out of memory\n");
    }
    printf("alloc 8 bytes\n");
    char* my_ptr_2 = mms_malloc(8, &err);
    if (err == 100) {
        printf("Out of memory\n");
    }
    printf("alloc 8 bytes\n");
    char* my_ptr_3 = mms_malloc(8, &err);
    if (err == 100) {
        printf("Out of memory\n");
    }
    printf("alloc 300 bytes\n");
    char* my_ptr_4 = mms_malloc(300, &err);
    if (err == 100) {
        printf("Out of memory\n");
    }


    printf("filling allocated regions with 0xA0, 0xB0 and 0xC0\n");
    for (char i = 0; i < 10; i++) {
        my_ptr_1[i] = 0xA0 + i;
    }
    for (char i = 0; i < 8; i++) {
        my_ptr_2[i] = 0xB0 +i;
    }
    for (char i = 0; i < 8; i++) {
        my_ptr_3[i] = 0xC0 + i;
    }

    printf("memset zone 1 with 'H'\n");
    err = mms_memset(my_ptr_1, 'H', 10);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    printf("memset zone 2 offset 4, 3 characters set to 'B'\n");
    err = mms_memset(my_ptr_2 + 4, 'B', 3);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    printf("memset zone1 - 3 as dest, should error\n");
    err = mms_memset(my_ptr_1 - 3, 'E', 10);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    printf("memset dest: zone1 + 7, width: 10, should error\n"); 
    err = mms_memset(my_ptr_1 + 7, 'F', 10);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }


    printf("memset a single character ('G') at the end of zone 2\n");
    err = mms_memset(my_ptr_2 + 7, 'G', 1);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }

    printf("memset an offset of zone1 that should land in zone 2, undef behavior but no way around this\n");
    err = mms_memset(my_ptr_1 + 18, 'I', 1);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }

    printf("memset width of zone 3, one to the right. dest z3+1, width 8, should error\n");
    err = mms_memset(my_ptr_3 + 1, 'J', 8);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }

    printf("memset width of zone3, one to the left. dest z3-1, width 8, should error\n");
    err = mms_memset(my_ptr_3 - 1, 'K', 8);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("Invalid destination address\n");
    }

    


    printf("deleting region 2\n");
    err = mms_free(my_ptr_2);
    if (err == 104) {
        printf("Invalid memory address\n");
    }

    printf("trying to fill region 2 with 'Z', should error\n");
    err = mms_memset(my_ptr_2, 'Z', 8);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 102) {
        printf("invalid memory address\n");
    }

    

    printf("copying from region 1 to region 4\n");
    err = mms_memcpy(my_ptr_4, my_ptr_1, 8);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 103) {
        printf("invalid memory address\n");
    }

    printf("copying from invalid location to region 4\n");
    err = mms_memcpy(my_ptr_4, my_ptr_1 - 20, 8);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 103) {
        printf("invalid memory address\n");
    }
 
    printf("copying from region 1 to invalid region \n");
    err = mms_memcpy(my_ptr_4+ 400, my_ptr_1, 8);
    if (err == 101) {
        printf("Memory too small\n");
    }
    if (err == 103) {
        printf("invalid memory address\n");
    }   



}




