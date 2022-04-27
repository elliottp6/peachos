#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "config.h"
#include "string/string.h"

void* isr80h_command6_process_load_start( struct interrupt_frame* frame ) {
    // get filename pointer in userspace
    void* userspace_filename = task_get_stack_item( task_current(), 0 );

    // copy userspace filename into kernelspace filename
    char filename[PEACHOS_MAX_PATH];
    int res = copy_string_from_task( task_current(), userspace_filename, filename, sizeof( filename ) );
    if( res < 0 ) goto out;

    // flesh it out into a full path
    char path[PEACHOS_MAX_PATH];
    strcpy( path, "0:/" );
    strcpy( path + 3, filename );

    // load the process & give it the focus
    struct process* process = NULL;
    if( (res = process_load_and_give_focus( path, &process )) < 0 ) goto out;

    // switch to the new task
    task_switch( process->task );
    task_return( &process->task->registers );

    // note that we never actually return from here, except in failure case
out:
    return NULL;
}

void* isr80h_command7_invoke_system_command( struct interrupt_frame* frame ) {
    return 0;
}

void* isr80h_command8_get_program_arguments( struct interrupt_frame* frame ) {
    // get 1st argument to this command (userspace pointer to process_arguments)
    struct task* task = task_current();
    void* item = task_get_stack_item( task, 0 );

    // convert pointer to kernel space & write the process' arguments into it
    struct process_arguments* args = task_virtual_address_to_physical( task, item );
    process_get_arguments( task->process, &args->argc, &args->argv );
    return 0;
}
