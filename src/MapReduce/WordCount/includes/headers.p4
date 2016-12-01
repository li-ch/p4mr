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


#define ENCODING_FOR_WORDS_BITS 5400 // 135 words per header at most 
#define COUNTERS_FOR_WORDS_BITS 4320 // 135 counters per header at most
#define FIXED_WORD_SIZE 40 // 40 bits per word


/**
* This type of header is used for sending words 
* and sending a send-reuest header. In other words, an
* instance of this type header is used for counting the
* occurrence of a word at a switch or for a reqeust to send 
* the word map stored by a switch 
*/
header_type word_header_t{
     fields{
         preamble : 64;
         wordEnc : FIXED_WORD_SIZE;
         wordLen : 8;
         flags : 8;
         port : 8;
     }
}


/**
* This type of header is used by the switches 
* for sending their word maps to the reducers.
*
*/
header_type word_map_header_t{
      fields{
         wordEnc : ENCODING_FOR_WORDS_BITS; // if num_of_words < (max words) ==> read from left to right
         count : COUNTERS_FOR_WORDS_BITS; // if num_of_words < (max words) ==> read from left to right
         num_of_words : 8; // stores the number of words in this header (at most 135 words)
      }   
}
