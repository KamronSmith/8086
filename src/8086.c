#include "parse.h"
#include "kam.h"
#ifdef __APPLE__
    #include <machine/endian.h>
#else
    #include <endian.h>
#endif

int main(int argc, char **argv) {

    // if (argc < 2) {
    //     printf("Please enter a binary file\n");
    //     exit(1);
    // }

    // char *filepath = argv[1];

    UNUSED(argc);
    UNUSED(argv);
    
    FILE *f = fopen("bin/listing_41", "rb");
    
    if (f == NULL) {
        printf("File unavailable\n");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    u8 filesize = (u8)ftell(f);
    fseek(f, 0, SEEK_SET);
    
    u8 buf[filesize];
    size_t size = fread(buf, 1, filesize, f);
    UNUSED(size);

    parse_instruction(buf, size);

    // 111111 11 11 111 111 11111111 11111111
    // ------ -- -- --- ---
    // mod    dw mo reg rm

    fclose(f);
}
