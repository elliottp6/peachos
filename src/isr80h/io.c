#include "io.h"
#include "task/task.h"
#include "kernel.h"

void* isr80h_command1_print( struct interrupt_frame* frame ) {
    // get 1st argument
    void* user_space_message_buffer = task_get_stack_item( task_current(), 0 );

    // read buffer from userspace & copy into kernel space
    char buf[1024];
    copy_string_from_task( task_current(), user_space_message_buffer, buf, sizeof( buf ) );

    // print the buffer
    print( buf );
    return 0;
}
