#include "mms.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>

static struct regions *memory;
static char *mem_region;
static char *end_mem;

int shared_mem_init() {
    // generate key from filepath and id
    key_t key = ftok(shared_filepath, SHARED_ID);
    // create and/or get id of shared memory
    int shmid = shmget(key, MAX_MEM_SIZE, 0666 | IPC_CREAT);
    // map the memory to local address space
    memory = (struct regions*)shmat(shmid, (void*)0, 0);
    mem_region = (char*) memory + sizeof(struct regions);
    end_mem = mem_region + memory->allocated_size;

    if (memory->active_manager != 1) {
        // Memory has not been initialized by manager script
        // unlink and destroy shared memory
        shmdt(memory);
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }
    return 0;
} 

// Allocate a piece of memory given the input size.
// If successful, returns a valid pointer. Otherwise it returns NULL (0) and set the error_code.
// Possible errors: 100
char* mms_malloc(int size, int* error_code) {
    pid_t this_pid = getpid();

    return 0;
}

// Set the destination buffer with a character of certain size.
// If successful, returns 0. Otherwise it returns an error code.
// Possible errors: 101, 102
int mms_memset(char* dest_ptr, char c, int size) {
    return 0;
}

// Copy the fixed number of bytes from source to destination.
// If successful, returns 0. Otherwise it returns an error code.
// Possible errors: 101, 103
// Must allow external buffer to be passed in as dest_ptr. (read only request)
int mms_memcpy(char* dest_ptr, char* src_ptr, int size) {
    return 0;
}

// Print the number of characters in hex format to STDOUT.
// If size=0, then print until the first hex 0 to STDOUT.
// If successful, returns 0. Otherwise it returns an error code.
// Possible errors: 103
// Must allow external buffer to be passed in as src_ptr. (read only request)
int mms_print(char* src_ptr, int size) {
    return 0;
}


// Free the allocated memory.
// If successful, returns 0.  Otherwise it returns an error code.
// Possible errors: 104
int mms_free ( char* mem_ptr ) {
    return 0;
}

	



