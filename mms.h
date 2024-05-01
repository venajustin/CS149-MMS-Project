#ifndef MMS_H
#define MMS_H


#include <time.h>
#define MAX_MEM_SIZE (10000)
#define MAX_REQUESTS ( 50 )
#define SHARED_ID ('M')
#define SHARED_FILEPATH "/shared_memory_temp_file"
#define LOGFILE_NAME "./mms.log"

enum err_code{
    NO_ERROR = 0,
    OUT_OF_MEM = 100, 
    MEM_TOO_SMALL = 101,
    INVALID_DEST_ADDR = 102,
    INVALID_CPY_ADDR = 103,
    INVALID_MEM_ADDR = 104
};

struct mmap_table_entry {
    long client_pid;
    int request_size;
    int actual_size;
    char *client_address;
    int mem_offset;
    time_t last_reference;
};

struct mem_map_table {
    int active_manager;
    int mem_size;
    int min_block_size;
    int current_clients;
    int max_requests;
    int total_entries;
    struct mmap_table_entry mmap_regions[MAX_REQUESTS];
    char mem_start[MAX_MEM_SIZE]; 
};

char *mms_init();
int shared_mem_delete();

char* mms_malloc(int size, int* error_code);
int mms_memset(char* dest_ptr, char c, int size);
int mms_memcpy(char* dest_ptr, char* src_ptr, int size);
int mms_print(char* src_ptr, int size);
int mms_free ( char* mem_ptr );

#endif
