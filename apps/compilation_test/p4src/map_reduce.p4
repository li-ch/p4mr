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

#include "includes/headers.p4"
#include "includes/parser.p4"


#define MAX_NUM_OF_WORDS 16 // power of 2
#define NUM_OF_BITS_FOR_INDEX 4 // 2^(NUM_OF_BITS_FOR_INDEX) = MAX_NUM_OF_WORDS

header_type ingress_metadata_t {
    fields {
       
        // Below are the values used for the MapReduce app
        dst_port_number : 8;

        word_hashed_index : NUM_OF_BITS_FOR_INDEX;
        word_current_counter : 32;
        
        word_index_word : ENCODING_FOR_WORDS_BITS;
        word_index_counter : COUNTERS_FOR_WORDS_BITS;

        // for resubmission	
        word_map_representation : ENCODING_FOR_WORDS_BITS; // defined in headers.p4
        word_counter_representation : COUNTERS_FOR_WORDS_BITS;	// defined in headers.p4
        word_pointer_current_value : NUM_OF_BITS_FOR_INDEX;
    }
}

metadata ingress_metadata_t ingress_metadata;



/****************************** Below code is for the MapReduce application ****************************************/


field_list resubmit_field_list {
  ingress_metadata.dst_port_number;
  ingress_metadata.word_map_representation;
  ingress_metadata.word_counter_representation;
  ingress_metadata.word_pointer_current_value;
 
  intrinsic_metadata.resubmit_flag;
}

field_list word_hashing_fields {
   word_header.wordEnc;
}

field_list_calculation hash_word_for_index {
    input {
        word_hashing_fields;
    }
    algorithm : crc16;
    output_width : NUM_OF_BITS_FOR_INDEX;
}


register word_counter {
  width : 32;
  instance_count : MAX_NUM_OF_WORDS;  // power of 2
}


register word_value {
  width : FIXED_WORD_SIZE; // define in headers.p4
  instance_count : MAX_NUM_OF_WORDS;  // power of 2
}

action increment_register() {
  
  
  modify_field_with_hash_based_offset(ingress_metadata.word_hashed_index, 0,
                                       hash_word_for_index, MAX_NUM_OF_WORDS);
  /*Since hashing is used to determine the index within the range [0, MAX_NUM_OF_WORDS - 1], there might be collisions.*/

  register_read(ingress_metadata.word_current_counter, word_counter, ingress_metadata.word_hashed_index); 
  add_to_field(ingress_metadata.word_current_counter, 1); // increment the counter of the received word
  register_write(word_counter, ingress_metadata.word_hashed_index, ingress_metadata.word_current_counter); // update register file
  
  register_write(word_value, ingress_metadata.word_hashed_index, word_header.wordEnc); // store the value of the received word

  drop(); // drop the received packet since there is no need to forward it because the switch stores it in the switch's internal memory
}


table word_map {
  actions{
     increment_register; // add 1 to the current value of the cell for the word the header stores
  }
}


/*action start_processing() {
  

  // the following few lines of code just adds a word_map_header_t instance to the packet behind a word_header_t header

  //modify_field(standard_metadata.egress_spec, word_header.port); // for picking the right port number
  modify_field(ingress_metadata.dst_port_number, word_header.port); // for picking the right port number  

  remove_header(word_header); // do this step in order to keep a proper sequence of headers
  //add_header(word_map_header); // set to valid and add to the packet
  //add_header(word_header);
  //modify_field(word_header.flags, 0x01); // set the flag 
  //modify_field(word_header.port, standard_metadata.egress_spec); // for picking the right port number
  // headers have been added in a predefined order |word_header| |word_map_header| |data|
  

  // read stored values from the registers
  register_read(ingress_metadata.word_index_word, word_value, 0);
  register_read(ingress_metadata.word_index_counter, word_counter, 0);
  bit_and(ingress_metadata.word_index_word, ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     

 
  //shift_left(ingress_metadata.word_map_representation, ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  //shift_left(ingress_metadata.word_counter_representation, ingress_metadata.word_counter_representation, 32);  
 
  bit_or(ingress_metadata.word_map_representation, ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  

  //Clear read registers
  register_write(word_value, 0, 0);
  register_write(word_counter, 0, 0);     
  add_to_field(ingress_metadata.word_pointer_current_value, 0x1);  

  // update header fields 
  //bit_or(word_map_header.wordEnc, word_map_header.wordEnc, ingress_metadata.word_map_representation);
  //bit_or(word_map_header.count, word_map_header.count, ingress_metadata.word_counter_representation); 

  //modify_field(word_map_header.num_of_words,  0x01); // the most significant word
  //register_write(word_map_pointer, 0, 0x1); // initialize the current value of the pointer to one (one word is processed)
  
}

table start_word_map_processing {
  actions {
     start_processing;
  }
}

action send_switch_map() {
   
  //register_read(ingress_metadata.word_pointer_current_value, word_map_pointer, 0); // only one instance
   
  // read stored values from the registers
  register_read(ingress_metadata.word_index_word, word_value, ingress_metadata.word_pointer_current_value);
  register_read(ingress_metadata.word_index_counter, word_counter, ingress_metadata.word_pointer_current_value);
  bit_and(ingress_metadata.word_index_word, ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     

  // a new word is being added. Shift the previous data by one word-length
  shift_left(ingress_metadata.word_map_representation, ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, ingress_metadata.word_counter_representation, 32);  
 
  bit_or(ingress_metadata.word_map_representation, ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter); 


  //Clear read registers
  register_write(word_value, ingress_metadata.word_pointer_current_value, 0);
  register_write(word_counter, ingress_metadata.word_pointer_current_value, 0); 

  add_to_field(ingress_metadata.word_pointer_current_value, 0x1); // update index for the next step
  //add_to_field(word_map_header.num_of_words,  0x01); // number of words stored

  /***************** The below two lines might be a problem for future work. Try to shift by a constant *********/
  /*shift_left(ingress_metadata.word_map_representation, ingress_metadata.word_map_representation, (ENCODING_FOR_WORDS_BITS - FIXED_WORD_SIZE * ingress_metadata.word_map_representation));
  shift_left(ingress_metadata.word_counter_representation, ingress_metadata.word_counter_representation, (ENCODING_FOR_WORDS_BITS - FIXED_WORD_SIZE * ingress_metadata.word_map_representation));  */
  /********************************** Problem ends here ********************************************************/

  

  // update header fields 
  //bit_or(word_map_header.wordEnc, word_map_header.wordEnc, ingress_metadata.word_map_representation);
  //bit_or(word_map_header.count, word_map_header.count, ingress_metadata.word_counter_representation);
  //register_write(word_map_pointer, 0, ingress_metadata.word_pointer_current_value);  // update the current value to prev_value + 1; 
 
  
/*  
}

table send_word_map {
   actions{
      send_switch_map;
   }
}
*/

action resubmit_action() {
   modify_field(intrinsic_metadata.resubmit_flag, 0x01);
   resubmit(resubmit_field_list);
}

table resubmit_table {
   actions {
      resubmit_action; // if still need to add more words
   }
}

action set_port() {
 modify_field(standard_metadata.egress_spec,  0x03); // for picking the right port number
 //add_header(word_map_header);
 //modify_field(word_map_header.wordEnc, ingress_metadata.word_map_representation);
 //modify_field(word_map_header.count, ingress_metadata.word_counter_representation);
 //modify_field(word_map_header.num_of_words, ingress_metadata.word_pointer_current_value);
 
 add_header(word_header);
 modify_field(word_header.flags, 0x01); // signal that this is a map packet
}

table set_port_table {
   actions {
      set_port;
   }
}

/************************************ Up to here ***********************************************************/




control ingress {
    // First check if the parsed packet has been resubmitted
   
   if(standard_metadata.instance_type != 0x00)
   { 
     apply(set_port_table);
   }
   else
   {
     if(valid(word_header) and word_header.flags == 0x01)
     {
         apply(resubmit_table);
      }
   }

   /* if(intrinsic_metadata.resubmit_flag == 0x01)
    {
       apply(send_word_map); // nothing more
    }
    
    else // a normal packet has been received
    {
      // First, check if the received packet is a data header or a send-request header.
      if(valid(word_header) and word_header.flags == 0x00)
      { // means a data header (carries a word/words)
        apply(word_map);
      }
      else
      {
        if(valid(word_header) and word_header.flags == 0x01) // means a send-request header
        {
          apply(start_word_map_processing);
        }
      }
    }
   
   
   if (valid(word_map_header))
   {
      if(word_map_header.num_of_words < MAX_NUM_OF_WORDS)
      {  // Resubmit if still some words left for reading
        apply(resubmit_table);
      }
      else 
      {
         apply(set_port_table);
      }
   }*/

}

control egress {
    
}
