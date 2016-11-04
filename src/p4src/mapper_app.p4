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
#include "includes/parser_mapper.p4"


#define CLONE_ID 255
#define NUM_OF_BITS_FOR_INDEX 4

// locally a mapper should only have ports within the range [1, 4] -- 1 - input port, [2, 4] - output ports
#define PORT_INDEX_BASE 2
#define PORT_INDEX_SIZE 3



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
 // just hash the word and set port number in the range [2, 4]
 modify_field_with_hash_based_offset(standard_metadata.egress_spec, PORT_INDEX_BASE, 
                                     word_hashing_spec, PORT_INDEX_SIZE); 
  //modify_field(standard_metadata.egress_spec, PORT_INDEX_BASE);
}

table set_port_table {
    actions {
       set_port;
   }
}


action send_to_all() {
  // hard-code forwarding for now as some primitive actions do not work.
  modify_field(standard_metadata.egress_spec, PORT_INDEX_BASE);
  /*
  modify_field(ingress_metadata.port, PORT_INDEX_BASE);
  clone_ingress_pkt_to_egress(CLONE_ID);

  add_to_field(ingress_metadata.port, 1); // update for next packet
  modify_field(standard_metadata.egress_spec, ingress_metadata.port);
  clone_ingress_pkt_to_egress(CLONE_ID);

  add_to_field(ingress_metadata.port, 1); // update for next packet
  modify_field(standard_metadata.egress_spec, ingress_metadata.port);
  */
}

table send_to_all_table {
  actions {
    send_to_all;
  }
}
 

control ingress {

  if(valid(word_header)) { // if the header successfully parsed -- mappers have to forward all packets, either flags = 0x00 or flags = 0x01 
    if(word_header.flags == 0x00) // a word is sent. Forward to a particular port
    {  
      apply(set_port_table);
    }
    else  // flags set // send to the reducers a request to reduce
    {
       apply(send_to_all_table);
    }
  } 
}


