#include "stdio.h"
#include "peachos.h"
#include "stdlib.h"
#include <stdarg.h>

int putchar( int c ) {
    peachos_putchar( (char)c );
    return 0;
}

int printf( const char* fmt, ... ) {
    // begin variable argument list
    va_list ap;
    va_start( ap, fmt );

    // for each character in fmt
    for( const char* p = fmt; *p; p++ ) {
        // if it's a regular character, just print it
        if( '%' != *p ) { putchar( *p ); continue; }

        // otherwise: parse next character
        switch( *++p ) {
            case 'i': {
                int ival = va_arg( ap, int );
                print( itoa( ival ) );
                break;
            }

            case 's': {
                char* sval = va_arg( ap, char* );
                print( sval );
                break;
            }

            default: {
                putchar( *p );
                break;
            }
        }
    }

    // end variable argument list
    va_end( ap );
    return 0;
}
