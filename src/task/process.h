#pragma once
#include <stdint.h>
#include "config.h"
#include "task.h"

// process filetype
#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1
typedef uint8_t PROCESS_FILETYPE;

// process object
struct process {
    // process info
    uint16_t id;
    char filename[PEACHOS_MAX_PATH];
    struct task* task;
    void* allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS]; // memory allocations of the process
    
    // process memory
    PROCESS_FILETYPE filetype;
    union {
        void* ptr; // process memory (pointer to binary file)
        struct elf_file* elf_file; // pointer to elf_file
    };
    
    void* stack; // stack memory
    uint32_t size; // size of data pointed to by 'ptr'
    
    // keyboard info
    struct keyboard_buffer {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail, head;
    } keyboard;
};

// functions
int process_load( const char* filename, struct process** process );
int process_load_switch( const char* filename, struct process** process );
int process_load_for_slot( const char* filename, struct process** process, int process_slot );
int process_switch( struct process* process );
struct process* process_current(); // this is the process which has the FOCUS (not neccessarily even running!)
struct process* process_get( int process_id );
