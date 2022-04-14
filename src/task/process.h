#pragma once
#include <stdint.h>
#include "config.h"
#include "task.h"

struct process {
    // process info
    uint16_t id;
    char filename[PEACHOS_MAX_PATH];
    struct task* task;
    void* allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS]; // memory allocations of the process
    void *ptr, *stack; // the physical pointer to the process memory, stack memory
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
