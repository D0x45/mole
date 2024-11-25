// C99
// mole is tool to extract files from memory dumps
#ifndef __MOLE_H__
#define __MOLE_H__

#include <stddef.h>
#include <stdint.h>

typedef struct {
    char  *ptr;
    size_t length;
} MoleBuffer;

typedef struct {
    char    *ptr;
    // length must not be greater than 2^31-1
    uint32_t length;
    int32_t  fd;
} MoleFileMem;

typedef struct {
    // the constant magic header part.
    // a byte sequence which the main thread
    // keeps searching for and when encountered
    // the repsective handler will be invoked.
    char magic[8];

    // the function that handles the rest
    void (*handle)();
} MoleMagicHandler;

int MoleFileLoad(MoleFileMem*, const char *const);
int MoleFileClose(MoleFileMem*);

uint32_t MoleReadU32LE(register char *buffer);
uint16_t MoleReadU16LE(register char *buffer);

// PKZIP
void MoleHandlePKZIP_LCFH(MoleFileMem*, MoleBuffer*);
void MoleHandlePKZIP_EOCD(MoleFileMem*, MoleBuffer*);

// PDF
void MoleHandlePDF_Header(MoleFileMem*, MoleBuffer*);
void MoleHandlePDF_Footer(MoleFileMem*, MoleBuffer*);

#endif // __MOLE_H__
