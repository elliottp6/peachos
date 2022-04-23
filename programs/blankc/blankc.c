#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"

int main( int argc, char** argv ) {
    print( "hello, how are you? I am blank.c!\n" );
    int* p = (int*)malloc( 4 );
    print( "pointer is: " );
    print( itoa( (int)p ) );
    print( "\n" );
    print( itoa( 4096 ) );
    putchar( 'z' );
    free( p );
    print( "freed the pointer\n" );
    printf( "My age is %i\n", 98 );
    print( "press any key to continue\n" );
    peachos_getkey_block();
    print( "continued! What is your name? " );
    char buf[1024];
    peachos_terminal_readline( buf, sizeof( buf ), true );
    printf( "\nnice to meet you, %s!\n", buf );
    while( 1 ) { if( 0 != peachos_getkey() ) print( "key was pressed\n" ); }
    return 0;
}
