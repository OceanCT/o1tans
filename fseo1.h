/*
    adding order1 feature to FSE compression(tans)
*/
#include "o1error_public.h"
#include <stddef.h>


size_t FSEO1_Compress(void* dst, size_t* dstSize,
                   const void* src, size_t srcSize);

size_t FSEO1_Compress2(void* dst, size_t* dstSize,
                   const void* src, size_t srcSize,
                    unsigned maxSymbolValue, unsigned tableLog, float o0numcap, float o1numcap, int symbollength);

size_t FSEO1_Decompress(void* dst, size_t dstSize,
                     const void* cSrc, size_t cSrcSize);
