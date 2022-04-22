#include "peachos.h"

int main( int argc, char** argv ) {
    print( "hello, how are you? I am blank.c!\n" );
    print( "second syscall from blank.c!\n" );
    while( 1 ) {
        if( 0 != getkey() ) print( "key was pressed\n" );
    }
    print( "ready to exit blank.c!\n" );
    return 0;
}
