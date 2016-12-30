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

/* boolean flags used for status checking */
extern const int SUCCESS;
extern const int FAILURE;

/* data type arrays for converting data types into strings */ 
extern const char* DATA_TYPES[NUMBER_OF_DATA_TYPES];
extern const Data_Type DATA_TYPE_VALUES[NUMBER_OF_DATA_TYPES];


#endif
