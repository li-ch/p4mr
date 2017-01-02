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

/*  
    locally a mapper should only have ports within the range [1, 4]
    -- 1 - input port, [2, 4] - output ports
*/
#define PORT_INDEX_BASE 2
#define PORT_INDEX_SIZE 3

header_type egress_metadata_t {
  fields {
     port_number : NUM_OF_BITS_FOR_INDEX;
  }  
}

metadata egress_metadata_t egress_metadata {
    port_number : PORT_INDEX_BASE;
};


field_list egress_port_number {
  egress_metadata.port_number;
}


action set_port() {
 // just hash the word and set port number in the range [2, 4]
 add_to_field(egress_metadata.port_number, 1); 
 modify_field(standard_metadata.egress_spec, egress_metadata.port_number); 
}

action _drop() {
    drop();
}

action _resubmit() {
    resubmit();
}


table set_port_table {
   actions {
      set_port;
      // _resubmit;
      _drop;
   }
}



control ingress {
  if(valid(word_header) ) { 
  // if(valid(word_header) and egress_metadata.port_number < 4) { 
    // if the header successfully parsed -- mappers have to 
    // forward all packets, either flags = 0x00 or flags = 0x01 
    // a word is sent. Forward to a particular port

      apply(set_port_table);
  } 
}



