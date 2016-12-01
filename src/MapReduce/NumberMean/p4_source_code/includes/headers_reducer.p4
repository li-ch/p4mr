/*
Copyright 2013-present Barefoot Networks, Inc. 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/



#define NUMBER_SIZE 64 // 64-bit signed integer (limited by the size of ALUs)
#define COUNTER_SIZE 32 // COUNT_SIZE unsigned int and the number of integers is a power of 2 
#define NUM_OF_NUMBERS 64 // NUM_OF_NUMBERS integers per packet at most
#define DATA_ID_SIZE 32 // 32-bit unsigned int for identifying id (used for table matching)


#define DATA_PACKET_FLAG 0x00




header_type number_header_reducer_t{
     fields{
         preamble : 64;
         // below is the data (64-bit signed integers)
         number_value_1 : NUMBER_SIZE (signed);
         number_value_2 : NUMBER_SIZE (signed);
         number_value_3 : NUMBER_SIZE (signed);
         number_value_4 : NUMBER_SIZE (signed);
         number_value_5 : NUMBER_SIZE (signed);
         number_value_6 : NUMBER_SIZE (signed);
         number_value_7 : NUMBER_SIZE (signed);
         number_value_8 : NUMBER_SIZE (signed);
         number_value_9 : NUMBER_SIZE (signed);
         number_value_10 : NUMBER_SIZE (signed); 
         number_value_11 : NUMBER_SIZE (signed);
         number_value_12 : NUMBER_SIZE (signed);
         number_value_13 : NUMBER_SIZE (signed);
         number_value_14 : NUMBER_SIZE (signed);
         number_value_15 : NUMBER_SIZE (signed);
         number_value_16 : NUMBER_SIZE (signed);
         number_value_17 : NUMBER_SIZE (signed);
         number_value_18 : NUMBER_SIZE (signed);
         number_value_19 : NUMBER_SIZE (signed);
         number_value_20 : NUMBER_SIZE (signed); 
         number_value_21 : NUMBER_SIZE (signed);
         number_value_22 : NUMBER_SIZE (signed);
         number_value_23 : NUMBER_SIZE (signed);
         number_value_24 : NUMBER_SIZE (signed);
         number_value_25 : NUMBER_SIZE (signed);
         number_value_26 : NUMBER_SIZE (signed);
         number_value_27 : NUMBER_SIZE (signed);
         number_value_28 : NUMBER_SIZE (signed);
         number_value_29 : NUMBER_SIZE (signed);
         number_value_30 : NUMBER_SIZE (signed); 
         number_value_31 : NUMBER_SIZE (signed);
         number_value_32 : NUMBER_SIZE (signed);
         number_value_33 : NUMBER_SIZE (signed);
         number_value_34 : NUMBER_SIZE (signed);
         number_value_35 : NUMBER_SIZE (signed);
         number_value_36 : NUMBER_SIZE (signed);
         number_value_37 : NUMBER_SIZE (signed);
         number_value_38 : NUMBER_SIZE (signed);
         number_value_39 : NUMBER_SIZE (signed);
         number_value_40 : NUMBER_SIZE (signed); 
         number_value_41 : NUMBER_SIZE (signed);
         number_value_42 : NUMBER_SIZE (signed);
         number_value_43 : NUMBER_SIZE (signed);
         number_value_44 : NUMBER_SIZE (signed);
         number_value_45 : NUMBER_SIZE (signed);
         number_value_46 : NUMBER_SIZE (signed);
         number_value_47 : NUMBER_SIZE (signed);
         number_value_48 : NUMBER_SIZE (signed);
         number_value_49 : NUMBER_SIZE (signed);
         number_value_50 : NUMBER_SIZE (signed); 
         number_value_51 : NUMBER_SIZE (signed);
         number_value_52 : NUMBER_SIZE (signed);
         number_value_53 : NUMBER_SIZE (signed);
         number_value_54 : NUMBER_SIZE (signed);
         number_value_55 : NUMBER_SIZE (signed);
         number_value_56 : NUMBER_SIZE (signed);
         number_value_57 : NUMBER_SIZE (signed);
         number_value_58 : NUMBER_SIZE (signed);
         number_value_59 : NUMBER_SIZE (signed);
         number_value_60 : NUMBER_SIZE (signed); 
         number_value_61 : NUMBER_SIZE (signed);
         number_value_62 : NUMBER_SIZE (signed);
         number_value_63 : NUMBER_SIZE (signed);
         number_value_64 : NUMBER_SIZE (signed);        

         num_of_numbs : COUNTER_SIZE;
         data_id : DATA_ID_SIZE;
         flags : 8; // if flags = 0x00 -- data packet, flags = 0x11 -- aggregate packet
     }
}

