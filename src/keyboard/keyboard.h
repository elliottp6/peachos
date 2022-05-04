#pragma once

// capslock on/off
#define KEYBOARD_CAPS_LOCK_ON 1
#define KEYBOARD_CAPS_LOCK_OFF 0
typedef int KEYBOARD_CAPS_LOCK_STATE;

// forward declarations
struct process;

// function pointer types
typedef int (*KEYBOARD_INIT_FUNCTION)();

// types
struct keyboard {
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    KEYBOARD_CAPS_LOCK_STATE capslock_state;
    struct keyboard* next;
};

// functions
void keyboard_init();
void keyboard_backspace( struct process* process );
void keyboard_push( char c );
char keyboard_pop();

// capslock getter/setter
KEYBOARD_CAPS_LOCK_STATE keyboard_get_capslock( struct keyboard* keyboard );
void keyboard_set_capslock( struct keyboard* keyboard, KEYBOARD_CAPS_LOCK_STATE state );
