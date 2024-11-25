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

// the first occurence of LCFH before an EOCD
static MoleBuffer first_lcfh = {
    .length = 0,
    .ptr = NULL
};

void MoleHandlePKZIP_EOCD(MoleFileMem *file, MoleBuffer *chunk)
{
    uint64_t zip_start_off = 0,
             zip_length = 0;

    // Offset of start of central directory,
    // relative to start of archive (or 0xffffffff for ZIP64)
    uint32_t central_start_length = MoleReadU32LE(chunk->ptr + 12);

    // Offset of start of central directory,
    // relative to start of archive (or 0xffffffff for ZIP64)
    uint32_t central_start_offset = MoleReadU32LE(chunk->ptr + 16);

    // Comment length (n)
    uint16_t comment_length = MoleReadU16LE(chunk->ptr + 20);

    chunk->length += 22 + comment_length;

    printf(
        "MoleHandlePKZIP_EOCD(%p [offset=%ld]):\n"
        "\tcentral_start_offset= %u\n"
        "\tcentral_start_length= %u\n"
        "\tcomment_length= %u\n"
        "\tchunk->length= %zu\n"
        ,
        (void*)chunk->ptr,
        chunk->ptr-file->ptr,
        central_start_offset,
        central_start_length,
        comment_length,
        chunk->length
    );

    if (first_lcfh.ptr == NULL) {
        puts("No previous LCFH, Probably an empty archive. ignoring...\n\n");
        return;
    }

    zip_start_off = (uint64_t)(first_lcfh.ptr - file->ptr);
    zip_length = (uint64_t)(chunk->ptr - first_lcfh.ptr + chunk->length);

    printf(
        "PKZip (size= %lu, start= %lu, end= %lu)\n",
        zip_length,
        zip_start_off,
        zip_start_off + zip_length - 1
    );

    // TODO: write the file to disk
    // TODO: improve
    char fname[16];
    memset(fname, 0, sizeof(fname));
    sprintf(
        fname, "0x%lx-0x%lx.zip",
        zip_start_off,
        zip_start_off + zip_length - 1
    );
    puts("Writing file to disk...\n");
    fwrite(
        file->ptr + zip_start_off,
        1, zip_length,
        fopen(fname, "w")
    );

    // reset the starting LCFH
    first_lcfh.length = 0;
    first_lcfh.ptr = NULL;
}

void MoleHandlePKZIP_LCFH(MoleFileMem *file, MoleBuffer *chunk)
{
    uint32_t comp_size     = MoleReadU32LE(chunk->ptr + 18);
    uint32_t uncomp_size   = MoleReadU32LE(chunk->ptr + 22);
    uint16_t file_name_len = MoleReadU16LE(chunk->ptr + 26);
    uint16_t extra_fld_len = MoleReadU16LE(chunk->ptr + 28);

    chunk->length = 30 + file_name_len + extra_fld_len + comp_size;

    printf(
        "MoleHandlePKZIP_LCFH(%p [offset=%ld]):\n"
        "\tcomp_size= %u\n"
        "\tuncomp_size= %u\n"
        "\tfile_name_len= %u\n"
        "\textra_fld_len= %u\n"
        "\tchunk->length= %zu\n\n"
        ,
        (void*)chunk->ptr,
        chunk->ptr-file->ptr,
        comp_size,
        uncomp_size,
        file_name_len,
        extra_fld_len,
        chunk->length
    );

    // not the first LCFH in the archive
    if (first_lcfh.ptr != NULL)
        return;

    first_lcfh.ptr = chunk->ptr;
    first_lcfh.length = chunk->length;

    printf(
        "PKZip Start (start= %lu)\n",
        first_lcfh.ptr - file->ptr
    );
}
