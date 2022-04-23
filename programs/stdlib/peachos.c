#include "peachos.h"

int peachos_getkey_block() {
    int val;
    do val = peachos_getkey(); while( !val );
    return val;
}

void peachos_terminal_readline( char* out, int max, bool output_while_typing ) {
    int i;
    for( i = 0; i < max - 1; i++ ) {
        char key = peachos_getkey_block();
        if( 13 == key ) break; // carriage return
        if( output_while_typing ) peachos_putchar( key );
        if( 8 == key && i >= 1 ) { out[i - 1] = 0; i-=2; continue; } // backspace
        out[i] = key;
    }
    out[i] = 0;
}
