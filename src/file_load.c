#include "mole.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

// maps the given file to memory
int MoleFileLoad(MoleFileMem *fm, const char *const path)
{
    struct stat sb;

    if (fm == NULL || path == NULL) {
        fputs("[E] MoleFileLoad: NULL input is invalid.\n", stderr);
        return 1;
    }

    fm->fd = open(path, O_RDONLY);
    if (fm->fd < 1) {
        fputs("[E] MoleFileLoad: file descriptor invalid.\n", stderr);
        return 2;
    }

    if (fstat(fm->fd, &sb) == -1) {
        fputs("[E] MoleFileLoad: could not query file size.\n", stderr);
        return 3;
    }

    if (sb.st_size > 1073741824) {
        fputs("[E] MoleFileLoad: file too big. file size must not exceed 1GiB.\n", stderr);
        return 4;
    }

    fm->length = sb.st_size;
    fm->ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fm->fd, 0);

    if (fm->ptr == MAP_FAILED) {
        close(fm->fd);
        fm->fd = -1;
        fm->ptr = 0;
        fm->length = 0;

        fputs("[E] MoleFileLoad: mmap failed.\n", stderr);
        return 5;
    }

    return 0;
}
