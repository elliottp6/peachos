#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

// data
struct task* current_task = NULL; // current task that's running
struct task* task_tail = NULL; // tail of the linked list (last insertion)
struct task* task_head = NULL; // head of the linked list (first insertion)

// functions
struct task* task_current() { return current_task; } // returns the current task

// does a round-robin loop through the task list (can return NULL if no tasks)
struct task* task_get_next() { return current_task->next ? current_task->next : task_head; }

static void task_list_insert( struct task* task ) {
    if( NULL == task_head ) { task_head = task_tail = task; return; }    
    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;
}

static void task_list_remove( struct task* task ) {
    if( task->prev ) task->prev->next = task->next;
    if( task_head == task ) task_head = task->next;
    if( task_tail == task ) task_tail = task->prev;
    if( current_task == task ) current_task = task_get_next();
}

int task_init( struct task* task ) {
    // clear structure
    memset( task, 0, sizeof( struct task ) );

    // map entire 4GB address space to itself (identity transform)
    // this is a read-only address space
    task->paging_directory = paging_new_4gb( PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL );
    if( !task->paging_directory ) return -EIO;

    // initialze the registers
    task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    return 0;
}

int task_free( struct task* task ) {
    paging_free_4gb( task->paging_directory );
    task_list_remove( task );
    kfree( task );
    return 0;
}

struct task* task_new() {
    // allocate the task
    struct task* task = kzalloc( sizeof( struct task ) );
    if( !task ) return ERROR( -ENOMEM );

    // initialize the task
    int res;
    if( (res = task_init( task )) < 0 ) goto out;

    // insert task into linked list
    task_list_insert( task );

out:
    if( ISERR( res ) ) { task_free( task ); return ERROR( res ); }
    return task;
}
