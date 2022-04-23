#include "peachos.h"

int peachos_getkey_block() {
    int val;
    do val = peachos_getkey(); while( !val );
    return val;
}
