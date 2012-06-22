#include "ntvl.h"
#include "ntvl_keyfile.h"
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

int main(int arc, const char * argv[] ) {
    int e;
    ntvl_cipherspec_t specs[NTVL_MAX_NUM_CIPHERSPECS];

    e = ntvl_read_keyfile( specs, NTVL_MAX_NUM_CIPHERSPECS, "keyctrl.conf" );

    if ( e < 0 ) perror( "Failed to read keyfile" );
    else  fprintf( stderr, "Stored %d keys.\n", e );

    return 0;
}
