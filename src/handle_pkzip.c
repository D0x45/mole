// C99
//
// https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
//
// all this implementation does is:
// take all LocalFileHeaders (LCFH)
// and also all EndOfCentralDirectory (EOCD).
// each zip file has an EOCD at the end.
// and before every EOCD there are multiple LCFHs.
// there may be no LCFHs before an EOCD which
// indicates an empty zip file.
//
// [LCFH][LCFH][LCFH]...[EOCD]
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^
//        a PKZip file
#include "mole.h"

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// the first occurrence of LCFH before an EOCD
// .ptr points to the first byte ('P') of 'PK\0\4'
// and the length is calculated upon reaching an EOCD
static char *lcfh_start = NULL;

// void MoleHandlePKZIP_EOCD(MoleFileMem *file, MoleBuffer *chunk)
size_t MoleHandlePKZIP_EOCD(MoleSlice *file, size_t start_index)
{
    char *ptr = file->ptr + start_index;
    uint64_t zip_start_off = 0,
             zip_length = 0,
             eocd_length = 0;

    eocd_length = 22 + MoleReadU16LE(ptr + 20); // 22 + comment length

    printf(
        "MoleHandlePKZIP_EOCD(%p [offset=%llu]):\n"
        "\teocd_length= %llu\n"
        ,
        (void*)ptr, start_index,
        eocd_length
    );

    if (lcfh_start == NULL) {
        puts("No previous LCFH, Probably an empty archive. ignoring...\n\n");
        return eocd_length;
    }

    zip_start_off = (uint64_t)(lcfh_start - file->ptr);
    zip_length = (uint64_t)(zip_start_off + start_index + eocd_length + 1);

    printf(
        "PKZip (size= %llu, start= %llu, end= %llu)\n",
        zip_length,
        zip_start_off,
        zip_start_off + zip_length - 1
    );

    char fname[16];
    memset(fname, 0, sizeof(fname));
    sprintf(
        fname, "0x%llx-0x%llx.zip",
        zip_start_off,
        zip_start_off + zip_length - 1
    );
    puts("Writing file to disk...\n");
#ifdef __WIN32
    WriteFile(
        CreateFile(fname, GENERIC_WRITE, 0, NULL, 2, 0, 0),
        lcfh_start, zip_length, NULL, NULL
    );
#else
    fwrite(
        lcfh_start,
        1, zip_length,
        fopen(fname, "w")
    );
#endif

    // reset the starting LCFH
    lcfh_start = NULL;
    return eocd_length;
}

size_t MoleHandlePKZIP_LCFH(MoleSlice *file, size_t start_index)
{
    char *ptr = file->ptr + start_index;

    uint64_t lcfh_length = MoleReadU32LE(ptr + 18) // compressed data size
                        +  MoleReadU16LE(ptr + 26) // file name length
                        +  MoleReadU16LE(ptr + 28) // extra field length
                        +  30; // the fields size up to file_name

    printf(
        "MoleHandlePKZIP_LCFH(%p [offset=%llu]):\n"
        "\tlcfh_length= %llu\n\n"
        ,
        (void*)ptr, start_index,
        lcfh_length
    );

    // not the first LCFH in the archive
    if (lcfh_start != NULL)
        return lcfh_length;

    lcfh_start = ptr;

    printf(
        "PKZip Start (start= %lld)\n",
        lcfh_start - file->ptr
    );

    return lcfh_length;
}
