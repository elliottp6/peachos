#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main( int argc, char** argv ) {
    // print arguments
    printf( "%i args: ", argc );
    for( int i = 0; i < argc; i++ ) { print( argv[i] ); if( i < argc - 1 ) print( "," ); }
    print( "\n" );

    // test malloc
    char *p = (char*)malloc( 20 );
    free( p );
    printf( "print-after-free does not pagefault (bug in task.c:copy_string_from_task): %s\n", p );

    // test parse command
    char str[] = "hello world";
    struct command_argument* arg = peachos_parse_command( str, sizeof( str ) );
    while( arg ) {
        printf( "%s,", arg->argument );
        arg = arg->next;
    }

    // test strtok
    /*char words[] = "hello there";
    const char* token = strtok( words, " " );
    while( token ) { printf( "%s,", token ); token = strtok( NULL, " " ); }*/

    // test terminal readline
    print( "\nWhat is your name? " );
    char buf[1024];
    peachos_terminal_readline( buf, sizeof( buf ), true );
    printf( "\nnice to meet you, %s!\n", buf );

    // loop forever
    while( 1 ) { if( 0 != peachos_getkey() ) print( "key was pressed\n" ); }
    return 0;
}
