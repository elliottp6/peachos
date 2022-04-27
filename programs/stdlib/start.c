#include "peachos.h"

extern int main( int argc, char** argv );

void c_start() {
    // system call to get program arguments
    struct process_arguments args;
    peachos_process_get_arguments( &args );
    
    // pass args to main
    // TODO: do something with the return code
    main( args.argc, args.argv );
}
