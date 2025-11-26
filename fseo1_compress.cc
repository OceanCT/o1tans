#include "fseo1.h"
#include "o1const_public.h"
#include <bits/stdc++.h>

/*
    assume o0count and o1count has been initialized
    o0count[n] : count of symbol n in order-0
    o1count[n][n] : count of symbol n in order-1
*/
void O1_HIST(const void* src, size_t srcSize, unsigned* o0count, unsigned* o1count, unsigned maxSymbolValue, unsigned symbolLength) {
    assert(symbolLength == 1); // for now only support 1 byte symbol
    const unsigned char* p = (const unsigned char*)src;
    const unsigned char* const pEnd = p + srcSize;
    unsigned prevSymbol = 0;
    while (p <= pEnd - symbolLength) {
        unsigned symbol = 0;
        for (unsigned i = 0; i < symbolLength; ++i) {
            symbol |= ((unsigned)(*p++)) << (i * 8);
        }
        assert(symbol <= maxSymbolValue);
        o0count[symbol]++;
        o1count[prevSymbol * (maxSymbolValue + 1) + symbol]++;
        prevSymbol = symbol;
    }
} 

/*
    rejust the number according to tablelog
    make sure the normalized count sum to 2^tablelog
    and each symbol's count is calculated according to o0count and totalCount (o0count / totalCount)
    return sizeof(uint16_t) * (maxSymbolValue + 1) number of symbols
*/
size_t normalizeCountO0(unsigned* o0count, unsigned maxSymbolValue, int totalCount, int tablelog, uint16_t* normalizedCount) {
    assert(tablelog <= 16); 
    const unsigned long long K = 1ULL << tablelog;

    const unsigned long long totalCount_ull = (unsigned long long)totalCount;

    unsigned long long sum = 0;
    unsigned symbol;

    for (symbol = 0; symbol <= maxSymbolValue; symbol++) {
        // Calculate: (o0count[symbol] * K) / totalCount
        // Use 64-bit arithmetic for the product to prevent potential 32-bit overflow.
        unsigned long long scaled_count_ull = ((unsigned long long)o0count[symbol] * K) / totalCount_ull;

        // Store the result as uint16_t (since K fits in 16 bits, the scaled count will too)
        normalizedCount[symbol] = (uint16_t)scaled_count_ull;

        sum += scaled_count_ull;
    }
    // adjust for those symbols whose normalized count is 0 but actually occurred
    for (symbol = 0; symbol <= maxSymbolValue; symbol++) {
        if((!normalizedCount[symbol]) && o0count[symbol]) {
            normalizedCount[symbol] = 1;
            sum++;
        }
    }

    unsigned long long deficit = K - sum;

    for (symbol = 0; symbol <= maxSymbolValue; symbol++) {
        if (deficit == 0) {
            break; 
        }
        normalizedCount[symbol]++;
        deficit--;
    }

    return sizeof(uint16_t) * (maxSymbolValue + 1);
}

// rejust the number according to tablelog and o1count
// notice that we only calculate o1numcap for those symbols whose occurrence exceeds the cap * totalCount of current o1 table 
// write these symbols plus their counts into header in such manner:
// [which record way to use(one byte)][record symbol num (one byte)]
// [symbol(one byte)] [normalized count (two bytes)]
// [symbol(one byte)] [normalized count (two bytes)]
// ... [symbol(one byte)] [normalized count (two bytes)]
// for the left 2^tablelog - normalized count sum, we distribute them accoring to the the counts calculated from o0count
// finally, if using the header size actually exceeds sizeof(uint16_t) * (maxSymbolValue + 1)
// we change the header to just store the output of normalized_count and set the record way flag to 0
// otherwise set the record way flag to 1
// notice we never fall back to only use o0 count unless all symbols' occurrence are below the cap
// return the size of header 
size_t normalizeCountO1(unsigned* o0count, unsigned* o1count, unsigned maxSymbolValue, float o1numcap, int o0totalCount, int tablelog, uint16_t* normalized_count, unsigned** header) {
    for(unsigned i = 0; i <= maxSymbolValue; i++) {
        normalized_count[i] = 0;
    }
    const unsigned N = 1U << tablelog;
    const size_t numSymbols = maxSymbolValue + 1;
    unsigned i;

    
    // --- Step 0: Calculate the full O1 total count for thresholding (FIXED) ---
    unsigned o1_full_totalCount = 0;
    for (i = 0; i < numSymbols; i++) {
        o1_full_totalCount += o1count[i];
    }
    
    // The threshold is the cap * totalCount of current o1 table.
    const unsigned threshold = (o1_full_totalCount > 0)
                             ? (unsigned)round(o1numcap * (double)o1_full_totalCount)
                             : 0;
    printf("O1 normalization threshold (raw count): %u\n", threshold);
    // --- Pass 1: Identify and Sum O1 Candidates ---
    // Store indices and counts of symbols that will use the O1 model
    // Using 2 uint16_t (4 bytes) per candidate: symbol index (uint16_t) and raw count (uint16_t).
    uint16_t* o1_candidates_data = (uint16_t*)malloc(numSymbols * 2 * sizeof(uint16_t)); 
    if (o1_candidates_data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for O1 candidates data.\n");
        return 0;
    }
    unsigned o1_candidates_count = 0;
    unsigned o0_remaining_total = 0;
    uint8_t o1_candidates[maxSymbolValue] = {0};
    
    for (i = 0; i < numSymbols; i++) {
        if (o1count[i] >= threshold) {
            // Store symbol index and raw count
            o1_candidates_data[o1_candidates_count * 2] = i;              // symbol
            o1_candidates_data[o1_candidates_count * 2 + 1] = o1count[i]; // raw count
            o1_candidates_count++;
            o1_candidates[i] = 1; // mark as O1 candidate
        } else {
            o0_remaining_total += o0count[i];
        }
    }
    // print o1 candidates 
    // for(i = 0; i < o1_candidates_count; i++) {
        // unsigned symbol = o1_candidates_data[i * 2];
        // unsigned raw_count = o1_candidates_data[i * 2 + 1];
        // printf("O1 Candidate Symbol: %u, Raw Count: %u\n", symbol, raw_count);
    // }
    // int o1_tmp_cnt = 0;
    // for(i = 0; i < numSymbols; i++) {
        // printf("Symbol: %u, O0 Count: %u, O1 Count: %u, threshold: %u, o1_tmp_cnt: %d\n", i, o0count[i], o1count[i], threshold, o1_tmp_cnt);
    //     o1_tmp_cnt += o1count[i];
    // }
    // --- Pass 2: Calculate O1 Normalized Counts and Build Sparse Header Buffer (Draft) ---
    
    // Max compact header size: flag(1B) + count(1B) + maxSymbolValue * (symbol(1B) + count(2B))
    // To handle byte packing, we will use a byte array for the draft header.
    const size_t max_sparse_header_bytes = 2 + numSymbols * 3;
    uint8_t* sparse_header_draft = (uint8_t*)malloc(max_sparse_header_bytes);
    if (sparse_header_draft == NULL) {
        free(o1_candidates_data);
        fprintf(stderr, "Error: Memory allocation failed for sparse_header_draft.\n");
        return 0;
    }
    size_t header_offset = 2; // Reserve 2 bytes for [record way][record symbol num]
    unsigned o1_norm_total = 0;

    for (i = 0; i < o1_candidates_count; i++) {
        unsigned symbol = o1_candidates_data[i * 2];
        unsigned raw_count = o1_candidates_data[i * 2 + 1];

        // Normalize O1 count (scaled by N)
        // Use 64-bit math for precision before dividing
        // Rounding to nearest integer
        unsigned norm_count = (unsigned)(raw_count * N / o1_full_totalCount);
    
        
        if (norm_count > 0) {
            normalized_count[symbol] = (uint16_t)norm_count;
            o1_norm_total += norm_count;

            // Write symbol (one byte)
            sparse_header_draft[header_offset++] = (uint8_t)symbol;
            
            // Write normalized count (two bytes, Little Endian assumed for simplicity)
            // Assuming uint16_t storage: low byte, high byte
            sparse_header_draft[header_offset++] = (uint8_t)(norm_count & 0xFF);
            sparse_header_draft[header_offset++] = (uint8_t)((norm_count >> 8) & 0xFF);
        } else {
            normalized_count[symbol] = 1; 
            o1_norm_total += 1;
            // Write symbol (one byte)
            sparse_header_draft[header_offset++] = (uint8_t)symbol;
            // Write normalized count (two bytes, Little Endian assumed for simplicity)
            sparse_header_draft[header_offset++] = 1 & 0xFF;
            sparse_header_draft[header_offset++] = (1 >> 8) & 0xFF;
        }
    }
    
    

    
    // Set the number of recorded symbols
    unsigned o1_symbols_written = (header_offset - 2) / 3;
    sparse_header_draft[1] = (uint8_t)o1_symbols_written;
    
    // --- Pass 3: O0 Fallback Distribution (for remaining slots) ---
    // printf("O1 normalization done. O1 normalized total: %u. Distributing remaining slots using O0 counts...\n", o1_norm_total);
    unsigned remaining_slots = N - o1_norm_total;
    // printf("N: %d, o1_norm_total: %d, accumulate count: %d\n", N, o1_norm_total, std::accumulate(normalized_count, normalized_count + numSymbols, 0u));
    // Note: current_total (o1_norm_total) is implicitly used via remaining_slots

    // remaining slots should be able to fill all used o0 counts 
    // printf("Remaining slots before O0 distribution: %u\n", remaining_slots);
    // printf("O0 remaining total count: %u\n", o0_remaining_total);
    // Distribute remaining slots using O0 counts for non-O1 symbols
    if (remaining_slots > 0 && o0_remaining_total > 0) {
        unsigned deficit = remaining_slots;
        // check all used normalized o0 count 
        for(unsigned symbol = 0; symbol <= maxSymbolValue; symbol++) {
            if(o0count[symbol] > 0 && normalized_count[symbol] == 0) {
                normalized_count[symbol] = 1;
                deficit--;
            }
        }
    
        // Simple proportional distribution
        for (i = 0; i < numSymbols; i++) {
            if(o1_candidates[i]) continue; // skip those symbols already used in o1 table
            uint16_t norm_count = (uint16_t)(o0count[i] * remaining_slots / o0_remaining_total);

            if (norm_count > 0) {
                // Check against remaining deficit
                if (norm_count > deficit) {
                    norm_count = deficit;
                }
                normalized_count[i] = norm_count;
                deficit -= norm_count - 1;
            }
        }

        assert(deficit >= 0);
        // Final correction for rounding error: distribute any remaining deficit (should be small)
        if (deficit > 0) {
            // Find the symbol with the largest O0 count that was NOT used in O1
            unsigned max_o0_symbol = 0;
            unsigned max_o0_count = 0;
            // First loop to find the best candidate for the deficit
            for (i = 0; i < numSymbols; i++) {
                 // Check if the symbol used O0-based count (o1count[i] <= threshold)
                 if (o1count[i] <= threshold && o0count[i] > max_o0_count) {
                    max_o0_count = o0count[i];
                    max_o0_symbol = i;
                 }
            }
            
            // If the best O0 count candidate is not suitable or all O0 counts were zero, 
            // find the first available slot that was intended for O0.
            if (max_o0_count == 0) {
                for (i = 0; i < numSymbols; i++) {
                    // Find the first symbol that is not fully claimed by O1 (i.e., its O1 count was <= threshold)
                    // and its normalized count is less than N
                    if (o1count[i] <= threshold && normalized_count[i] < N) {
                         max_o0_symbol = i;
                         break;
                    }
                }
            }
            
            // Add the deficit to the chosen symbol
            if (normalized_count[max_o0_symbol] + deficit <= N) {
                normalized_count[max_o0_symbol] += (uint16_t)deficit;
            } else {
                // If even the final correction overflows, distribute the remaining deficit
                // to available symbols sequentially.
                for (i = 0; i < numSymbols && deficit > 0; i++) {
                     if (normalized_count[i] < UINT16_MAX) { // Check for max value
                         unsigned increment = (deficit < (UINT16_MAX - normalized_count[i])) ? deficit : (UINT16_MAX - normalized_count[i]);
                         normalized_count[i] += increment;
                         deficit -= increment;
                     }
                }
            }
        }
    } else {
                while(remaining_slots > 0) {
            for(int i = 0; i < o1_candidates_count; i++) {
                unsigned symbol = o1_candidates_data[i * 2];
                if(remaining_slots == 0) break;
                else {
                    normalized_count[symbol]++;
                    remaining_slots--;
                }
            }
        }
    }
    free(o1_candidates_data); // Done with intermediate data
    // printf("remaining slots after O0 distribution: %d, %d\n", N - std::accumulate(normalized_count, normalized_count + numSymbols, 0u), remaining_slots);
    // if there are still remaining slots, distribute them using o0 table 


    // extra checking stage 
    for(unsigned symbol = 0; symbol <= maxSymbolValue; symbol++) {
        if(o1count[symbol]) assert(normalized_count[symbol] > 0);
    }
    
    // --- Pass 4: Final Header Assembly and Size Check ---
    
    // Calculate size of the dense (full) header in bytes
    const size_t dense_header_size = sizeof(uint16_t) * numSymbols; 
    
    // Calculate size of the sparse header draft
    const size_t sparse_header_size = header_offset;

    // Check if the sparse header is smaller than the dense header
    if (sparse_header_size < dense_header_size) {
        // Option 1: Use the compact, sparse header
        sparse_header_draft[0] = 1; // Record way flag set to 1 (sparse)
        
        // Copy the relevant part of the draft into the final header pointer
        *header = (unsigned*)malloc(sparse_header_size);
        if (*header == NULL) {
             free(sparse_header_draft);
             fprintf(stderr, "Error: Memory allocation failed for final sparse header.\n");
             return 0;
        }
        memcpy(*header, sparse_header_draft, sparse_header_size);
        free(sparse_header_draft);
        return sparse_header_size;
    } else {
        // Option 2: Fall back to the dense header (just normalized_count array)
        
        // Allocate space for the full dense header
        *header = (unsigned*)malloc(dense_header_size);
        if (*header == NULL) {
             free(sparse_header_draft);
             fprintf(stderr, "Error: Memory allocation failed for final dense header.\n");
             return 0;
        }
        
        // Copy the normalized counts into the header memory block
        memcpy(*header, normalized_count, dense_header_size);
        
        // The record way flag (0) is implicitly known if the returned size matches the dense size.
        // The prompt asks the header to *just* store the normalized_count.
        
        free(sparse_header_draft);
        return dense_header_size;
    }
}

size_t FSEO1_Compress2(void* dst, size_t* dstSize,
                    const void* src, size_t srcSize,
                    unsigned maxSymbolValue, unsigned tableLog, float o0numcap, float o1numcap, int symbollength) {
    printf("FSEO1_Compress2 called with maxSymbolValue=%u, tableLog=%u, o0numcap=%f, o1numcap=%f, symbollength=%d\n", maxSymbolValue, tableLog, o0numcap, o1numcap, symbollength);
    size_t output_size = sizeof(unsigned) + sizeof(unsigned);
    // if all counters are used, the table should look like:
    // [N] o0 table 
    // [0] [N] o1 table of symbol 0
    // [1] [N] o1 table of symbol 1
    // ...
    // [N] [N] o1 table of symbol N
    // since max tablelog size is 11 bits, each counter should be able to store in 2 bytes 
    unsigned* header = (unsigned*)malloc((maxSymbolValue + 1) * (maxSymbolValue + 2) * 2 + (maxSymbolValue + 1) * 2);
    unsigned header_offset = 0;
    // output should first contain maxSymbolValue and tablelog 
    // initialize o1 and o0 histograms
    unsigned o0count[maxSymbolValue + 1] = {0};
    unsigned o1count[(maxSymbolValue + 1) * (maxSymbolValue + 1)] = {0};
    O1_HIST(src, srcSize, o0count, o1count, maxSymbolValue, symbollength);
    // normalize o0 count
    uint16_t* o0_normalized_count = (uint16_t*)malloc(sizeof(uint16_t) * (maxSymbolValue + 1));
    printf("Normalizing O0 counts...\n");
    normalizeCountO0(o0count, maxSymbolValue, srcSize / symbollength, tableLog, o0_normalized_count);
    printf("O0 normalization done.\n");
    // convert normalized o0 count to unsigned and write to header 
    for (int i = 0; i <= maxSymbolValue; i++) {
        header[2 * i] = o0_normalized_count[i] & 0xFF;
        header[2 * i + 1] = (o0_normalized_count[i] >> 8) & 0xFF;
    }
    header_offset += sizeof(uint16_t) * (maxSymbolValue + 1);
    // select only those symbol whose occurrence exceeds the cap
    size_t header_size = 0;
    size_t o1_use_cnt = 0;
    unsigned o1_use_or_not[maxSymbolValue];
    uint16_t** o1_normalized_count = (uint16_t**)malloc(sizeof(uint16_t*) * (maxSymbolValue + 1));

    for(int i = 0; i <= maxSymbolValue; i++) {
        o1_use_or_not[i] = o0count[i] >= o0numcap * srcSize / symbollength;
        if (o1_use_or_not[i]) {
            o1_use_cnt++;
            o1_normalized_count[i] = (uint16_t*)malloc(sizeof(uint16_t) * (maxSymbolValue + 1));
            unsigned* new_header;
            printf("Normalizing O1 counts for symbol %d...\n", i);
            size_t o1_header_size = normalizeCountO1(o0count, o1count + i * (maxSymbolValue + 1), maxSymbolValue, o1numcap, srcSize / symbollength, tableLog, o1_normalized_count[i], &new_header);
            printf("O1 normalization for symbol %d done.\n", i);
            // copy new_header to header
            memcpy((unsigned char*)header + header_offset, new_header, o1_header_size);
            header_offset += o1_header_size;
            free(new_header);
        }
    }
    unsigned final_header_size = header_offset;
    printf("o1_use_cnt=%zu, final_header_size=%u\n", o1_use_cnt, final_header_size);
    // use TANS to compress the src 
    // if prev symbol's value is in o1_use_or_not, use o1 table to compress current symbol
    // else use o0 table to compress current symbol
    
    // tANS state variables
    const unsigned tableSize = 1U << tableLog;
    const unsigned stateMask = tableSize - 1;
    // The initial state value can be anywhere in [tableSize, 2*tableSize - 1].
    // Using 2*tableSize - 1 for consistency with common implementations.
    unsigned state = tableSize; 

    // Pointers for reading symbols and writing compressed data
    const unsigned char* src_bytes = (const unsigned char*)src;
    // Total number of symbols (assuming symbollength is 1 for simplicity in symbol handling)
    const size_t num_symbols = srcSize / symbollength; 
    
    // Pointers for writing the compressed data (encoded stream)
    unsigned char* out_ptr = (unsigned char*)dst;
    
    *(unsigned*)out_ptr = maxSymbolValue;
    out_ptr += sizeof(uint8_t);

    *(unsigned*)out_ptr = tableLog;
    out_ptr += sizeof(uint8_t);
    
    // Write the normalized count header
    memcpy(out_ptr, header, final_header_size);
    out_ptr += final_header_size;
    free(header);

    assert(srcSize >= 2 && symbollength == 1);
    int prev_id = srcSize - 2;
    int current_id = srcSize - 1;
    // calculate cumul 
    unsigned o0_cumul[maxSymbolValue + 2] = {0};
    unsigned o1_cumul[maxSymbolValue + 1][maxSymbolValue + 2] = {{0}};
    for(int i = 1; i <= maxSymbolValue; i++) {
        o0_cumul[i] = o0_cumul[i - 1] + o0_normalized_count[i - 1];
        for(int j = 0; j <= maxSymbolValue; j++) {
            if(!o1_use_or_not[j]) continue;
            o1_cumul[j][i] = o1_cumul[j][i - 1] + o1_normalized_count[j][i - 1];
        }
    }
    printf("RANS Table built. Starting compression...\n");
    while(current_id >= 0) {
        unsigned current_symbol = src_bytes[current_id];
        unsigned prev_symbol = 0;
        if(current_id > 0) prev_symbol = src_bytes[prev_id];
        // determine which table to use
        // use update fomula:
        // x = (x // freq[s]) * M + cumul[s] + (x & freq[s]))
        // so that for decode:
        // block, slot = x // M, x % M;
        // slot >= cumul[s] && slot < cumul[s + 1]
        // renormalize to [M, 2M -1]
        // just use rans for now
        int freq = o0_normalized_count[current_symbol];
        int cumul = o0_cumul[current_symbol];
        if (o1_use_or_not[prev_symbol]) {
            freq = o1_normalized_count[prev_symbol][current_symbol];
            cumul = o1_cumul[prev_symbol][current_symbol];
        } 
        assert(freq > 0);
        // calculate the next state
        int new_state = (state / freq) * tableSize + cumul + (state % freq);
        // renormalize
        while (new_state >= (tableSize << 8)) {
            *out_ptr++ = (unsigned char)(new_state & 0xFF);
            new_state >>= 8;
        }
        state = new_state;
        current_id--;
        prev_id--;
    }
    // flush remaining state
    while (state > 0) {
        *out_ptr++ = (unsigned char)(state & 0xFF);
        state >>= 8;
    }
    printf("RANS Compression done.\n");
    unsigned final_output_size = out_ptr - (unsigned char*)dst;
    *dstSize = final_output_size;
    // for(int i = 0; i <= maxSymbolValue; i++) if(o1_normalized_count[i]) free(o1_normalized_count[i]);
    // free(o1_normalized_count);
    // free(o0_normalized_count);
    printf("Final compressed size: %u bytes\n", final_output_size);
    return final_output_size;
}

size_t FSEO1_Compress(void* dst, size_t* dstSize,
                   const void* src, size_t srcSize) {
    return FSEO1_Compress2(dst, dstSize, src, srcSize, FSEO1_DEFAULT_MAX_SYMBOL_VALUE, FSEO1_DEFAULT_TABLELOG, FSEO1_DEFAULT_O0NUMCAP, FSEO1_DEFAULT_O1NUMCAP, 1);
}