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


parser start {
     
       return select(current(0, 64)) {
        0 : parse_word;
        default : ingress;
      }
}


header word_header_t word_header;

parser parse_word{
     extract(word_header);
     
     return select (word_header.flags) {
       0 : ingress;
       default : parse_word_map;
     }
}


header word_map_header_t map_reduce_header;

parser parse_word_map {
      extract(map_reduce_header);
      return ingress;   
}

