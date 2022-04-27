#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main( int argc, char** argv ) {
    // test malloc
    char* p = (char*)malloc( 20 );
    strcpy( p, "Welcome to blankc!" );
    printf( "read before free: %s\n", p );
    free( p );
    printf( "read after free: %s\n", p ); // <-- does not trigger crash, but cannot read memory (zeros)
    //p[0] = 'A'; // <-- triggers a crash
    //printf( "write after free: %s\n", p );

    // test strtok
    char words[] = "hello there";
    const char* token = strtok( words, " " );
    while( token ) { printf( "%s,", token ); token = strtok( NULL, " " ); }

    // test terminal readline
    print( "\nWhat is your name? " );
    char buf[1024];
    peachos_terminal_readline( buf, sizeof( buf ), true );
    printf( "\nnice to meet you, %s!\n", buf );

    // loop forever
    while( 1 ) { if( 0 != peachos_getkey() ) print( "key was pressed\n" ); }
    return 0;
}
