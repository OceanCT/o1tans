#include "../FiniteStateEntropy/lib/fse.h"
// #include "../FiniteStateEntropy/lib/common/error_public.h" // Needed for FSE_getErrorName
// Although the user used <bits/stdc++.h>, using standard C headers is generally better practice
#include <cstdio>   // For printf, fopen, fclose
#include <cstdlib>  // For malloc, free, size_t
#include <cstring>  // For memcmp

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
    size_t maxCompressedSize = FSE_compressBound(fileSize);
    void* compressedBuffer = malloc(maxCompressedSize);

    if (!compressedBuffer) {
        printf("Error: Failed to allocate compression memory.\n");
        free(srcBuffer);
        return 1;
    }

    size_t compressedSize = FSE_compress(compressedBuffer, maxCompressedSize, srcBuffer, fileSize);

    if (FSE_isError(compressedSize)) {
        printf("Compression error: %s\n", FSE_getErrorName(compressedSize));
        // Cleanup and exit on error
        free(srcBuffer);
        free(compressedBuffer);
        return 1;
    }

    printf("\n--- Compression Info ---\n");
    printf("Original size: %zu bytes\n", fileSize);
    printf("Compressed size: %zu bytes\n", compressedSize);
    printf("Compression Ratio: %.2f:1\n", (double)fileSize / compressedSize);


    // --- 3. Decompression ---
    // The target buffer size for decompression must be the original fileSize
    void* decompressedBuffer = malloc(fileSize);
    
    if (!decompressedBuffer) {
        printf("Error: Failed to allocate decompression memory.\n");
        free(srcBuffer);
        free(compressedBuffer);
        return 1;
    }

    // FSE_decompress takes the destination buffer, max destination size,
    // source buffer (compressed), and source size (compressedSize).
    size_t returnedDecompressedSize = FSE_decompress(
        decompressedBuffer, fileSize,
        compressedBuffer, compressedSize
    );

    if (FSE_isError(returnedDecompressedSize)) {
        printf("\nDecompression error: %s\n", FSE_getErrorName(returnedDecompressedSize));
        // Cleanup and exit on error
        free(srcBuffer);
        free(compressedBuffer);
        free(decompressedBuffer);
        return 1;
    }

    // Check if the decompressed size matches the expected size
    if (returnedDecompressedSize != fileSize) {
        printf("\nVerification FAILED (Size Mismatch):\n");
        printf("Expected size: %zu, Actual decompressed size: %zu\n", fileSize, returnedDecompressedSize);
        // Cleanup and exit on error
        free(srcBuffer);
        free(compressedBuffer);
        free(decompressedBuffer);
        return 1;
    }


    // --- 4. Verification (Byte-by-Byte Comparison) ---
    int comparisonResult = memcmp(srcBuffer, decompressedBuffer, fileSize);

    printf("\n--- Verification Result ---\n");
    if (comparisonResult == 0) {
        printf("SUCCESS: Decompressed data matches original data byte-for-byte.\n");
    } else {
        printf("FAILURE: Decompressed data does NOT match original data (Lossy compression detected).\n");
    }


    // --- 5. Cleanup ---
    free(srcBuffer);
    free(compressedBuffer);
    free(decompressedBuffer);

    return comparisonResult == 0 ? 0 : 1;
}
// g++ --std=c++14 fse_test.cc ../FiniteStateEntropy/lib/*.c -o fse_test