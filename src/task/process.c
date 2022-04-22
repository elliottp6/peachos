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
#include "loader/formats/elfloader.h"

// process storage
struct process* focused_process = 0;
static struct process* processes[PEACHOS_MAX_PROCESSES] = {};

static void process_init( struct process* process ) { memset( process, 0, sizeof( struct process ) ); }

struct process* process_focused() { return focused_process; }

struct process* process_get( int process_id ) {
    if( process_id < 0 || process_id >= PEACHOS_MAX_PROCESSES ) return NULL;
    return processes[process_id];
}

static int process_find_free_allocation_index( struct process* process ) {
    for( int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++ )
        if( !process->allocations[i] ) return i;
    return -ENOMEM;
}

void* process_malloc( struct process* process, size_t size ) {
    // find an allocation slot
    int index = process_find_free_allocation_index( process );
    if( index < 0 ) return NULL;
    
    // allocate on kernel heap
    void* ptr = kzalloc( size );
    if( !ptr ) return NULL;

    // save ptr so we can deallocate it when process terminates
    process->allocations[index] = ptr;
    return ptr;
}

// note: in another OS, there might be something to actually do here
int process_focus( struct process* process ) { focused_process = process; return 0; }

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
    process->filetype = PROCESS_FILETYPE_BINARY;
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    // TODO: in the lecture version, the 'program_data_ptr' is not deallocated
    if( res < 0 ) { if( program_data_ptr ) kfree( program_data_ptr ); } // if error: deallocate program_data
    fclose( fd ); // close file
    return res;
}

static int process_load_elf( const char* filename, struct process* process ) {
    // load elf file
    struct elf_file* elf_file = NULL;
    int res = elf_load( filename, &elf_file );
    if( res < 0 ) return res;

    // set the process attributes
    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = elf_file;
    return 0;
}

// later on, add support for ELF files, etc. rather than just raw binaries (which are like DOS COM files)
static int process_load_data( const char* filename, struct process* process ) {
    // try to load 'filename' as ELF
    int res = process_load_elf( filename, process );

    // if it was not ELF format, then load as raw binary
    if( -EINFORMAT == res ) return process_load_binary( filename, process );
    return res;
}

int process_map_binary( struct process* process ) {
    return paging_map_to(
        process->task->paging_directory,
        (void*)PEACHOS_PROGRAM_VIRTUAL_ADDRESS,
        process->ptr,
        paging_align_ceiling( process->ptr + process->size ),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE );
}

static int process_map_elf( struct process* process ) {
    // get the program header table
    struct elf_file* elf_file = process->elf_file;
    struct elf_header* header = elf_header( elf_file );

    // map each program header into memory
    for( int i = 0; i < header->e_phnum; i++ ) {
        // get next program header
        struct elf32_phdr* program_header = elf_program_header( header, i );
        
        // determine page flags for this segment
        int page_flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
        if( PF_W & program_header->p_flags ) page_flags|= PAGING_IS_WRITEABLE;

        // get physical address where we're going to put this segment
        void* physical_address = elf_program_header_physical_address( elf_file, program_header );
        
        // map the memory
        int res = paging_map_to(
            process->task->paging_directory,
            paging_align_floor( (void*)program_header->p_vaddr ),
            paging_align_floor( physical_address ),
            paging_align_ceiling( physical_address + program_header->p_filesz ),
            page_flags );
        if( res < 0 ) return res;
    }
    return 0;
}

int process_map_memory( struct process* process ) {
    // map the virtual address space for the process' static memory
    int res = 0;
    switch( process->filetype ) {
        case PROCESS_FILETYPE_ELF: res = process_map_elf( process ); break;
        case PROCESS_FILETYPE_BINARY: res = process_map_binary( process ); break;
        default: panic( "process_map_memory: unsupported process filetype\n" ); return -EINVARG;
    }
    if( res < 0 ) return res;

    // map the virtual address space for the process' stack (otherwise, these will be unmapped, and it will cause a page fault when the process tries to push/pop from its stack)
    return paging_map_to(
        process->task->paging_directory,
        (void*)PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, // note that 'end' comes BEFORE start, since stack grows from higher addresses to lower ones
        process->stack,
        paging_align_ceiling( process->stack + PEACHOS_USER_PROGRAM_STACK_SIZE ),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE );
}

int process_get_free_slot() {
    for( int i = 0; i < PEACHOS_MAX_PROCESSES; i++ ) if( NULL == processes[i] ) return i;
    return -EISTKN;
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

int process_load( const char* filename, struct process** process ) {
    // get available slot
    int process_slot = process_get_free_slot();
    if( process_slot < 0 ) return -ENOMEM;

    // load process into slot
    return process_load_for_slot( filename, process, process_slot );
}

int process_load_and_give_focus( const char* filename, struct process** process ) {
    int res = process_load( filename, process );
    return 0 == res ? process_focus( *process ) : res;
}
