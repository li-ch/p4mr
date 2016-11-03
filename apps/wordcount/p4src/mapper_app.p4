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

#include "includes/headers_mapper.p4"
#include "includes/parser.p4"



#define CLONE_SPEC_ID 255
#define NUM_OF_BITS_FOR_INDEX 4
#define NUM_OF_WORD_TABLE_ENTRIES 16 

// locally a mapper should only have ports within the range [1, 4] -- 1 - input port, [2, 4] - output ports
// Read commands.txt in mininet_test. The text file sets table entries for forwarding words. 
#define PORT_INDEX_BASE 2
#define PORT_INDEX_SIZE 3

header_type ingress_metadata_t {
  fields {
     port_number : NUM_OF_BITS_FOR_INDEX;
     hashed_word : NUM_OF_BITS_FOR_INDEX;
  }  
}

metadata ingress_metadata_t ingress_metadata {port_number : PORT_INDEX_BASE;};


field_list ingress_port_fields {
  ingress_metadata;
}

field_list word_hashing_fields { 
   word_header.wordEnc;
}

field_list_calculation word_hashing_spec {
    input {
        word_hashing_fields;
    }
    algorithm : crc16;
    output_width : NUM_OF_BITS_FOR_INDEX;
}


action drop_packet() {
  drop();
}


action hash_word() {
  modify_field_with_hash_based_offset(standard_metadata.egress_spec, PORT_INDEX_BASE, word_hashing_spec, PORT_INDEX_SIZE); // for determining the port number
}

table hash_packet_table {
  actions {
     hash_word;
  }
}

/*action set_port(port) {
 
  modify_field(standard_metadata.egress_spec, port);
}
*/
/*
table set_port_table {
   reads {
      ingress_metadata.hashed_word : exact;  
   }
 
   actions {
      set_port;
      drop_packet;
   }
  
  size : NUM_OF_WORD_TABLE_ENTRIES; 
}
*/

action send_to_all() {
  // Check if this works. We keep incrementing the egress port number to send data on a port
  modify_field(standard_metadata.egress_spec, PORT_INDEX_BASE);
  clone_ingress_pkt_to_egress(CLONE_SPEC_ID);
  add_to_field(standard_metadata.egress_spec, 1);
  clone_ingress_pkt_to_egress(CLONE_SPEC_ID);
  add_to_field(standard_metadata.egress_spec, 1);
  
}

table send_to_all_table {
  actions {
     send_to_all;
  }
}


control ingress {
 
 if(valid(word_header)) { // if the header successfully parsed -- mappers have to forward all packets 
   if(word_header.flags == 0x00) { // means a word is being sent ==> forward to a reducer
     apply(hash_packet_table);
     //apply(set_port_table);
   }
   
   else { // means a request to send maps to the data collector ==> forward to all the reducers
     apply(send_to_all_table);
   }   

 }
 
}



