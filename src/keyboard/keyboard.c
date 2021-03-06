#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"
#include <stddef.h>
#include "classic.h"

static struct keyboard *keyboard_list_head = NULL, *keyboard_list_last = NULL;

// insert & initialize the keyboard
int keyboard_insert( struct keyboard* keyboard ) {
    // must have an 'initialize' function
    if( NULL == keyboard->init ) return -EINVARG;

    // insert it into our linked list
    if( keyboard_list_last ) {
        keyboard_list_last->next = keyboard;
        keyboard_list_last = keyboard;
    } else {
        keyboard_list_head = keyboard_list_last = keyboard;
    }

    // initialize the keyboard
    return keyboard->init();
}

// initialize all of the keyboard drivers
void keyboard_init() {
    keyboard_insert( classic_init() );
}

static int keyboard_get_tail_index( struct process* process ) {
    return process->keyboard.tail % sizeof( process->keyboard.buffer );
}

static int keyboard_get_head_index( struct process* process ) {
    return process->keyboard.head % sizeof( process->keyboard.buffer );
}

void keyboard_backspace( struct process* process ) {
    process->keyboard.tail--;
    int real_index = keyboard_get_tail_index( process );
    process->keyboard.buffer[real_index] = 0;
}

// capslock getter/setter
KEYBOARD_CAPS_LOCK_STATE keyboard_get_capslock( struct keyboard* keyboard ) { return keyboard->capslock_state; }
void keyboard_set_capslock( struct keyboard* keyboard, KEYBOARD_CAPS_LOCK_STATE state ) { keyboard->capslock_state = state; }

// pushing a key inputs to just the process which has the FOCUS (process_current)
void keyboard_push( char c ) {
    // don't allow zero to be pushed (or else buffer would be corrupted)
    if( !c ) return;

    // get the current process that has the focus
    struct process* process = process_focused();
    if( !process ) return;

    // push character into process' keyboard buffer
    int real_index = keyboard_get_tail_index( process );
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;
}

// popping a key, however, is done by ANY task (thread)
char keyboard_pop() {
    // get the current process
    struct task* task = task_current();
    if( !task ) return 0;
    struct process* process = task->process;

    // get the keyboard buffer's head
    int real_index = keyboard_get_head_index( process );
    char c = process->keyboard.buffer[real_index];
    if( !c ) return 0; // nothing to pop

    // pop the character
    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head++;
    return c;
}
