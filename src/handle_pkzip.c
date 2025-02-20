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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// the first occurrence of LCFH before an EOCD
// .ptr points to the first byte ('P') of 'PK\3\4'
// and the length is calculated upon reaching an EOCD
static char *lcfh_start = NULL;

// void MoleHandlePKZIP_EOCD(MoleFileMem *file, MoleBuffer *chunk)
size_t MoleHandlePKZIP_EOCD(MoleSlice *file, size_t start_index)
{
    if (start_index >= file->length) {
        puts("start_index >= file->length");
        return 7;
    }

    // char *ptr = file->ptr + start_index;
    uint64_t zip_start_off = 0,
             zip_length = 0,
             eocd_length = 0;

    eocd_length = 22 + MoleReadU16LE(&file->ptr[start_index + 20]); // 22 + comment length

    printf(
        "MoleHandlePKZIP_EOCD(%p [offset=%zu]):\n"
        "\teocd_length= %zu\n"
        ,
        (void*)&file->ptr[start_index], start_index,
        eocd_length
    );

    if (lcfh_start == NULL) {
        puts("No previous LCFH, Probably an empty archive. ignoring...\n\n");
        return eocd_length;
    }

    zip_start_off = (uint64_t)(lcfh_start - file->ptr);
    zip_length    = (uint64_t)((start_index - zip_start_off) + eocd_length + 1);

    printf(
        "PKZip (size= %zu, start= %zu, end= %zu)\n",
        zip_length,
        zip_start_off,
        zip_start_off + zip_length - 1
    );

    char fname[16];
    memset(fname, 0, sizeof(fname));
    sprintf(
        fname, "0x%zx-0x%zx.zip",
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
    if (start_index >= file->length) {
        puts("start_index >= file->length");
        return 7;
    }

    // char *ptr = file->ptr + start_index;

    uint64_t comp_len = MoleReadU32LE(&file->ptr[start_index + 18]);
    uint16_t fname_len = MoleReadU16LE(&file->ptr[start_index + 26]);
    uint64_t lcfh_length = comp_len // compressed data size
                        +  fname_len // file name length
                        +  MoleReadU16LE(&file->ptr[start_index + 28]) // extra field length
                        +  30; // the fields size up to file_name

    printf(
        "MoleHandlePKZIP_LCFH(%p [offset=%zu]):\n"
        "\tlcfh_length= %zu\n"
        "\tfilename= %.*s\n"
        "\tcomp_len= %zu\n\n"
        ,
        (void*)&file->ptr[start_index], start_index,
        lcfh_length,
        fname_len,
        &file->ptr[start_index + 30],
        comp_len
    );

    if ( (lcfh_length+start_index) > file->length ) {
        fputs(
            "[!] lcfh block length exceeds maximum file size. weird!\n",
            stderr
        );
        return lcfh_length - comp_len;
    }

    // not the first LCFH in the archive
    if (lcfh_start != NULL)
        return lcfh_length;

    lcfh_start = &file->ptr[start_index];

    printf(
        "PKZip Start (start= %zu)\n",
        lcfh_start - file->ptr
    );

    return lcfh_length;
}
