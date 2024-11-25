// C99
#include "mole.h"

#include <bits/stdint-uintn.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// the first occurence of %PDF-
static MoleBuffer current_pdf = {
    .length = 0,
    .ptr = NULL
};

void MoleHandlePDF_Header(MoleFileMem *file, MoleBuffer *chunk)
{
    // %PDF-1.5
    // ^    ^ ^
    // 0    5 7
    char pdf_ver_major = chunk->ptr[5] - '0',
         pdf_ver_minor = chunk->ptr[7] - '0';

    printf(
        "PDF %d.%d (offset= %ld)\n",
        pdf_ver_major, pdf_ver_minor,
        chunk->ptr - file->ptr
    );

    if (current_pdf.ptr != NULL) {
        puts(
            "[!] pdf header found before previous one's eof. "
            "possibly corrupted file. "
            "ignoring header...\n"
        );
        return;
    }

    current_pdf.ptr = chunk->ptr;
}

void MoleHandlePDF_Footer(MoleFileMem *file, MoleBuffer *chunk)
{
    uint64_t pdf_start_off = 0;

    if (current_pdf.ptr == NULL) {
        puts(
            "[!] pdf footer found with no preceding header! "
            "possibly corrupted file. "
            "ignoring footer...\n"
        );
        return;
    }

    current_pdf.length = chunk->ptr - file->ptr + chunk->length;
    pdf_start_off = current_pdf.ptr - file->ptr;

    printf(
        "EOF (start= %ld, size= %ld)\n",
        pdf_start_off,
        current_pdf.length
    );

    // TODO: improve saving and stuff
    char fname[16];
    memset(fname, 0, sizeof(fname));
    sprintf(
        fname, "0x%lx-0x%lx.pdf",
        pdf_start_off,
        pdf_start_off + current_pdf.length
    );
    puts("Writing file to disk...\n");
    fwrite(
        current_pdf.ptr,
        1, current_pdf.length,
        fopen(fname, "w")
    );

    current_pdf.length = 0;
    current_pdf.ptr = NULL;
}
