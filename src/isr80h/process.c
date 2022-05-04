#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "config.h"
#include "string/string.h"
#include "kernel.h"

void* isr80h_command6_process_load_start( struct interrupt_frame* frame ) {
    // get filename pointer in userspace
    void* userspace_filename = task_get_stack_item( task_current(), 0 );

    // copy userspace filename into kernelspace filename
    char filename[PEACHOS_MAX_PATH];
    int res = copy_string_from_task( task_current(), userspace_filename, filename, sizeof( filename ) );
    if( res < 0 ) return ERROR( res );

    // flesh it out into a full path
    char path[PEACHOS_MAX_PATH];
    strcpy( path, "0:/" );
    strcpy( path + 3, filename );

    // load the process & give it the focus
    struct process* process = NULL;
    if( (res = process_load_focus( path, &process )) < 0 ) return ERROR( res );

    // switch to the new task
    task_switch( process->task );
    task_return( &process->task->registers );
    return NULL;
}

void* isr80h_command7_invoke_system_command( struct interrupt_frame* frame ) {
    // get 1st arg
    struct task *task = task_current();
    struct command_argument *root_arg = task_virtual_address_to_physical( task, task_get_stack_item( task, 0 ) );
    if( !root_arg || 0 == strlen( root_arg[0].argument ) ) return ERROR( -EINVARG );

    // turn 1st arg into a full path
    // TODO: the lecture does sizeof( path ) instead of sizeof( path ) - 3
    const char *program_name = root_arg->argument;
    char path[PEACHOS_MAX_PATH];
    strcpy( path, "0:/" );
    strncpy( path + 3, program_name, sizeof( path ) - 3 );

    // load process & give it focus
    struct process* process = NULL;
    int res = process_load_focus( path, &process );
    if( res < 0 ) return ERROR( res );

    // inject our command arguments
    if( (res = process_inject_arguments( process, root_arg )) < 0 ) return ERROR( res );

    // switch current-task and switch to user page directory
    task_switch( process->task );

    // drop into the task TODO: still not how we access the registers *after* we switch the paging directory (unless, of course, these regs are in the user's address space!)
    task_return( &process->task->registers );
    return NULL;
}

void* isr80h_command8_get_program_arguments( struct interrupt_frame* frame ) {
    // get 1st arg
    struct task* task = task_current();
    void* item = task_get_stack_item( task, 0 );

    // convert pointer to kernel space & write the process' arguments into it
    struct process_arguments* args = task_virtual_address_to_physical( task, item );
    process_get_arguments( task->process, &args->argc, &args->argv );
    return 0;
}

void* isr80h_command9_exit( struct interrupt_frame* frame ) {
    process_terminate( task_current()->process );
    task_next();
    return 0;
}
