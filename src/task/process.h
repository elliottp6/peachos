#pragma once
#include <stdint.h>
#include "config.h"
#include "task.h"

// process filetype
#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1
typedef uint8_t PROCESS_FILETYPE;

// process allocation object
struct process_allocation { void* ptr; size_t size; };

// command argument
struct command_argument { char argument[512]; struct command_argument* next; };

// process argument
struct process_arguments { int argc; char** argv; };

// process object
struct process {
    // process info
    uint16_t id;
    char filename[PEACHOS_MAX_PATH];
    struct task* task;
    struct process_allocation allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS]; // memory allocations of the process
    
    // process memory (which can be either binary or ELF)
    PROCESS_FILETYPE filetype;
    union { void* ptr; struct elf_file* elf_file; };
    
    void* stack; // stack memory
    uint32_t size; // size of data pointed to by 'ptr'
    
    // keyboard info
    struct keyboard_buffer {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail, head;
    } keyboard;

    // arguments
    struct process_arguments arguments;
};

// functions
int process_load( const char* filename, struct process** process );
int process_load_focus( const char* filename, struct process** process );
int process_focus( struct process* process );
struct process* process_focused();
struct process* process_get( int process_id );
void* process_malloc( struct process* process, size_t size );
void process_free( struct process* process, void* ptr );
void process_get_arguments( struct process* process, int* argc, char*** argv );
int process_inject_arguments( struct process* process, struct command_argument* root_argument );
int process_terminate( struct process* process );
