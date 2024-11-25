#include "mole.h"

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

// unmaps the given file
int MoleFileClose(MoleFileMem *fm)
{
    if (fm == NULL) {
        fputs("[E] MoleFileClose: NULL input.\n", stderr);
        return 1;
    }

    if (munmap(fm->ptr, fm->length) != 0) {
        fputs("[E] MoleFileClose: munmap failed.\n", stderr);
        return 2;
    }

    if (close(fm->fd) != 0) {
        fputs("[E] MoleFileClose: close failed.\n", stderr);
        return 3;
    }

    return 0;
}
