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
    while( 1 ) { if( 0 != getkey() ) print( "key was pressed\n" ); }
    return 0;
}
