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
        printf( "\n'%s'\n", buf );
        peachos_process_load_start( buf );
    }
    return 0;
}
