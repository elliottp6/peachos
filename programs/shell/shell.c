#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "peachos.h"

int main( int argc, char** argv ) {
    print( "Shell loaded\n" );
    while( 1 ) {
        print( "> " );
        char buf[1024];
        peachos_terminal_readline( buf, sizeof( buf ), true );
        print( "\n" );
        if( peachos_system_run( buf ) < 0 ) printf( "'%s' not found\n", buf );
    }
    return 0;
}
