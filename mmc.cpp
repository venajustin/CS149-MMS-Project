#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "mms.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>



const struct {
    char exit[2];
    char dump[2];
    char display[2];
} commands = {"E", "D", "M"};

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Invalid Arguments. \n");
        printf("Correct Usage: %s <physical> <boundary>\n", argv[0]);
        return 1;
    }

    int psize = atoi(argv[1]);
    if (psize < 8) {
        printf("Minimum Physical: 8 Bytes\n");
        return 1;
    }
    if (psize > MAX_MEM_SIZE - sizeof(struct regions)) {
        printf("Maximum Physical: %ld Bytes\n", MAX_MEM_SIZE - sizeof(struct regions));
        return 1;
    }


    int bsize = atoi(argv[2]);
    if (bsize % 8 != 0)
        bsize += 8 - ( bsize % 8 );

    if (bsize < 8) {
        printf("Minimum Boundary: 8 Bytes\n");
        return 1;
    }
    if (bsize > MAX_MEM_SIZE - sizeof(struct regions)) {
        printf("Maximum Boundary: %ld Bytes\n", MAX_MEM_SIZE - sizeof(struct regions));
        return 1;
    }

    printf("Allocating shared memory:\n");
    printf("Memory Size: %d, Boundary Size: %d\n", psize, bsize);


    // Creating the unique key from id and path
    key_t key = ftok(SHARED_FILEPATH, SHARED_ID);

    // creating shared memory and getting identifier
    int shmid = shmget(key, MAX_MEM_SIZE, 0666 | IPC_CREAT);

    // attaching to shared memory
    struct regions *memory = (struct regions*)shmat(shmid, (void*)0, 0);

    // used to signify that a memory manager has initialized the shared region
    memory->active_manager = 1;

    // Input size info
    memory->allocated_size = psize;
    memory->boundary_size = bsize;

    // defaults
    memory->current_clients = 0;
    memory->max_requests = psize / bsize;

    // The number of entries in the mapping table 
    memory->total_entries = 1;
    // the initial free space entry
    struct mmap_table_entry *new_entry = memory->mmap_table;
    new_entry->client_pid = 0;
    new_entry->actual_size = psize;
    new_entry->mem_offset = 0;    


    char* memory_region = (char*)memory + sizeof(struct regions);
    char* end_region = memory_region + psize;


    // Tries to read mms.log, if it doesn't exist, it creates it and writes header
    FILE * mms_log = fopen("./mms.log", "r");
    if (mms_log == NULL) {
        mms_log = fopen("./mms.log", "w");
        fprintf(mms_log, "   Timestamp    |  Program Name  |   Process ID   |   Operation\n");
        fclose(mms_log);
    }



    char input[50];

    scanf("%49[^\n]", input);
    if (input[1] != ' ') {
        input[2] = '\0';
    }
    input[1] = '\0';
    while (strcmp(input, commands.exit) != 0) {

        if (strcmp(input, commands.dump) == 0) {
            
            // Dumping the memory region into console
            char* iterator = memory_region;
            while (iterator < end_region - 32) {
                printf("%p    ", iterator);
                for (int i = 0; i < 32; i++) {
                    printf("%.2hhX ", *iterator);
                    iterator++;
                }
                printf("\n");
            }
            if (iterator < end_region) {
                printf("%p    ", iterator);
                while (iterator < end_region) {
                    printf("%.2hhX ", *iterator);
                    iterator++;
                }
                printf("\n");
            }

            // Printing to file if specified
            if (strlen(&input[2]) > 0) {
                FILE *fptr = fopen(&input[2], "w");
                char* iterator = memory_region;
                while (iterator < end_region - 32) {
                    fprintf(fptr, "%p    ", iterator);
                    for (int i = 0; i < 32; i++) {
                        fprintf(fptr, "%.2hhX ", *iterator);
                        iterator++;
                    }
                    fprintf(fptr, "\n");
                }
                if (iterator < end_region) {
                    fprintf(fptr, "%p    ", iterator);
                    while (iterator < end_region) {
                        fprintf(fptr, "%.2hhX ", *iterator);
                        iterator++;
                    }
                    fprintf(fptr, "\n");
                }
                fclose(fptr);

                printf("Filename: %s written\n", &input[2]);
            }
        }

        if (strcmp(input, commands.display) == 0) {

            printf("---------------------------------------------------------------------------\n");
            printf("PID          | Request Size | Actual Size | Client Address | Last Reference\n");
            printf("---------------------------------------------------------------------------\n");
            for (int i = 0 ; i < memory->total_entries; i++) {
                struct mmap_table_entry entry = memory->mmap_table[i];
                if (entry.client_pid != 0) {
        
                    char *ascii_time = ctime(&entry.last_reference); 

                    printf("%-12ld | %-12d | %-11d | %-12p | %s\n", 
                            entry.client_pid,
                            entry.request_size,
                            entry.actual_size,
                            entry.client_address,
                            ascii_time);
                }
            }

            if (strlen(&input[2]) > 0) {
                FILE *fptr = fopen(&input[2], "w");

                fprintf(fptr,"---------------------------------------------------------------------------\n");
                fprintf(fptr,"PID          | Request Size | Actual Size | Client Address | Last Reference\n");
                fprintf(fptr,"---------------------------------------------------------------------------\n");
                for (int i = 0 ; i < memory->total_entries; i++) {
                    struct mmap_table_entry entry = memory->mmap_table[i];
                    if (entry.client_pid != 0) {

                        char *ascii_time = ctime(&entry.last_reference); 

                        fprintf(fptr,"%-12ld | %-12d | %-11d | %-12p | %s\n", 
                                entry.client_pid,
                                entry.request_size,
                                entry.actual_size,
                                entry.client_address,
                                ascii_time);
                    }
                }

                fclose(fptr);
                printf("Filename: %s\n", &input[2]);
            }
        }
        input[0] = 'A';
        scanf("%*[\n]%49[^\n]", input);
        if (input[1] != ' ') {
            input[2] = '\0';
        }
        input[1] = '\0';
    }

    // practically useless, since memory is destroyed anyway, but if another 
    // program created shared memmory with the same bytes, we wouldn't want
    // it to think that there is an active manager.
    memory->active_manager = 0;

    //detach from memory
    shmdt(memory);

    // destroy shared memory
    shmctl(shmid, IPC_RMID, NULL);


    printf("Exiting...\n");
    return 0;
}
