#include "peachos.h"

int peachos_getkey_block() {
    int val;
    do val = peachos_getkey(); while( !val );
    return val;
}

void peachos_terminal_readline( char* out, int max, bool output_while_typing ) {
    int i;
    for( i = 0; i < max - 1; i++ ) {
        // get key
        char key = peachos_getkey_block();

        // handle carriage return
        if( 13 == key ) break;

        // handle backspace
        if( 8 == key ) {
            if( -1 == --i ) continue; // do nothing if @ start of buffer
            if( output_while_typing ) peachos_putchar( 8 ); // write backspace to terminal
            out[i--] = 0; // erase character & move backwards
            continue;
        }

        // handle normal character
        if( output_while_typing ) peachos_putchar( key );
        out[i] = key;
    }
    out[i] = 0;
}
