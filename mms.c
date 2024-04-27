#include "mms.h"
#include <alloca.h>
#include <limits.h>
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

// Finds an offset to place the requested size at, returns -1 if no space exists
int find_space(int size) {
    int tmp_offset = 0;
    int curr_size = 0;
    int boundary_offset = memory->allocated_size;
    while(curr_size < size || tmp_offset >= boundary_offset) {
        // keeps jumping the tmp_offset pointer untill it reaches an unnallocated byte
        for (int i = 0; i < memory->current_clients; i++) {
            if (memory->mmap_table[i].client_pid == 0) 
                continue;
            int mem_entry_offset = memory->mmap_table[i].mem_offset;
            int mem_entry_end = memory->mmap_table[i].actual_size + mem_entry_offset;
            if (tmp_offset >= mem_entry_offset && tmp_offset < mem_entry_end) {
                tmp_offset = mem_entry_end;
            }
        }

        // finding the width of the hole
        int tmp_size = INT_MAX;
        for (int i = 0; i < memory->current_clients; i++) {
            if (memory->mmap_table[i].client_pid == 0) 
                continue;
            int mem_entry_offset = memory->mmap_table[i].mem_offset;
            if (mem_entry_offset > tmp_offset
                    && mem_entry_offset - tmp_offset < tmp_size) {
                tmp_size = mem_entry_offset - tmp_offset;
            }
        }
        if (tmp_size > curr_size) {
            curr_size = tmp_size;
        }
    }
    if (tmp_offset >= boundary_offset) {
        return -1;
    }
    if (curr_size < size) {
        return -1;
    }
    return tmp_offset;        
}

// Allocate a piece of memory given the input size.
// If successful, returns a valid pointer. Otherwise it returns NULL (0) and set the error_code.
// Possible errors: 100
char* mms_malloc(int size, int* error_code) {
    pid_t this_pid = getpid();
   
    if (size < 1) {
        return 0;
    }
    int allocated_size = size;
    if (allocated_size % 8 != 0) {
        allocated_size += 8 - allocated_size % 8;
    }
    

    char* allocated_ptr = mem_region + memory->next_pointer_offset;
    if (allocated_ptr >= end_mem) {
        enum err_code error = OUT_OF_MEM;
        *error_code = (int) error;
        return 0;

    }
    // TODO: implement filling holes in memory using find_space function
        
    memory->next_pointer_offset = (int)(allocated_ptr - mem_region) + allocated_size;
    struct mmap_table_entry new_entry;
    new_entry.client_pid = this_pid;
    new_entry.request_size = size;
    new_entry.actual_size = allocated_size;
    new_entry.client_address = allocated_ptr;
    new_entry.mem_offset = (int) ( allocated_ptr - mem_region );
    //struct tm *time = localtime(NULL);
    //new_entry.last_reference = *time;

    memory->mmap_table[memory->total_entries] = new_entry;
    memory->total_entries++;
    memory->current_clients++;
    memory->allocated_size++;

    return allocated_ptr; 
}

// returns 0 for ownership or error code of error type
int verify_ownership(int offset, int size) {
    pid_t this_pid = getpid();
    for (int i = 0; i < memory->total_entries; i++) {
        struct mmap_table_entry entry = memory->mmap_table[i];
        if (entry.client_pid == this_pid) {
            if (offset >= entry.mem_offset && offset < entry.mem_offset + entry.request_size) {
                if (offset + size < entry.mem_offset + entry.request_size) {
                    return 0;
                } else {
                    return 101;
                }
            }
        }
    }
    return 102;
}

// Set the destination buffer with a character of certain size.
// If successful, returns 0. Otherwise it returns an error code.
// Possible errors: 101, 102
int mms_memset(char* dest_ptr, char c, int size) {
    
    int dest_offset = dest_ptr - mem_region;
    int err = verify_ownership(dest_offset, size); 
    if (err == 0) {
        for (int i = 0; i < size; i++) {
            dest_ptr[i] = c;
        }
    } else {
        return  err;
    }

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



	



