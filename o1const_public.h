const int FSEO1_DEFAULT_MEMORY_USAGE = 13;
const int FSEO1_DEFAULT_TABLELOG = FSEO1_DEFAULT_MEMORY_USAGE - 2;
// here the default table log should be smaller considering we need to store additional order-1 table
// but keeping it before further analysis on compression ratio impact

const int FSEO1_DEFAULT_MAX_SYMBOL_VALUE = 255;


// numcap decides which o1 table should be saved and used 
// in other words, for n length src, the occurrence of symbol should be n * FSEO1_DEFAULT_O0NUMCAP 
// to store its symbol
// FSEO1_DEFAULT_O1NUMCAP decides in the o1 table which symbol should be stored
const float FSEO1_DEFAULT_O0NUMCAP = 1.0f/256 * 20;
// const float FSEO1_DEFAULT_O0NUMCAP = 1.0f / 256 * 40;
// const float FSEO1_DEFAULT_O0NUMCAP = 0;
const float FSEO1_DEFAULT_O1NUMCAP = 0;