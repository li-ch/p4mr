#define PREAMBLE_SIZE 64
#define ROUTE_BITS 8 
#define APP_BITS 8 
#define RESUME_BITS 8 
#define INT_BITS 64 
#define DEFAULT_COLLECTION 0x00 
#define ASSIGNED_ID 0x00 
#define NEXT_APP 0x00


/***********************
   Header Definition
 **********************/

header_type data_header_t {
    fields {
        preamble       : PREAMBLE_SIZE;
        application_id : APP_BITS;
        routing_id     : ROUTE_BITS;
        resume_signal  : RESUME_BITS;
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

// METADATA
header_type egress_metadata_t {
  fields {
     temp_result : INT_BITS;
  }  
}

metadata egress_metadata_t egress_metadata;

// REGISTER
register my_result {
    width : INT_BITS;
    instance_count : 16; 
}

// ACTIONS
action _drop() {
    drop();
}

action reset_port(next_hop) {
    modify_field (standard_metadata.egress_spec, next_hop); 
}

action my_update() {
    register_read (egress_metadata.temp_result, my_result, 0x00); 
    add_to_field (egress_metadata.temp_result, data_header.value); 
    register_write (my_result, 0x00, egress_metadata.temp_result); 

    modify_field (data_header.routing_id, NEXT_APP);
}

action my_collection() {
    register_read (egress_metadata.temp_result, my_result, 0x00); 
    modify_field (data_header.value, egress_metadata.temp_result);

    modify_field (data_header.routing_id, DEFAULT_COLLECTION);
}

// TABLES
table forward_table {
    reads {
        data_header.routing_id : exact;
    }
    actions {
        reset_port;
    }
}

table update_table {
    actions {
        my_update;
        // _drop;
    }
}

table collection_table {
    actions {
        my_collection;
    }
}

// CONTROLS
control ingress {
  if ( valid(data_header) ) { // if the data header is successfully parsed,
      apply (forward_table);
      if (data_header.resume_signal != 0) {
          if (data_header.application_id == ASSIGNED_ID) {
              apply (collection_table);
          }
      }
      else {
          if (data_header.routing_id == ASSIGNED_ID) {
              if (data_header.routing_id != DEFAULT_COLLECTION) {  
                  apply (update_table);
              }
          }
      }
  } 
}
