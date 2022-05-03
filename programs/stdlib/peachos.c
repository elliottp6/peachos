#include "peachos.h"
#include "string.h"

struct command_argument* peachos_parse_command( const char* command, int max ) {
    // get 1st arg
    char scommand[1024];

    // sanity check 'max'
    if( max > (int)sizeof( scommand ) ) return NULL;

    // copy 'command' => 'scommand'
    strncpy( scommand, command, sizeof( scommand ) );

    // split string by spaces
    char* token = strtok( scommand, " " );
    if( !token ) return NULL;

    // allocate command argument
    struct command_argument* root_command = peachos_malloc( sizeof( struct command_argument ) );
    if( !root_command ) return NULL;

    // copy the token
    strncpy( root_command->argument, token, sizeof( root_command->argument ) );
    root_command->next = NULL;

    // parse next tokens
    struct command_argument* current = root_command;
    token = strtok( NULL, " " );
    while( token ) {
        // allocate next command
        struct command_argument* new_command = peachos_malloc( sizeof( struct command_argument ) );
        if( !new_command ) break;
        
        //
        strncpy( new_command->argument, token, sizeof( root_command->argument ) );
        new_command->next = NULL;
        current->next = new_command;
        current = new_command;
        token = strtok( NULL, " " );
    }

    // done
    return root_command;
}

int peachos_getkey_block() {
    int val;
    do val = peachos_getkey(); while( !val );
    return val;
}

void peachos_terminal_readline( char* out, int max, bool output_while_typing ) {
    int i;
    for( i = 0; i < max - 1; i++ ) {
        // get key
        char key = peachos_getkey_block();

        // handle carriage return
        if( 13 == key ) break;

        // handle backspace
        if( 8 == key ) {
            if( -1 == --i ) continue; // do nothing if @ start of buffer
            if( output_while_typing ) peachos_putchar( 8 ); // write backspace to terminal
            out[i--] = 0; // erase character & move backwards
            continue;
        }

        // handle normal character
        if( output_while_typing ) peachos_putchar( key );
        out[i] = key;
    }
    out[i] = 0;
}

int peachos_system_run( const char* command ) {
    // copy command into buffer TODO: why does the lecture do this? seems unnecessary
    char buffer[1024];
    strncpy( buffer, command, sizeof( buffer ) );

    // parse the command
    struct command_argument* root_arg = peachos_parse_command( buffer, sizeof( buffer ) );
    if( !root_arg ) return -1;
    
    // run it
    return peachos_system( root_arg );
}
