#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main( int argc, char** argv ) {
    while( 1 ) {
        // print arguments
        printf( "%i args: ", argc );
        for( int i = 0; i < argc; i++ ) { print( argv[i] ); if( i < argc - 1 ) print( "," ); }
        print( "\n" );

        // long loop
        for( int i = 0; i < 500000000; i++ );
    }

    /*
    // test terminal readline
    print( "What is your name? " );
    char buf[1024];
    peachos_terminal_readline( buf, sizeof( buf ), true );
    print( "\n" );

    // main loop
    while( 1 ) {
        printf( "nice to meet you, %s! Press E to exit, C to crash:\n", buf );

        // wait for keypress
        int key = peachos_getkey_block();
        switch( key ) {
            case 'E': case 'e': print( "Goodbye!\n" ); return 0;
            case 'C': case 'c': { print( "Crash time!\n" ); char* p = NULL; p[0] = 5; break; }
            default: break;
        }
    }
    */
    return 0;
}
