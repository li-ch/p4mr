#!/usr/bin/python

# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from scapy.all import sniff, sendp
from scapy.all import Packet
from scapy.all import ShortField, IntField, LongField, BitField

import networkx as nx

import sys
import random

class SrcRoute(Packet):
    name = "SrcRoute"
    fields_desc = [
        LongField("preamble", 0),
    ]


class CounterPacketPayload(Packet):
    name = "CounterPacketPayload"
    fields_desc = [ IntField("counter", 64), ]

class LongIntegerPayload(Packet):
    name = "CounterPacketPayload"
    fields_desc = [ LongField("longPayload", 0) ]    

def read_topo():
    nb_hosts = 0
    nb_mappers = 0
    nb_reducers = 0
    links = []
    with open("topo.txt", "r") as f:
        line = f.readline()[:-1]
        w, nb_mappers = line.split()
        assert(w == "mappers")

        line = f.readline()[:-1]
        w, nb_reducers = line.split()
        assert(w == "reducers")

        line = f.readline()[:-1]
        w, nb_hosts = line.split()
        assert(w == "hosts")
        for line in f:
            if not f: break
            a, b = line.split()
            links.append( (a, b) )
    return int(nb_hosts), int(nb_mappers), int(nb_reducers), links

def ascii_encode(msg):
    encoded = ''
    for c in msg:
        encoded += hex(ord(c))
    return encoded

def flag_assemble(pkt_type):
    if pkt_type == '0':
        return chr(0)
    elif pkt_type == '1':
        return chr(1)
    else:
        print "Unexpected error:", sys.exc_info()[0]
        raise

def word_assemble(msg, msg_size):
    l = msg
    # Paddings
    while len(l) < msg_size:
        l += 'o'
    l = l[:msg_size]
    return l


def generate_random_long_integer():  
    
    randomLong = random.getrandbits(64) # always 64 bits
       
    return LongIntegerPayload(longPayload=randomLong) # create a 64-bit integer for a new packet


def main():
    if len(sys.argv) != 4:
        print "Usage: send.py [this_host] [target_host] [pkt_type]"
        print "For example: send.py h1 h3 0"
        sys.exit(1)

    src, dst, pkt_type = sys.argv[1:]

    nb_hosts, nb_mappers, nb_reducers, links = read_topo()

    port_map = {}

    for a, b in links:
        if a not in port_map:
            port_map[a] = {}
        if b not in port_map:
            port_map[b] = {}

        assert(b not in port_map[a])
        assert(a not in port_map[b])
        port_map[a][b] = len(port_map[a]) + 1
        port_map[b][a] = len(port_map[b]) + 1


    G = nx.Graph()
    for a, b in links:
        G.add_edge(a, b)

    shortest_paths = nx.shortest_path(G)
    shortest_path = shortest_paths[src][dst]

    print "path is:", shortest_path

    port_list = []
    first = shortest_path[1]
    for h in shortest_path[2:]:
        port_list.append(port_map[first][h])
        first = h

    print "port list is:", port_list

    port_str = ""
    for p in port_list:
        port_str += chr(p)

    while(1):
        
        
        FIXED_INTEGER_NUMBER = 64 # has to be taken from headers.p4 
        
        p = SrcRoute()
        
        # add a number of 64-bit unsigned integers
        counter = 0
        while counter <  FIXED_INTEGER_NUMBER:
            p.add_payload(generate_random_long_integer())
            counter = counter + 1 

        # add a counter and flags
        p.add_payload(CounterPacketPayload(counter=FIXED_INTEGER_NUMBER))
        p.add_payload(flag_assemble(pkt_type))
  
        
        print p.show()  # print msg
       
        sendp(p, iface = "eth0")

        msg = raw_input("Do you want to send another packet (yes/no) ?  ")
        
        if msg != "yes":
            break  

        pkt_type = raw_input("Please enter the type of a new packet (0 - data, 1 - reduce): ")      

       

if __name__ == '__main__':
    main()
