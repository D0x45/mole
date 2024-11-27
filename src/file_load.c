#include "mole.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#ifdef __WIN32
#  include <windows.h>
#  include <errhandlingapi.h>
#  include <handleapi.h>
#  include <winnt.h>
#  include <fileapi.h>
#  include <memoryapi.h>
#else
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

int MoleFileLoad(MoleFileStream *f, const char *const path)
{
#ifndef __WIN32
    struct stat sb;
#else
    LARGE_INTEGER tmp;
#endif

    printf("MoleFileLoad(..., path=%s)\n", path);

    if (f == NULL || path == NULL) {
        fputs("[E] MoleFileLoad: NULL input is invalid.\n", stderr);
        return 1;
    }

    // write all zero just in case
    memset(f, 0, sizeof(MoleFileStream));

#ifndef __WIN32
    f->fd = open(path, O_RDONLY);
    if (f->fd < 1) {
        fputs("[E] MoleFileLoad: file descriptor invalid.\n", stderr);
        return 2;
    }

    if (fstat(f->fd, &sb) == -1) {
        fputs("[E] MoleFileLoad: could not query file size.\n", stderr);
        return 3;
    }

    if (sb.st_size > MOLE_FILESIZE_MAX) {
        fputs(
            "[E] MoleFileLoad: file too big. file size must not exceed 1GiB.\n",
            stderr
        );
        return 4;
    }

    f->buffer.length = sb.st_size;
    f->buffer.ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, f->fd, 0);

    if (f->buffer.ptr == MAP_FAILED) {
        close(f->fd);

        f->buffer.ptr = 0;
        f->buffer.length = 0;

        fputs("[E] MoleFileLoad: mmap failed.\n", stderr);
        return 5;
    }
#else
    f->hFile = CreateFile(path,
        GENERIC_READ,          // dwDesiredAccess
        FILE_SHARE_READ,       // dwShareMode
        NULL,                  // lpSecurityAttributes
        OPEN_EXISTING,         // dwCreationDisposition
        FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
        0                      // hTemplateFile
    );

    if (f->hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[E] MoleFileLoad: CreateFile() = %lu\n", GetLastError());
        return 2;
    }

    if (!GetFileSizeEx(f->hFile, &tmp)) {
        fprintf(stderr, "[E] MoleFileLoad: GetFileSizeEx() = %lu\n", GetLastError());
        CloseHandle(f->hFile);
        return 3;
    }

    if (tmp.QuadPart > MOLE_FILESIZE_MAX) {
        fprintf(stderr, "[E] MoleFileLoad: Invalid file size = %zu\n", tmp.QuadPart);
        CloseHandle(f->hFile);
        return 4;
    }

    f->hMap = CreateFileMapping(
        f->hFile,
        NULL,          // Mapping attributes
        PAGE_READONLY, // Protection flags
        0,             // MaximumSizeHigh
        0,             // MaximumSizeLow
        NULL           // Name
    );

    if (f->hMap == 0) {
        fprintf(stderr, "[E] MoleFileLoad: CreateFileMapping() = %lu\n", GetLastError());
        CloseHandle(f->hFile);
        return 5;
    }

    f->buffer.length = tmp.QuadPart;
    f->buffer.ptr = (char*)MapViewOfFile(
        f->hMap,
        FILE_MAP_READ,         // dwDesiredAccess
        0,                     // dwFileOffsetHigh
        0,                     // dwFileOffsetLow
        0                      // dwNumberOfBytesToMap
    );

    if (f->buffer.ptr == NULL) {
        fprintf(stderr, "[E] MoleFileLoad: MapViewOfFile() = %lu\n", GetLastError());
        CloseHandle(f->hFile);
        CloseHandle(f->hMap);
        return 6;
    }

#endif

    return 0;
}
