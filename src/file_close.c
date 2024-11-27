#include "mole.h"

#include <stdio.h>

#ifdef __WIN32
#  include <handleapi.h>
#  include <memoryapi.h>
#else
#  include <unistd.h>
#  include <sys/mman.h>
#endif

int MoleFileClose(MoleFileStream *f)
{
    if (f == NULL) {
        fputs("[E] MoleFileClose: NULL input.\n", stderr);
        return 1;
    }
#ifndef __WIN32
    if (munmap(f->buffer.ptr, f->buffer.length) != 0) {
        fputs("[E] MoleFileClose: munmap failed.\n", stderr);
        return 2;
    }

    if (close(f->fd) != 0) {
        fputs("[E] MoleFileClose: close failed.\n", stderr);
        return 3;
    }
#else
    UnmapViewOfFile(f->buffer.ptr);
    CloseHandle(f->hMap);
    CloseHandle(f->hFile);
#endif

    return 0;
}
