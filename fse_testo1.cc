#include "./fseo1.h"
#include "./o1const_public.h"

#include <cstdio>  
#include <cstdlib>
#include <cstring> 

int main(int argc, char** argv) {
    const char* filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        printf("Error: Please provide a file path as a command line argument.\n");
        return 1;
    }

    // --- 1. File Reading & Preparation (Original Source) ---
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file: %s\n", filename);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long file_size_long = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size_long <= 0) {
        printf("Error: File is empty or size could not be determined.\n");
        fclose(file);
        return 1;
    }
    size_t fileSize = (size_t)file_size_long;

    void* srcBuffer = malloc(fileSize);
    if (!srcBuffer) {
        printf("Error: Failed to allocate source memory.\n");
        fclose(file);
        return 1;
    }
    fread(srcBuffer, 1, fileSize, file);
    fclose(file);

    // --- 2. Compression ---
    size_t maxCompressedSize = fileSize * 8;
    void* compressedBuffer = malloc(maxCompressedSize);

    if (!compressedBuffer) {
        printf("Error: Failed to allocate compression memory.\n");
        free(srcBuffer);
        return 1;
    }

    size_t compressed_size;
    if(argc == 4) {
        float o0numcap = atof(argv[2]);
        float o1numcap = atof(argv[3]);
        compressed_size = FSEO1_Compress2(compressedBuffer, &compressed_size, srcBuffer, fileSize, FSEO1_DEFAULT_MAX_SYMBOL_VALUE, FSEO1_DEFAULT_TABLELOG, o0numcap, o1numcap, 1);
    } else {
        compressed_size = FSEO1_Compress(compressedBuffer, &compressed_size, srcBuffer, fileSize);
    }

    printf("\n--- Compression Info ---\n");
    printf("Original size: %zu bytes\n", fileSize);
    printf("Compressed size: %zu bytes\n", compressed_size);
    printf("Compression Ratio: %.2f:1\n", (double)fileSize / compressed_size);

}
// g++ --std=c++14 fse_testo1.cc ./fseo1_compress.cc -o fse_testo1