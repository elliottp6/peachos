#pragma once
#include <stdint.h>
#include "config.h"
#include "task.h"

struct process {
    uint16_t id;
    char filename[PEACHOS_MAX_PATH];
    struct task* task;
    void* allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS]; // memory allocations of the process
    void *ptr, *stack; // the physical pointer to the process memory, stack memory
    uint32_t size; // size of data pointed to by 'ptr'  
};

// functions
int process_load_for_slot( const char* filename, struct process** process, int process_slot );
int process_load( const char* filename, struct process** process );
