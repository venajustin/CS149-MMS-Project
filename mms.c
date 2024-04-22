#include "mms.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>


int shared_mem_init() {
    // generate key from filepath and id
    key_t key = ftok(shared_filepath, SHARED_ID);
    // create and/or get id of shared memory
    int shmid = shmget(key, MAX_MEM_SIZE, 0666 | IPC_CREAT);
    // map the memory to local address space
    memory = (struct regions*)shmat(shmid, (void*)0, 0);

    if (memory->active_manager != 1) {
        // Memory has not been initialized by manager script
        // unlink and destroy shared memory
        shmdt(memory);
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }
    return 0;
}

int first_byte() {
	return memory->allocated_size;
}

void set_mem(int c) {
    memory->allocated_size = c;
}
