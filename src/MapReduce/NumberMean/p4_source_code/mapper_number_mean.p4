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


#define OUTPUT_PORT 2 

header_type ingress_metadata_t {
    fields {
       
        // Below are the values used for the MapReduce app
        temp_counter : COUNTER_SIZE; // for storing temporarily the current value of the counter
        temp_sum : NUMBER_SIZE (signed); // for storing temporarily the current sums of integers
    }
}

metadata ingress_metadata_t ingress_metadata;



// register file for storing sums
register integer_sum_register {

  width : NUMBER_SIZE; // constants from headers.p4
  instance_count : 1;
}


// register file for storing the number of integers received
register integer_count_register {
  
  width : NUMBER_SIZE; // constant from headers.p4
  instance_count : 1;  
}


action add_to_sum() {
  // just take the number of integers and add it to the counter
  register_read(ingress_metadata.temp_counter, integer_count_register, 0); // only one instance of this 
  //add_to_field(ingress_metadata.temp_counter, number_header.counter_value ); // increment the counter of the received word
  //register_write(integer_count_register, 0, ingress_metadata.temp_counter); // update register file

  
  // load the current sums and update them by the integers of the given packet
  register_read(ingress_metadata.temp_sum, integer_sum_register, 0); // only one instance of this 
  add_to_field(ingress_metadata.temp_sum, number_header.number_value); // add to the sums
  register_write(integer_sum_register, 0, ingress_metadata.temp_sum); // update register file
  
  // don't forward the packet

  drop();
}

table sum_table {
  actions {
    add_to_sum;
  }
}



action forward_to_reducer() {
  // forwarding is done pretty simply

  // 1. Read the sum register file, clear it and store the value in the received packet
  register_read(ingress_metadata.temp_sum, integer_sum_register, 0); // only one instance of this
  register_write(integer_sum_register, 0, 0); // clear the register file
  add_to_field(number_header.number_value, ingress_metadata.temp_sum); 


  // 2. Read the counter value. clear it and store it in the received packet 
  register_read(ingress_metadata.temp_counter, integer_count_register, 0); // only one instance of this 
  register_write(integer_count_register, 0, 0); // clear the register file
  add_to_field(number_header.counter_value, ingress_metadata.temp_counter);


  modify_field(standard_metadata.egress_spec, OUTPUT_PORT); // update the output port number (forward)
} 

table forward_to_reducer_table {
  actions {
    forward_to_reducer; // reducer as another switch
  }

}


control ingress {

    // Check the flags in order to make a decision
    if(valid(number_header))
    {
       if(number_header.flags == DATA_PACKET_FLAG /*constant from headers.p4*/) // means a data packet -- no forwarding
       {
         apply(sum_table);
       }
 
       else // means a forwarding request has been received
       {
          apply(forward_to_reducer_table);
       }

   }

}

