#include "includes/headers.p4"
#include "includes/parser.p4"



#define MAX_NUM_OF_WORDS 16 // power of 2
#define NUM_OF_BITS_FOR_INDEX 4 // 2^(NUM_OF_BITS_FOR_INDEX) = MAX_NUM_OF_WORDS

header_type ingress_metadata_t {
    fields {
       
        // Below are the values used for the MapReduce app

        word_hashed_index : NUM_OF_BITS_FOR_INDEX;
        word_current_counter : 32;
        
        word_index_word : ENCODING_FOR_WORDS_BITS;
        word_index_counter : COUNTERS_FOR_WORDS_BITS;

        // for resubmission	
        word_map_representation : ENCODING_FOR_WORDS_BITS; // defined in headers.p4
        word_counter_representation : COUNTERS_FOR_WORDS_BITS;	// defined in headers.p4
    }
}

metadata ingress_metadata_t ingress_metadata;

header word_map_header_t map_reduce_header;


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

action increment_word_counter() {
   modify_field_with_hash_based_offset(ingress_metadata.word_hashed_index, 0,
                                       hash_word_for_index, MAX_NUM_OF_WORDS);

  /*Since hashing is used to determine the index within the range [0, MAX_NUM_OF_WORDS - 1], there might be collisions.*/

  register_read(ingress_metadata.word_current_counter, word_counter, ingress_metadata.word_hashed_index); 
  add_to_field(ingress_metadata.word_current_counter, 1); // increment the counter of the received word
  register_write(word_counter, ingress_metadata.word_hashed_index, ingress_metadata.word_current_counter); // update register file
  
  register_write(word_value, ingress_metadata.word_hashed_index, word_header.wordEnc); // store the value of the received word

  drop(); // drop the received packet since there is no need to forward it because the switch stores it in the switch's internal memory
}


table word_count_table {
  actions {
    increment_word_counter;
  }
}


action send_map_table() {
  
  // change the headers a bit
  remove_header(word_header);
  add_header(map_reduce_header);
  add_header(word_header);
  modify_field(word_header.flags, 0x01); // set the flag  

  // Read registers, add them to a packet, erase them.
  
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x00);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x00);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x00, 0);
  register_write(word_counter, 0x00, 0);     
  

 
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x01);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x01);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x01, 0);
  register_write(word_counter, 0x01, 0);  



  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x02);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x02);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x02, 0);
  register_write(word_counter, 0x02, 0);



  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x03);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x03);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x03, 0);
  register_write(word_counter, 0x03, 0);



  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x04);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x04);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x04, 0);
  register_write(word_counter, 0x04, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x05);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x05);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x05, 0);
  register_write(word_counter, 0x05, 0);



  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x06);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x06);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x06, 0);
  register_write(word_counter, 0x06, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x07);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x07);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x07, 0);
  register_write(word_counter, 0x07, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x08);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x08);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x08, 0);
  register_write(word_counter, 0x08, 0);



  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x09);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x09);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x09, 0);
  register_write(word_counter, 0x09, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x0A);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x0A);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x0A, 0);
  register_write(word_counter, 0x0A, 0);


      
    // read
  register_read(ingress_metadata.word_index_word, word_value, 0x0B);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x0B);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x0B, 0);
  register_write(word_counter, 0x0B, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x0C);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x0C);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x0C, 0);
  register_write(word_counter, 0x0C, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x0D);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x0D);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x0D, 0);
  register_write(word_counter, 0x0D, 0);


   
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x0E);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x0E);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
  
  // shift for next reading
  shift_left(ingress_metadata.word_map_representation, 
                             ingress_metadata.word_map_representation, FIXED_WORD_SIZE);
  shift_left(ingress_metadata.word_counter_representation, 
                             ingress_metadata.word_counter_representation, 32);  

  //Clear read registers
  register_write(word_value, 0x0E, 0);
  register_write(word_counter, 0x0E, 0);


  // Last reading -- no shifting 
  // read
  register_read(ingress_metadata.word_index_word, word_value, 0x0F);
  register_read(ingress_metadata.word_index_counter, word_counter, 0x0F);
  
  // make sure that only the word is read
  bit_and(ingress_metadata.word_index_word, 
                                ingress_metadata.word_index_word, 0xFFFFFFFFFF);
  bit_and(ingress_metadata.word_index_counter, 
                                ingress_metadata.word_index_counter, 0xFFFFFFFFFF);     
 

  // add to the map packet
  bit_or(ingress_metadata.word_map_representation, 
                       ingress_metadata.word_map_representation, ingress_metadata.word_index_word);
  bit_or(ingress_metadata.word_counter_representation, 
                 ingress_metadata.word_counter_representation, ingress_metadata.word_index_counter);  
   

  //Clear read registers
  register_write(word_value, 0x0F, 0);
  register_write(word_counter, 0x0F, 0);


  /********************************************** End of reading the map *************************************/
  modify_field(map_reduce_header.num_of_words, MAX_NUM_OF_WORDS);
  modify_field(standard_metadata.egress_spec, 3); // make sure that the port number is 3
}

table aggregation_table {
   actions {
     send_map_table;
   }
}

control ingress {
  
   // Since resubmit() is not supported, the code only reads all possible values in an action.
   if(valid(word_header))
   {
     if(word_header.flags == 0x00) // just increment the counter
     {
       apply(word_count_table);
     }
     
     else // send the entire map to a host
     { 
       apply(aggregation_table);
     }
   }
}


control egress {
}
