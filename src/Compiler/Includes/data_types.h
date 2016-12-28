#ifndef P4_FRAMEWORK_COMPILER_DATA_TYPES_H
#define P4_FRAMEWORK_COMPILER_DATA_TYPES_H

typedef enum {
 UINT_8 = 1008,
 INT_8,
 UINT_16, 
 INT_16, 
 UINT_32, 
 INT_32, 
 UINT_64, 
 INT_64,
 PATH_STRING, /*IP string*/
 EMPTY_VAL, 
 ERROR_DATA = 0
} Data_Type;


#define NUMBER_OF_DATA_TYPES 9
const char* DATA_TYPES[NUMBER_OF_DATA_TYPES] = {"uint8_t", "int8_t", "uint16_t", "int16_t", "uint32_t", "int32_t", "uint64_t", "int64_t", "path_string"};
const Data_Type DATA_TYPE_VALUES[NUMBER_OF_DATA_TYPES] = {UINT_8, INT_8, UINT_16, INT_16, UINT_32, INT_32, UINT_64, INT_64, PATH_STRING};



#endif
