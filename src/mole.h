// C99
// mole is tool to extract files from memory dumps
#ifndef __MOLE_H__
#define __MOLE_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __WIN32
#  include <windows.h>
#endif

typedef struct {
    char *ptr;
    size_t length;
} MoleSlice;

typedef struct {
    MoleSlice buffer;

#ifdef __WIN32
    HANDLE hFile;
    HANDLE hMap;
#else
    int fd;
#endif

} MoleFileStream;

typedef struct {
    // the constant magic header part.
    // a byte sequence which the main thread
    // keeps searching for and when encountered
    // the respective handler will be invoked.
    char magic[8];

    // the function that handles the matched pattern
    // takes a pointer to the main file buffer
    // and the offset which the specified magic starts in the given file->ptr
    // must return the amount of bytes to skip to match the next pattern
    // can return 0 for error
    size_t (*handle)(MoleSlice *file, size_t start_index);
} MoleMagicHandler;

int MoleFileLoad (MoleFileStream *f, const char *const path);
int MoleFileClose(MoleFileStream *f);

uint32_t MoleReadU32LE(register char *buffer);
uint16_t MoleReadU16LE(register char *buffer);

// PKZIP
size_t MoleHandlePKZIP_LCFH(MoleSlice *file, size_t start_index);
size_t MoleHandlePKZIP_EOCD(MoleSlice *file, size_t start_index);

// PDF
size_t MoleHandlePDF_Header(MoleSlice *file, size_t start_index);
size_t MoleHandlePDF_Footer(MoleSlice *file, size_t start_index);

#endif // __MOLE_H__
