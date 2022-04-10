#include "misc.h"
#include "task/task.h"
#include <stddef.h>

// TODO: this is very inefficient way to get stack items, we ideally should do them all in one swoop
void* isr80h_command0_sum( struct interrupt_frame* frame ) {
    int arg1 = (int)task_get_stack_item( task_current(), 1 ),
        arg0 = (int)task_get_stack_item( task_current(), 0 );
    return (void*)(arg0 + arg1);
}
