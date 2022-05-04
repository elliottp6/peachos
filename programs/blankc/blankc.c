#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main( int argc, char** argv ) {
    // print arguments
    printf( "%i args: ", argc );
    for( int i = 0; i < argc; i++ ) { print( argv[i] ); if( i < argc - 1 ) print( "," ); }
    print( "\n" );

    // test terminal readline
    print( "What is your name? " );
    char buf[1024];
    peachos_terminal_readline( buf, sizeof( buf ), true );
    printf( "\nnice to meet you, %s!\n", buf );

    // loop forever
    while( 1 ) {
        if( 0 != peachos_getkey() ) {
            print( "key was pressed\n" );

            // cause a page fault
            char* p = NULL;
            p[0] = 5;
        }
    }
    return 0;
}
