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
        printf("Maximum Physical: %d Bytes\n", MAX_MEM_SIZE - sizeof(struct regions));
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
        printf("Maximum Boundary: %d Bytes\n", MAX_MEM_SIZE - sizeof(struct regions));
        return 1;
    }

    printf("Allocating static memory.\n");
    printf("Memory Size: %d, Boundary Size: %d\n", psize, bsize);


    // Creating the unique key from id and path
    key_t key = ftok(shared_filepath, SHARED_ID);

    // creating shared memory and getting identifier
    int shmid = shmget(key, MAX_MEM_SIZE, 0666 | IPC_CREAT);

    // attaching to shared memory
    struct regions *memory = (struct regions*)shmat(shmid, (void*)0, 0);

    memory->active_manager = 1;

    // Input size info
    memory->allocated_size = psize;
    memory->boundary_size = bsize;

    // defaults
    memory->current_clients = 0;
    memory->max_requests = psize / bsize;


    char input[50];

    scanf("%49[^\n]%*1[\n]", input);
    if (input[1] != ' ') {
        input[2] = '\0';
    }
    input[1] = '\0';
    while (strcmp(input, commands.exit) != 0) {
        if (strcmp(input, commands.dump) == 0) {
            printf("TODO: implement dump\n");
            if (strlen(&input[2]) > 0) {
                printf("Filename: %s\n", &input[2]);
            }
        }

        if (strcmp(input, commands.display) == 0) {
            printf("TODO: implement display\n");
            if (strlen(&input[2]) > 0) {
                printf("Filename: %s\n", &input[2]);
            }
        }

        scanf("%49[^\n]%*1[\n]", input);
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
