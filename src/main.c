#include "mole.h"

#include <stdio.h>

static const MoleMagicHandler handlers[] = {
    {.magic = {80,75,3,4,0,0,0,0},    .handle = MoleHandlePKZIP_LCFH},
    {.magic = {80,75,5,6,0,0,0,0},    .handle = MoleHandlePKZIP_EOCD},
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
        "file.buffer { .ptr= 0x%p, .length= %zu }\n",
        (void*)file.buffer.ptr,
        file.buffer.length
    );

    puts("press any key to read file.");
    getchar();

    for(size_t b = 0; b < file.buffer.length; b++) {
        for(
            int h = 0;
            h < (sizeof(handlers)/sizeof(handlers[0]));
            h++
        ) {
            size_t c = 0;
            while(handlers[h].magic[c] != 0) {
                if(handlers[h].magic[c] != file.buffer.ptr[b + c])
                    goto no_match_for_magic;
                c++;
            }
            printf("MATCH b=%zu, h= %d\n", b, h);
            size_t skip_len = handlers[h].handle(&file.buffer, b);
            printf("skip_len= %zu\n\n", skip_len);
            b += skip_len-1;
            if (b >= file.buffer.length) {
                puts("[*] b >= file.buffer.length");
                return 4;
            }
no_match_for_magic:;
        }
    }

    puts("press any key to close file.");
    getchar();

    if (MoleFileClose(&file))
        return 3;

    puts("press any key to exit.");
    getchar();

    return 0;
}
