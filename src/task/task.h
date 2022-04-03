#pragma once
#include "config.h"
#include "memory/paging/paging.h"

struct registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip; // instruction pointer
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

// forward declarations
struct process;

// types
struct task {
    struct paging_4gb_chunk* paging_directory; // page tables for this task
    struct registers registers; // holds registers when task is not running
    struct process* process;
    struct task *next, *prev; // previous & next task in the linked list
};

// functions
struct task* task_new( struct process* process );
struct task* task_current();
struct task* task_get_next();
int task_free( struct task* task );
