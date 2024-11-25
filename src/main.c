#include "mole.h"

#include <stdio.h>

static const MoleMagicHandler handlers[] = {
    // {.magic = {80,75,3,4,0,0,0,0},    .handle = MoleHandlePKZIP_LCFH},
    // {.magic = {80,75,5,6,0,0,0,0},    .handle = MoleHandlePKZIP_EOCD},
    {.magic = {37,80,68,70,45,0,0,0}, .handle = MoleHandlePDF_Header},
    {.magic = {37,37,69,79,70,0,0,0}, .handle = MoleHandlePDF_Footer},
};

int main(int argc, char **argv)
{
    MoleFileStream file;

    if (argc < 2)
        return 1;

    if (MoleFileLoad(&file, argv[1]))
        return 2;

    printf(
        "file.buffer { .ptr= 0x%p, .length= %llu }\n",
        (void*)file.buffer.ptr,
        file.buffer.length
    );

    for(size_t b = 0; b < file.buffer.length; b++) {
        for(
            int h = 0;
            h < (sizeof(handlers)/sizeof(handlers[0]));
            h++
        ) {
            int c = 0;
            while(handlers[h].magic[c] != 0) {
                if(handlers[h].magic[c] != file.buffer.ptr[b + c])
                    goto no_match_for_magic;
                c++;
            }
            printf("\tMATCH b=%llu, c= %d\n", b, c);
            b += handlers[h].handle(&file.buffer, b);
no_match_for_magic:;
        }
    }

    if (MoleFileClose(&file))
        return 3;

    return 0;
}
