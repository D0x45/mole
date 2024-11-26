// C99
#include "mole.h"

#include <fileapi.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winnt.h>

// the current occurrence of %PDF-
// ptr points to '%' of the '%PDF-'
// and length is calculated when reaching a '%EOF'
static char *pdf_header = NULL;

size_t MoleHandlePDF_Header(MoleSlice *file, size_t start_index)
{
    // %PDF-1.5
    // ^    ^ ^
    // 0    5 7
    char pdf_ver_major = file->ptr[start_index + 5],
         pdf_ver_minor = file->ptr[start_index + 7];

    printf(
        "PDF %c.%c (offset= %llu)\n",
        pdf_ver_major, pdf_ver_minor,
        start_index
    );

    if (
           (pdf_ver_major < '1' || pdf_ver_major > '2')
        || (pdf_ver_minor < '0' || pdf_ver_minor > '9')
    ) {
        fputs("[!] invalid pdf version...\n", stderr);
        return 0;
    }

    if (pdf_header != NULL) {
        fputs(
            "[!] pdf header found before previous one's eof. "
            "possibly corrupted file. "
            "ignoring header...\n",
            stderr
        );
        return 0;
    }

    pdf_header = file->ptr + start_index;
    return 8;
}

size_t MoleHandlePDF_Footer(MoleSlice *file, size_t start_index)
{
    size_t pdf_end = start_index + 5,
           pdf_start = 0,
           pdf_len = 0;

    printf(
        "EOF (offset= %llu)\n",
        start_index
    );

    if (pdf_header == NULL) {
        fputs(
            "[!] pdf footer found before even starting. "
            "possibly corrupted file. "
            "ignoring header...\n",
            stderr
        );
        return 0;
    }

    pdf_start = (pdf_header - file->ptr);
    pdf_len = pdf_end - pdf_start;

    // TODO: improve saving and stuff
    char fname[16];
    memset(fname, 0, sizeof(fname));
    sprintf(
        fname, "0x%llx-0x%llx.pdf",
        pdf_start, pdf_end
    );
    printf(
        "Writing file to disk (start= %llu, end=%llu)...\n",
        pdf_start, pdf_end
    );

#ifdef __WIN32
    // ! apparently using cstdlib's fwrite with a windows memory-mapped buffer
    // ! does not work as intended.
    // ! this took so much of my time to figure out... i hate windows.
    WriteFile(
        CreateFile(fname, GENERIC_WRITE, 0, NULL, 2, 0, 0),
        pdf_header, pdf_len, NULL, NULL
    );
#else
    // TODO: use write and open?
    fwrite(
        pdf_header,
        1, pdf_len,
        fopen(fname, "w")
    );
#endif

    pdf_header = NULL;
    return 5;
}
