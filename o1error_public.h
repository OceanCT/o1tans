typedef enum {
    FSE_error_no_error,
    FSE_error_GENERIC,
    FSE_error_dstSize_tooSmall,
    FSE_error_srcSize_wrong,
    FSE_error_corruption_detected,
    FSE_error_tableLog_tooLarge,
    FSE_error_maxSymbolValue_tooLarge,
    FSE_error_maxSymbolValue_tooSmall,
    FSE_error_workSpace_tooSmall,
    FSE_error_maxCode
} FSEO1_ErrorCode;