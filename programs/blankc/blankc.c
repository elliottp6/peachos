#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main( int argc, char** argv ) {
    // test strtok
    char words[] = "hello how are you";
    const char* token = strtok( words, " " );
    while( token ) {
        printf( "%s\n", token );
        token = strtok( NULL, " " );
    }

    // test malloc
    int* p = (int*)malloc( 4 );
    print( "pointer is: " );
    print( itoa( (int)p ) );
    print( "\n" );
    print( itoa( 4096 ) );
    putchar( 'z' );
    free( p );

    // test terminal readline
    print( "continued! What is your name? " );
    char buf[1024];
    peachos_terminal_readline( buf, sizeof( buf ), true );
    printf( "\nnice to meet you, %s!\n", buf );
    while( 1 ) { if( 0 != peachos_getkey() ) print( "key was pressed\n" ); }
    return 0;
}
