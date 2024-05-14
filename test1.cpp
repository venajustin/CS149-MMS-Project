#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include "mms.h"

int main(int argc, char **argv) {


    printf("Initializing shared memory...\n");
    if (mms_init() == NULL) {
        printf("Error initializing shared memory, make sure manager is running\n");
        return 1;
    }
 
    printf("Type any character and Press Enter to continue, continue this to proceed through each test\n");
    char input[50];

    scanf("%s", input);
   
    int err;
    printf("alloc 10 bytes\n");
    char* my_ptr_1 = mms_malloc(10, &err);
    if (err == 100) {
        printf("ERROR: Out of memory\n");
    } else {
        for (char i = 0; i < 10; i++) {
            my_ptr_1[i] = 0xA0 + i;
        }
    }
            
    printf("alloc 8 bytes\n");
    char* my_ptr_2 = mms_malloc(8, &err);
    if (err == 100) {
        printf("ERROR: Out of memory\n");
    } else {
       for (char i = 0; i < 8; i++) {
            my_ptr_2[i] = 0xB0 +i;
        }
    }

    printf("alloc 8 bytes\n");
    char* my_ptr_3 = mms_malloc(8, &err);
    if (err == 100) {
        printf("ERROR: Out of memory\n");
   } else {
       for (char i = 0; i < 8; i++) {
            my_ptr_3[i] = 0xC0 + i;
        }
   }
 
    scanf("%s", input);

    printf("\n-> to test memory size set: \n     32 <= physical < 336\n     boundary = 8\n");

    printf("alloc 300 bytes - will error if not enough memory\n");
    char* my_ptr_4 = mms_malloc(300, &err);
    if (err == 100) {
        printf("ERROR: Out of memory\n");
    } else {
        for (int i = 0; i < 300; i++) {
            my_ptr_4[i] = 0xDD;
        }
    }

    scanf("%s", input);

    printf("\n-> testing mms_memset:\n");

    printf("memset zone 1 with 'H'\n");
    err = mms_memset(my_ptr_1, 'H', 10);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }


    printf("memset zone 2 offset 4, 3 characters set to 'B'\n");
    err = mms_memset(my_ptr_2 + 4, 'B', 3);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }


    printf("memset zone1 - 3 as dest, should error\n");
    err = mms_memset(my_ptr_1 - 3, 'E', 10);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }


    printf("memset dest: zone1 + 7, width: 10, should error\n"); 
    err = mms_memset(my_ptr_1 + 7, 'F', 10);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }


    printf("memset a single character ('G') at the end of zone 2\n");
    err = mms_memset(my_ptr_2 + 7, 'G', 1);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }

    printf("memset an offset of zone1 that should land in zone 2, undef behavior but no way around this\n");
    err = mms_memset(my_ptr_1 + 18, 'I', 1);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }

    printf("memset width of zone 3, one to the right. dest z3+1, width 8, should error\n");
    err = mms_memset(my_ptr_3 + 1, 'J', 8);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }

    printf("memset width of zone3, one to the left. dest z3-1, width 8, should error\n");
    err = mms_memset(my_ptr_3 - 1, 'K', 8);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: Invalid destination address\n");
    }

    
    scanf("%s", input);

    printf("\n-> testing mms_free:\n");

    printf("deleting region 2\n");
    err = mms_free(my_ptr_2);
    if (err == 104) {
        printf("ERROR: Invalid memory address\n");
    }

    printf("trying to fill region 2 with 'Z', should error\n");
    err = mms_memset(my_ptr_2, 'Z', 8);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 102) {
        printf("ERROR: invalid memory address\n");
    }

    printf("trying to delete region 2 again, should error\n");
    err = mms_free(my_ptr_2);
    if (err == 104) {
        printf("ERROR: Invalid memory address\n");
    }
    
    scanf("%s", input);

    printf("\n-> testing mms_memcpy:\n");

    printf("Zone 1 and zone 3: \n");
    err = mms_print(my_ptr_1, 10);
    if (err != 0 ) printf("ERROR\n");
    err = mms_print(my_ptr_3, 8);
    if (err != 0 ) printf("ERROR\n");
    printf("\n");

    printf("copying from region 1 to region 3\n");
    err = mms_memcpy(my_ptr_3, my_ptr_1, 8);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 103) {
        printf("ERROR: invalid memory address\n");
    }
    printf("region 3:\n");
    err = mms_print(my_ptr_3, 8);
    if (err != 0 ) printf("ERROR\n");
    printf("\n");

    printf("copying from invalid location to region 3\n");
    err = mms_memcpy(my_ptr_3, my_ptr_1 - 20, 8);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 103) {
        printf("ERROR: invalid memory address\n");
    }
    printf("region 3:\n");
    err = mms_print(my_ptr_3, 8);
    if (err != 0 ) printf("ERROR\n");
    printf("\n");

    printf("copying from region 1 to invalid region \n");
    err = mms_memcpy(my_ptr_1 - 3 , my_ptr_1, 8);
    if (err == 101) {
        printf("ERROR: Memory too small\n");
    }
    if (err == 103) {
        printf("ERROR: invalid memory address\n");
    }   



    // extern buffer used in memcpy and print
    char* buff = (char*) malloc(32);
    printf("region 1 and 3 reset to 0x11 and 0x33:\n");
    err = mms_memset(my_ptr_1, 0x11, 10);
    if (err != 0 ) printf("ERROR\n");
    err = mms_memset(my_ptr_3, 0x33, 8);
    if (err != 0 ) printf("ERROR\n");

    printf("copying region 1 into buffer\n");
    err = mms_memcpy(buff, my_ptr_1, 10);
    if (err == 103) printf("ERROR: invalid pointer\n");

    printf("buffer:\n");
    for (int i = 0; i < 8; i++) {
        printf("%x ", buff[i]);
    }
    printf("\n");

    
    printf("copying 8 bytes of buffer into region 3\n");
    err = mms_memcpy(my_ptr_3, buff, 8);
    if (err != 0) printf("ERROR: invalid pointer\n");
    printf("region 3:\n");
    err = mms_print(my_ptr_3, 8);
    if (err != 0 ) printf("ERROR\n");
    printf("\n");

    printf("setting 3 bytes of region 3 to  0x97\n");
    err = mms_memset(my_ptr_3 + 1, 0x97, 3);
    if (err != 0) printf("ERROR\n");
    printf("region 3:\n");
    err = mms_print(my_ptr_3, 8);
    if (err != 0 ) printf("ERROR\n");
    printf("\n");

    printf("copying 4 bytes of region 3 (from the middle) into region 1 (in the middle)\n");
    err = mms_memcpy(my_ptr_1 + 2, my_ptr_3 + 1, 4);
    if (err == 103) printf("ERROR: invalid pointer\n");
    printf("region 1:\n");
    err = mms_print(my_ptr_1, 10);
    if (err != 0 ) printf("ERROR\n");
    printf("\n");

    printf("memcpy from invalid location into my_ptr_1, should error\n");
    err = mms_memcpy(my_ptr_1, my_ptr_1 - 12, 4);
    if (err == 103) printf("ERROR: invalid pointer\n");   

    printf("memcpy from my_ptr_1 into invalid location, should error\n");
    err = mms_memcpy(my_ptr_1,my_ptr_1 + 9 , 4);
    if (err == 103) printf("ERROR: invalid pointer\n");   




    scanf("%s", input);


    printf("\n-> testing mms_print\n");

    printf("printing region 1\n");
    err = mms_print(my_ptr_1, 10);
    if (err == 103) {
        printf("ERROR: invalid pointer\n");
    }

    printf("setting region 3 byte 4 to 0x0\n");
    err = mms_memset(my_ptr_3 + 3, '\0', 1);
    if (err != 0 ) printf("ERROR\n");

    printf("printing region 3 with size = 0 (print untill 0x0)\n");
    err = mms_print(my_ptr_3, 0);
    if (err == 103) printf("ERROR: invalid source\n");

    printf("printing region 1 with size = 0 (print untill 0x0), should error\n");
    err = mms_print(my_ptr_1, 0);
    if (err == 103) printf("ERROR: invalid source\n");   

    printf("printing with size=0 from pointer in invalid space of memory mapped file, should error\n");
    err = mms_print(my_ptr_1 - 20, 0);
    if (err == 103) printf("ERROR: invalid source\n");   
     
}




