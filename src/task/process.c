#include "process.h"
#include "memory/memory.h"
#include "status.h"
#include "task/task.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "config.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"

struct process* current_process = 0;
static struct process* processes[PEACHOS_MAX_PROCESSES] = {};

static void process_init( struct process* process ) {
    memset( process, 0, sizeof( struct process ) );
}

struct process* process_current() { return current_process; }

struct process* process_get( int process_id ) {
    if( process_id < 0 || process_id >= PEACHOS_MAX_PROCESSES ) return NULL;
    return processes[process_id];
}

static int process_load_binary( const char* filename, struct process* process ) {
    // open the file containing the binary
    int fd = fopen( filename, "r" );
    if( fd <= 0 ) return -EIO;

    // get filesize
    struct file_stat stat;
    int res = fstat( fd, &stat );
    if( res < 0 ) goto out;

    // allocate program data
    void* program_data_ptr = kzalloc( stat.filesize );
    if( !program_data_ptr ) { res = -ENOMEM; goto out; }

    // read program into memory
    if( 1 != fread( program_data_ptr, stat.filesize, 1, fd ) ) { res = -EIO; goto out; }

    // setup process
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    // TODO: in the lecture version, the 'program_data_ptr' is not deallocated
    if( res < 0 ) { if( program_data_ptr ) kfree( program_data_ptr ); } // if error: deallocate program_data
    fclose( fd ); // close file
    return res;
}

// later on, add support for ELF files, etc. rather than just raw binaries (which are like DOS COM files)
static int process_load_data( const char* filename, struct process* process ) { return process_load_binary( filename, process ); }

int process_map_binary( struct process* process ) {
    return paging_map_to(
        process->task->paging_directory->directory_entry,
        (void*)PEACHOS_PROGRAM_VIRTUAL_ADDRESS,
        process->ptr,
        paging_align_address( process->ptr + process->size ),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE );
}

int process_map_memory( struct process* process ) { return process_map_binary( process ); }

int process_get_free_slot() {
    for( int i = 0; i < PEACHOS_MAX_PROCESSES; i++ ) if( NULL == processes[i] ) return i;
    return -EISTKN;
}

int process_load( const char* filename, struct process** process ) {
    // get available slot
    int process_slot = process_get_free_slot();
    if( process_slot < 0 ) return -ENOMEM;

    // load process into slot
    return process_load_for_slot( filename, process, process_slot );
}

int process_load_for_slot( const char* filename, struct process** process, int process_slot ) {
    // make sure slot is available
    if( NULL != process_get( process_slot ) ) return -EISTKN;

    // allocate process
    struct process* _process = kzalloc( sizeof( struct process ) );
    if( NULL == _process ) return -ENOMEM;
    
    // initialize the process
    process_init( _process );

    // load code + static data
    int res;
    if( (res = process_load_data( filename, _process )) < 0 ) goto out;
    
    // allocate stack space (note: this must be 4096 aligned for paging purposes, which is easy since our kzalloc is always aligned)
    void* program_stack_ptr = kzalloc( PEACHOS_USER_PROGRAM_STACK_SIZE );
    if( NULL == program_stack_ptr ) { res = -ENOMEM; goto out; }

    // set process filename, stack & id
    strncpy( _process->filename, filename, sizeof( _process->filename ) );
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    // create a task
    struct task* task = task_new( _process );
    if( 0 == ERROR_I( task ) ) { res = ERROR_I( task ); goto out; }
    _process->task = task;

    // map the memory
    if( (res = process_map_memory( _process ) ) < 0 ) goto out;

    // set the output variable & add to slot
    *process = _process;
    processes[process_slot] = _process;

out:
    if( ISERR( res ) ) {
        if( _process->task ) task_free( _process->task ); // free task
        // TODO: free the process data
    }
    return res;
}
