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


#define NUM_OF_BITS_FOR_INDEX 16 

// locally a mapper should only have ports within the range [0, 3] -- 0 - input port, [1, 3] - output ports
#define PORT_INDEX_BASE 1
#define PORT_INDEX_SIZE 3

header_type egress_metadata_t {
  fields {
     port_number : NUM_OF_BITS_FOR_INDEX;
  }  
}

metadata egress_metadata_t egress_metadata {port_number : PORT_INDEX_BASE;};


field_list egress_port_number {
  egress_metadata.port_number;
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


action set_port() {
 // just hash the word and set port number in the range [1, 3]
 modify_field_with_hash_based_offset(standard_metadata.egress_spec, PORT_INDEX_BASE, 
                                     word_hashing_spec, PORT_INDEX_SIZE); 
}

table set_port_table {
   actions {
      set_port;
   }
}


action set_first_request() {
  modify_field(standard_metadata.egress_spec, PORT_INDEX_BASE); // start with the first port
}

table set_first_request_table {
  actions {
    set_first_request;
  }
}

action send_to_all() {
  add_to_field(egress_metadata.port_number, 1); // update for next packet
  modify_field(standard_metadata.egress_spec, egress_metadata.port_number);
  clone_egress_pkt_to_egress(10, egress_port_number);
}

table send_to_all_table {
  actions {
    send_to_all;
  }
}
 

control ingress {
  if(valid(word_header)) { // if the header successfully parsed -- mappers have to forward all packets, either flags = 0x00 or flags = 0x01 
    if(word_header.flags == 0x00 or word_header.wordEnc != 0) // a word is sent. Forward to a particular port
    {  
      apply(set_port_table);
    }
    else  // flags set // send to the reducers a request to reduce
    {
       apply(set_first_request_table);
    }
  } 
}

control egress {
  if(valid(word_header) and word_header.flags != 0x00 and word_header.wordEnc == 0
     and egress_metadata.port_number <= PORT_INDEX_SIZE) { // send to all reducers a request to their tables
    
       apply(send_to_all_table);
  }
}


