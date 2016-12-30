#include "data_types.h"


/*data type arrays*/
const char* DATA_TYPES[NUMBER_OF_DATA_TYPES] = {"uint8_t", "int8_t", "uint16_t", "int16_t", "uint32_t", "int32_t", "uint64_t", "int64_t", "path_string"};
const Data_Type DATA_TYPE_VALUES[NUMBER_OF_DATA_TYPES] = {UINT_8, INT_8, UINT_16, INT_16, UINT_32, INT_32, UINT_64, INT_64, PATH_STRING};


/* boolean flags used for status checking */
const int SUCCESS = 1;
const int FAILURE = 0;
