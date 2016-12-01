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

#include "includes/headers_reducer.p4"
#include "includes/parser_reducer.p4"


#define OUTPUT_PORT 4 
#define NUM_OF_MAPPERS 3

header_type ingress_metadata_t {
    fields {
       
        // Below are the values used for the MapReduce app
        temp_counter : COUNTER_SIZE; // a counter that has two functions: (1) stores the number of valid numbers for mean; 
                                     // (2) used for knowing how many mappers have sent a request to reduce.
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
  
  width : COUNTER_SIZE; // constant from headers.p4
  instance_count : 1;  
}


// register file for storing the number of packets that were forwarded from mappers
register count_mappers_register {
  width : COUNTER_SIZE; // constant from headers.p4
  instance_count : 1;
}

action add_to_sum() {

  // perform a particular number of additions (do not use resubmit since it is 
  // theoretically a slower way of implementing the same functionality).

  /************************************************* Long code goes here ******************************************/
                   /*************************** 32 additions ******************************/
  add_to_field(number_header.number_value_1, number_header.number_value_2);   // #1 = #1 + #2 
  add_to_field(number_header.number_value_3, number_header.number_value_4);   // #3 = #3 + #4
  add_to_field(number_header.number_value_5, number_header.number_value_6);   // #5 = #5 + #6
  add_to_field(number_header.number_value_7, number_header.number_value_8);   // #7 = #7 + #8
  add_to_field(number_header.number_value_9, number_header.number_value_10);  // #9 = #9 + #10 
  add_to_field(number_header.number_value_11, number_header.number_value_12); // #11 = #11 + #12
  add_to_field(number_header.number_value_13, number_header.number_value_14); // and so on
  add_to_field(number_header.number_value_15, number_header.number_value_16);
  add_to_field(number_header.number_value_17, number_header.number_value_18);
  add_to_field(number_header.number_value_19, number_header.number_value_20);
  add_to_field(number_header.number_value_21, number_header.number_value_22);
  add_to_field(number_header.number_value_23, number_header.number_value_24);
  add_to_field(number_header.number_value_25, number_header.number_value_26);
  add_to_field(number_header.number_value_27, number_header.number_value_28);  
  add_to_field(number_header.number_value_29, number_header.number_value_30);
  add_to_field(number_header.number_value_31, number_header.number_value_32);
  add_to_field(number_header.number_value_33, number_header.number_value_34);
  add_to_field(number_header.number_value_35, number_header.number_value_36);
  add_to_field(number_header.number_value_37, number_header.number_value_38);
  add_to_field(number_header.number_value_39, number_header.number_value_40);
  add_to_field(number_header.number_value_41, number_header.number_value_42);
  add_to_field(number_header.number_value_43, number_header.number_value_44);
  add_to_field(number_header.number_value_45, number_header.number_value_46);
  add_to_field(number_header.number_value_47, number_header.number_value_48);
  add_to_field(number_header.number_value_49, number_header.number_value_50);
  add_to_field(number_header.number_value_51, number_header.number_value_52);
  add_to_field(number_header.number_value_53, number_header.number_value_54);
  add_to_field(number_header.number_value_55, number_header.number_value_56); 
  add_to_field(number_header.number_value_57, number_header.number_value_58);
  add_to_field(number_header.number_value_59, number_header.number_value_60);
  add_to_field(number_header.number_value_61, number_header.number_value_62); // ...
  add_to_field(number_header.number_value_63, number_header.number_value_64); // #64 = #64 + #63

  
                 /*************************** 16 additions ******************************/

  add_to_field(number_header.number_value_1, number_header.number_value_3);   // #1 = #1 + #2 + #3 + #4
  add_to_field(number_header.number_value_5, number_header.number_value_7);   // #5 = #5 + #6 + #7 + #8
  add_to_field(number_header.number_value_9, number_header.number_value_11);  // and so on
  add_to_field(number_header.number_value_13, number_header.number_value_15);   
  add_to_field(number_header.number_value_17, number_header.number_value_19);  
  add_to_field(number_header.number_value_21, number_header.number_value_23); 
  add_to_field(number_header.number_value_25, number_header.number_value_27); 
  add_to_field(number_header.number_value_29, number_header.number_value_31);
  add_to_field(number_header.number_value_33, number_header.number_value_35);
  add_to_field(number_header.number_value_37, number_header.number_value_39);
  add_to_field(number_header.number_value_41, number_header.number_value_43);
  add_to_field(number_header.number_value_45, number_header.number_value_47);
  add_to_field(number_header.number_value_49, number_header.number_value_51);
  add_to_field(number_header.number_value_53, number_header.number_value_55);  
  add_to_field(number_header.number_value_57, number_header.number_value_59); // ...
  add_to_field(number_header.number_value_61, number_header.number_value_63); // #61 = #61 + #62 + #63 + #64 


                 /*************************** 8 additions ******************************/

  add_to_field(number_header.number_value_1, number_header.number_value_5);   // #1 = #1 + #2 + #3 + #4 + #5 + #6 + #7 + #8
  add_to_field(number_header.number_value_9, number_header.number_value_13);  // #9 = #9 + #10 + #11 + #12 + #13 + #14 + #15 + #16
  add_to_field(number_header.number_value_17, number_header.number_value_21); // and so on
  add_to_field(number_header.number_value_25, number_header.number_value_29);   
  add_to_field(number_header.number_value_33, number_header.number_value_37);  
  add_to_field(number_header.number_value_41, number_header.number_value_45); 
  add_to_field(number_header.number_value_49, number_header.number_value_53); // ...
  add_to_field(number_header.number_value_57, number_header.number_value_61); // #57 = #57 + #58 + #59 + #60 + #61 + #62 + #63 + #64


                 /*************************** 4 additions ******************************/

  add_to_field(number_header.number_value_1, number_header.number_value_9);   // #1 = #1 + #2 + #3 + #4 + #5 + #6 + #7 + #8 + 
                                                                  // + #9 + #10 + #11 + #12 + #13 + #14 + #15 + #16
  add_to_field(number_header.number_value_17, number_header.number_value_25); // and so on 
  add_to_field(number_header.number_value_33, number_header.number_value_41); // ...
  add_to_field(number_header.number_value_49, number_header.number_value_57); // #49 = #49 + ... + #64

  
                   /*************************** 2 additions ******************************/

  add_to_field(number_header.number_value_1, number_header.number_value_17);  // #1 = #1 + #2 + ... + #32
  add_to_field(number_header.number_value_33, number_header.number_value_49); // #33 = #33 + #34 + ... + #64  
  

                     /*************************** 1 addition ******************************/

  add_to_field(number_header.number_value_1, number_header.number_value_33);  // #1 = #1 + #2 + ... + #64

  /************************************************* Long code ends here ******************************************/
 
   // just take the number of integers and add it to the counter
  register_read(ingress_metadata.temp_counter, integer_count_register, 0); // only one instance of this 
  add_to_field(ingress_metadata.temp_counter, number_header.num_of_numbs); // increment the counter of the received word
  register_write(integer_count_register, 0, ingress_metadata.temp_counter); // update register file

  
  // load the current sums and update them by the integers of the given packet
  register_read(ingress_metadata.temp_sum, integer_sum_register, 0); // only one instance of this 
  add_to_field(ingress_metadata.temp_sum, number_header.number_value_1); // add the current sum to the summed value of data
  register_write(integer_sum_register, 0, ingress_metadata.temp_sum); // update register file
    

  // don't forward the packet
  drop();
}

table sum_table {
  actions {
    add_to_sum;
  }
}



action increment_map_counter() {
  
  // increment the number of received packets from mappers
  register_read(ingress_metadata.temp_counter, count_mappers_register, 0); // only one instance of this 
  add_to_field(ingress_metadata.temp_counter, 1); // increment the counter by 1
  register_write(count_mappers_register, 0, ingress_metadata.temp_counter); // update register file  
}

table update_map_count_table {
  actions {
    increment_map_counter;
  }
}

action forward_to_dest() {
  // forwarding is done pretty simply

  // 1. Read the sum register file and store the value in the received packet
  register_read(number_header.number_value_1, integer_sum_register, 0); // only one instance of this
  register_write(integer_sum_register, 0, 0); // clear the register file
  

  // 2. Read the counter value and store it in the received packet 
  register_read(number_header.num_of_numbs, integer_count_register, 0); // only one instance of this 
  register_write(integer_count_register, 0, 0); // clear the register file


  modify_field(standard_metadata.egress_spec, OUTPUT_PORT); // update the output port number (forward)
} 

table forward_to_dest_table {
  actions {
    forward_to_dest; // forward to a server
  }

}


control ingress {

    // Check the flags in order to make a decision
    if(valid(number_header))
    {

       apply(sum_table); // always sum as even the last packet carries data
       
       if(number_header.flags != DATA_PACKET_FLAG /*constant from headers.p4*/) // a request to reduce
       {
         apply(update_map_count_table);
    
         if(ingress_metadata.temp_counter >= NUM_OF_MAPPERS) // forwarding has to be performed
         {
            apply(forward_to_dest_table);
         }// if
       
       } // if
   }// if 

}

