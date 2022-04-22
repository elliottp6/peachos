#include "peachos.h"
#include "stdlib.h"

int main( int argc, char** argv ) {
    print( "hello, how are you? I am blank.c!\n" );
    print( "second syscall from blank.c!\n" );
    int* p = (int*)malloc( 4 );
    if( NULL == p ) print( "malloc returned null\n" ); else print( "malloc returned non-null\n" );
    while( 1 ) {
        if( 0 != getkey() ) print( "key was pressed\n" );
    }
    print( "ready to exit blank.c!\n" );
    return 0;
}
