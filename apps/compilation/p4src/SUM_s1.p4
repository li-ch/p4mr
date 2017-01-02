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

#define PREAMBLE_SIZE 64
#define APP_BITS 16 
#define INT_BITS 64 
#define SWITCH_ID 0x01 
#define NEXT_HOP 4


/***********************
   Header Definition
 **********************/

header_type data_header_t {
    fields {
        preamble       : PREAMBLE_SIZE;
        application_id : APP_BITS;
        value          : INT_BITS; 
    }
}

header data_header_t data_header;


/***********************
   Parser Definition
 **********************/

parser start {
    return select ( current(0,PREAMBLE_SIZE) ) {
        0 : parse_data;
        default: ingress;
    }
}

parser parse_data {
    extract (data_header);
    return ingress;
}


/***********************
   Registers & Tables
 **********************/

header_type egress_metadata_t {
  fields {
     temp_result : INT_BITS;
  }  
}

metadata egress_metadata_t egress_metadata;

register my_result {
    width : INT_BITS;
    instance_count : 16; 
}

action _drop() {
    drop();
}

action set_port() {
    modify_field (standard_metadata.egress_spec, NEXT_HOP); 
}

table forward_table {
   actions {
      set_port;
   }
}

action my_update() {
    register_read (egress_metadata.temp_result, my_result, 0x00); 
    add_to_field (egress_metadata.temp_result, data_header.value); 
    register_write (my_result, 0x00, egress_metadata.temp_result); 
}

table update_table {
    reads {
        data_header.value : exact;
    }
    actions {
       my_update;
       _drop;
    }
}


control ingress {
  if ( valid(data_header) ) { // if the data header is successfully parsed,
    if (data_header.application_id == SWITCH_ID) {  
      apply (update_table);
    } else {
       apply (forward_table);
    }
  } 
}
