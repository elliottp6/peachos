#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "process.h"
#include "idt/idt.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "loader/formats/elfloader.h"

// data
struct task* current_task = NULL; // current task that's running
struct task* task_tail = NULL; // tail of the linked list (last insertion)
struct task* task_head = NULL; // head of the linked list (first insertion)

// functions
struct task* task_current() { return current_task; } // returns the current task

// does a round-robin loop through the task list (can return NULL if no tasks)
struct task* task_get_next() { return current_task->next ? current_task->next : task_head; }

static void task_list_insert( struct task* task ) {
    if( NULL == task_head ) { current_task = task_head = task_tail = task; return; }    
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

int task_init( struct task* task, struct process* process ) {
    // clear structure
    memset( task, 0, sizeof( struct task ) );

    // map entire 4GB address space to itself (identity transform)
    // this is a read-only address space
    task->paging_directory = paging_new_4gb( PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL );
    if( !task->paging_directory ) return -EIO;

    // initialze the registers
    task->registers.ip = PROCESS_FILETYPE_ELF == process->filetype ?
                         elf_header( process->elf_file )->e_entry :
                         PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->process = process;
    return 0;
}

int task_free( struct task* task ) {
    paging_free_4gb( task->paging_directory );
    task_list_remove( task );
    kfree( task );
    return 0;
}

void task_next() {
    struct task* next_task = task_get_next();
    if( !next_task ) panic( "No more tasks!\n" );
    task_switch( next_task );
    task_return( &next_task->registers );
}

// change the current task that's running & change the page directories to point to the tasks'
int task_switch( struct task* task ) {
    current_task = task;
    paging_switch( task->paging_directory );
    return 0;
}

void task_save_state( struct task* task, struct interrupt_frame* frame ) {
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}

// the 'virtual' address is in userspace, so we cannot directly access from kernelland
int copy_string_from_task( struct task* task, void* virtual, void* physical, int max ) {
    // allocate a page-aligned buffer that is < PAGE_SIZE
    if( max >= PAGING_PAGE_SIZE ) return -EINVARG;
    char* tmp = kzalloc( max );
    if( NULL == tmp ) return -ENOMEM;

    // backup the task page, because it may point to physical memory that the process is using
    // (process is frozen, so it won't need this page for now)
    uint32_t *task_directory = task->paging_directory->directory_entry,
             old_entry = paging_get( task_directory, tmp );

    // point virtual address 'tmp' to physical (kernel) address 'tmp'
    // TODO: why do we need 'tmp'? can't we just map somewhere directly to 'physical' and do the copy in user space?
    paging_map( task->paging_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL );
    
    // switch to task's address space (note that 'tmp' is being mapped into process space)
    paging_switch( task->paging_directory );

    // copy into tmp from virtual TODO: what if 'virtual' is located in 'tmp'?
    strncpy( tmp, virtual, max );

    // switch back to the kernel address space
    kernel_page();
    
    // restore the task's page
    int res = 0;
    if( paging_set( task_directory, tmp, old_entry ) < 0 ) { res = -EIO; goto out; }

    // copy from 'tmp' into the destination
    strncpy( physical, tmp, max );

    // deallocate the 'tmp' memory
out:
    kfree( tmp );
    return res;
}

void task_current_save_state( struct interrupt_frame* frame ) {
    struct task* task = task_current();
    if( !task ) panic( "No current task to save\n" );
    task_save_state( task, frame );
}

// interrupt in userland causes our kernel to run
// from our kernel, we'll want to call this to restore the user registers
// before switching the task
int task_page() {
    user_registers();
    task_switch( current_task );
    return 0;
}

int task_page_task( struct task* task ) {
    user_registers();
    paging_switch( task->paging_directory );
    return 0;
}

void task_run_first_ever_task() {
    // TODO: shouldn't this check for task_head?
    if( !current_task ) panic( "task_run_first_ever_task(): No current task exists!\n" );
    
    // switch the page directory & set current_task
    task_switch( task_head );

    // drop into userland
    task_return( &task_head->registers );
}

struct task* task_new( struct process* process ) {
    // allocate the task
    struct task* task = kzalloc( sizeof( struct task ) );
    if( !task ) return ERROR( -ENOMEM );

    // initialize the task
    int res;
    if( (res = task_init( task, process )) < 0 ) goto out;

    // insert task into linked list
    task_list_insert( task );

out:
    if( ISERR( res ) ) { task_free( task ); return ERROR( res ); }
    return task;
}

// reads ith item from task's stack
void* task_get_stack_item( struct task* task, int i ) {
    // get virtual stack pointer
    uint32_t* virtual_stack = (uint32_t*)task->registers.esp;

    // switch to the task's page
    task_page_task( task );

    // read the value in task address space
    void* value = (void*)virtual_stack[i];

    // switch back to the kernel page
    kernel_page();
    return value;
}

void* task_virtual_address_to_physical( struct task* task, void* virt ) {
    return paging_get_physical_address( task->paging_directory->directory_entry, virt );
}
