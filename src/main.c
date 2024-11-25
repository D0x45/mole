#include <stdio.h>

#include "mole.h"

static const MoleMagicHandler handlers[] = {
    {.magic = {80,75,3,4,0,0,0,0}, .handle = MoleHandlePKZIP_LCFH },
    {.magic = {80,75,5,6,0,0,0,0}, .handle = MoleHandlePKZIP_EOCD },
    {.magic = {37,80,68,70,45,0,0,0}, .handle = MoleHandlePDF_Header},
    {.magic = {37,37,69,79,70,0,0,0}, .handle = MoleHandlePDF_Footer},
};

int main(int argc, char **argv)
{
    MoleFileMem fm;
    MoleBuffer chunk = { .ptr = NULL, .length = 0 };

    if (argc < 2)
        return 1;

    if (MoleFileLoad(&fm, argv[1]))
        return 2;

    for(int b = 0; b < fm.length; b++) {
        for(
            int h = 0;
            h < (sizeof(handlers)/sizeof(handlers[0]));
            h++
        ) {
            int c = 0;
            while(handlers[h].magic[c] != 0) {
                if(handlers[h].magic[c] != fm.ptr[b + c])
                    goto no_match_for_magic;
                c++;
            }
            chunk.length = c;
            chunk.ptr = &fm.ptr[b];
            handlers[h].handle(&fm, &chunk);
            b += chunk.length-1;
no_match_for_magic:;
        }
    }

    if (MoleFileClose(&fm))
        return 3;

    return 0;
}
