#ifndef MMS_H
#define MMS_H


#include <time.h>

// The size of the shared memory region
#define MAX_MEM_SIZE (10000)
// The max number of requests that can be requesting info
#define MAX_REQUESTS ( 30 )
// id and filepath used in the ftok function
#define SHARED_ID ('M')
char *shared_filepath = "/shared_memory_temp_file";

struct mmap_table_entry {
    long client_pid;
    int request_size;
    int actual_size;
    char *client_address;
    struct tm last_reference;
};

struct regions {
    int active_manager;
    int allocated_size;
    int current_clients;
    int max_requests;
    struct mmap_table_entry mmap_table[MAX_REQUESTS];
};


int shared_mem_init();

int first_byte();
void set_mem(int c);

static struct regions *memory;

#endif
