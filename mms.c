#include "mms.h"
#include <alloca.h>
#include <limits.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>

static struct regions *memory;
static char *mem_region;
static char *end_mem;
static char prog_name[50]; 

int shared_mem_init(char * executable_name) {
    // generate key from filepath and id
    key_t key = ftok(shared_filepath, SHARED_ID);
    // create and/or get id of shared memory
    int shmid = shmget(key, MAX_MEM_SIZE, 0666 | IPC_CREAT);
    // map the memory to local address space
    memory = (struct regions*)shmat(shmid, (void*)0, 0);
    mem_region = (char*) memory + sizeof(struct regions);
    end_mem = mem_region + memory->allocated_size;

    strcpy(prog_name, executable_name);
    if (strlen(prog_name) > 14) {
        strcpy(prog_name + 11, "...\0");
    }

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

    /// TODO: modify to take O(N) time and use "free space" entries

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
        // check every client in the list
        for (int i = 0; i < memory->current_clients; i++) {
            if (memory->mmap_table[i].client_pid == 0) 
                continue;
            int mem_entry_offset = memory->mmap_table[i].mem_offset;
            // if the client is after the start of the hole
            if (mem_entry_offset > tmp_offset
                    // and the start of the region is closest found yet
                    && mem_entry_offset - tmp_offset < tmp_size) {
                // the width of the hole shrinks to the new size
                tmp_size = mem_entry_offset - tmp_offset;
            }
        }
        // if this is the hole at the end, set the size 
        if (tmp_offset + tmp_size > memory->allocated_size) {
            tmp_size = memory->allocated_size - tmp_offset;
        }
        if (tmp_size > curr_size) {
            curr_size = tmp_size;
        }
    }
    if (tmp_offset >= boundary_offset) {
        return -1;
    }
    if (curr_size > size) {
        return -1;
    }
    return tmp_offset;        
}

// Allocate a piece of memory given the input size.
// If successful, returns a valid pointer. Otherwise it returns NULL (0) and set the error_code.
// Possible errors: 100
char* mms_malloc(int size, int* error_code) {
    pid_t this_pid = getpid();
  
    // TODO: verify room for another entry



    if (size < 1) {
        return 0;
    }
    int allocated_size = size;
    if (allocated_size % memory->boundary_size != 0) {
         allocated_size += memory->boundary_size - allocated_size % memory->boundary_size;
    }
    //if (allocated_size < memory->boundary_size) {
    //    allocated_size = memory->boundary_size;
    //}
    

    char* allocated_ptr = mem_region + memory->next_pointer_offset;
    if (allocated_ptr >= end_mem || allocated_ptr + allocated_size > end_mem) {
        int found_offset = find_space(allocated_size);
        if (found_offset == -1) {
            enum err_code error = OUT_OF_MEM;
            *error_code = (int) error;
            return 0;
        } else {
            allocated_ptr = mem_region + found_offset;
        }
    }
        
    memory->next_pointer_offset = (int)(allocated_ptr - mem_region) + allocated_size;
    struct mmap_table_entry new_entry;
    new_entry.client_pid = this_pid;
    new_entry.request_size = size;
    new_entry.actual_size = allocated_size;
    new_entry.client_address = allocated_ptr;
    new_entry.mem_offset = (int) ( allocated_ptr - mem_region );
    time_t curr_time;
    time(&curr_time);
    new_entry.last_reference = curr_time;

    // formatting timestamp for log
    struct tm *curr_time_info = localtime(&curr_time);
    char time_string[15];
    strftime(time_string, 15, "%Y%m%d%H%M%S", curr_time_info);
    
    //  logging operation
    FILE *log_file = fopen("./mms.log", "a");
    // timestamp, exectuable name, pid, function name, return value, parameters
    fprintf(log_file, " %14s | %14s | %14d | %s %p %d %d\n", 
            time_string, prog_name, this_pid, "mms_malloc", allocated_ptr, size, *error_code); 
    fclose(log_file);

    memory->mmap_table[memory->total_entries] = new_entry;
    memory->total_entries++;
    memory->current_clients++;
    memory->allocated_size++;

    return allocated_ptr; 
}

// returns 0 for ownership or error code of error type
int verify_ownership(int offset, int size, struct mmap_table_entry **table_entry) {
    pid_t this_pid = getpid();
    for (int i = 0; i < memory->total_entries; i++) {
        struct mmap_table_entry entry = memory->mmap_table[i];
        if (entry.client_pid == this_pid) {
            if (offset >= entry.mem_offset && offset < entry.mem_offset + entry.request_size) {
                if (offset + size <= entry.mem_offset + entry.request_size) {
                    *table_entry = &(memory->mmap_table[i]);
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
    int this_pid = getpid();
    // pointer will hold the entry found by the ownership verification 
    struct mmap_table_entry *found_entry;
     time_t curr_time;
    time(&curr_time);
    
    int dest_offset = dest_ptr - mem_region;
    int err = verify_ownership(dest_offset, size, &found_entry); 
    if (err == 0) {
        for (int i = 0; i < size; i++) {
            dest_ptr[i] = c;
        }
       
        // updating time since last modification
        found_entry->last_reference = curr_time;


    }    

   // formatting timestamp for log
    struct tm *curr_time_info = localtime(&curr_time);
    char time_string[15];
    strftime(time_string, 15, "%Y%m%d%H%M%S", curr_time_info);
    
    //  logging operation
    FILE *log_file = fopen("./mms.log", "a");
    // timestamp, exectuable name, pid, function name, return value (error), dest, character, size 
    fprintf(log_file, " %14s | %14s | %14d | %s %d %p %c, %d\n", 
            time_string, prog_name, this_pid, "mms_memset", err, dest_ptr, c, size); 
    fclose(log_file);



    return err;
}

// Copy the fixed number of bytes from source to destination.
// If successful, returns 0. Otherwise it returns an error code.
// Possible errors: 101, 103
// Must allow external buffer to be passed in as dest_ptr. (read only request)
int mms_memcpy(char* dest_ptr, char* src_ptr, int size) {
    int this_pid = getpid();
    int dest_offset = dest_ptr - mem_region;
    int src_offset = src_ptr - mem_region;

    struct mmap_table_entry *dest_entry;
    struct mmap_table_entry *src_entry;


    /// specifications say "must allow for external buffer to be passed in as 
    /// dest. (read only request)
    /// Because this says read only, but also destination, it is unclear which
    /// should be able to be external. I decided that this function would 
    /// protect the shared memory, but will allow for external source or dest
    /// buffers to be passed in. An error will be thrown if the src or dest 
    /// cross a boundary of allocated memory, but any client pid is allowed as 
    /// long as the buffer lies within one region.


    // TODO: implement this ^ 
    

    int err;
    err = verify_ownership(dest_offset, size, &dest_entry);
    if (err != 0) {
        if (err == 102)
                err = 103;
        return err;

    }
    err = verify_ownership(src_offset, size, &src_entry);
    if (err != 0) {
        if (err == 102) 
                err = 103;
        return err;
    }
    
    if (err == 0) {
        for (int i = 0; i < size; i++) {
            dest_ptr[i] = src_ptr[i];
        }
    }
 

    time_t curr_time;
    time(&curr_time);
    // formatting timestamp for log
    struct tm *curr_time_info = localtime(&curr_time);
    char time_string[15];
    strftime(time_string, 15, "%Y%m%d%H%M%S", curr_time_info);
    
    //  logging operation
    FILE *log_file = fopen("./mms.log", "a");
    // timestamp, exectuable name, pid, function name, return value (error), dest, src, size 
    fprintf(log_file, " %14s | %14s | %14d | %s %d %p %p, %d\n", 
            time_string, prog_name, this_pid, "mms_memcpy", err, dest_ptr, src_ptr, size); 
    fclose(log_file);

    return err;

}

// Print the number of characters in hex format to STDOUT.
// If size=0, then print until the first hex 0 to STDOUT.
// If successful, returns 0. Otherwise it returns an error code.
// Possible errors: 103
// Must allow external buffer to be passed in as src_ptr. (read only request)
int mms_print(char* src_ptr, int size) {
    int src_offset = src_ptr - mem_region;
    struct mmap_table_entry * entry;
    int err = verify_ownership(src_offset, size, &entry);
    if (err == 102) {
        err = 103;
    }


    // TODO: test, fix whatever it means by "allow external buffer"


    if (err == 0) {
        if (size == 0) {
            while (*src_ptr != 0) {
                printf("%2hhX ", *src_ptr);
                src_ptr++;
            }
        } else {
            for (int i = 0; i < size; i++) {
                printf("%2hhX ", src_ptr[i]);
            }
        }
    }

    printf("\n");


    pid_t this_pid = getpid();
    time_t curr_time;
    time(&curr_time);
    // formatting timestamp for log
    struct tm *curr_time_info = localtime(&curr_time);
    char time_string[15];
    strftime(time_string, 15, "%Y%m%d%H%M%S", curr_time_info);
    
    //  logging operation
    FILE *log_file = fopen("./mms.log", "a");
    // timestamp, exectuable name, pid, function name, return value (error), src, size 
    fprintf(log_file, " %14s | %14s | %14d | %s %d %p, %d\n", 
            time_string, prog_name, this_pid, "mms_print", err, src_ptr, size); 
    fclose(log_file);




    return 0;

}



// Free the allocated memory.
// If successful, returns 0.  Otherwise it returns an error code.
// Possible errors: 104
int mms_free ( char* mem_ptr ) {
    pid_t this_pid  = getpid();
    int target_offset = mem_ptr - mem_region;
 
    time_t curr_time;
    time(&curr_time);
    // formatting timestamp for log
    struct tm *curr_time_info = localtime(&curr_time);
    char time_string[15];
    strftime(time_string, 15, "%Y%m%d%H%M%S", curr_time_info);
    
    //  logging operation
    FILE *log_file = fopen("./mms.log", "a");


    for (int i = 0; i < memory->total_entries; i++) {
        struct mmap_table_entry curr_entry = memory->mmap_table[i];
        if ( this_pid == curr_entry.client_pid ) {
            if ( target_offset == curr_entry.mem_offset ) {
                
                memory->mmap_table[i].client_pid = 0;


                // timestamp, exectuable name, pid, function name, return value (error), memory pointer 
                fprintf(log_file, " %14s | %14s | %14d | %s %d %p\n", 
                        time_string, prog_name, this_pid, "mms_print", 0, mem_ptr ); 
                fclose(log_file);

                   
                return 0;
            }
        }
    }

    enum err_code error = INVALID_MEM_ADDR;

    // timestamp, exectuable name, pid, function name, return value (error), memory pointer
    fprintf(log_file, " %14s | %14s | %14d | %s %d %p\n", 
            time_string, prog_name, this_pid, "mms_print", (int)error, mem_ptr); 
    fclose(log_file);

   
    return (int) error;

}



	



