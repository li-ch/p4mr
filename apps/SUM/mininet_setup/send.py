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

from scapy.all import sendp
from scapy.all import Packet
#from scapy.all import ShortField, IntField, LongField, BitField
from scapy.all import  LongField
#from read_topo import read_topo

import time
import sys
import os

DATA_DIR = "../data_src"

class SrcRoute(Packet):
    name = "SrcRoute"
    fields_desc = [
        LongField("preamble", 0),
    ]

def data_allocation():
    """This is the test version to apply a fixed allocation
       Future version should get allocation information from
       the ast.json
    """
    data_srcs = {
                    "A": "3",
                    "B": "3",
                    "C": "4"
            }
    return data_srcs

def data_loading(file_path):
    data = []
    with open(file_path, 'r') as fstream:
        for line in fstream:
            data += line.strip().split(' ')
    return data

def ascii_encode(msg):
    encoded = ''
    for c in msg:
        encoded += hex(ord(c))
    return encoded

def flag_assemble(pkt_type):
    if pkt_type == '0':
        return chr(0) + chr(0)
    elif pkt_type == '1':
        return chr(0) + chr(1)
    elif pkt_type == '2':
        return chr(0) + chr(2)
    elif pkt_type == '5':
        return chr(0) + chr(5)
    else:
        print "Unexpected error:", sys.exc_info()[0]
        raise

def msg_assemble(msg, fix_size):
    assert(fix_size % 8, 0)
    bytes_num = fix_size / 8
    l = bin(int(msg))[2:]
    bits_num = len(l)
    # Paddings
    if bits_num < fix_size:
        l = '0'*(fix_size - bits_num) + l
    i = 1
    pkt_msg = ""
    while i <= bytes_num:
        pkt_msg += chr( int(l[(i-1)*8: i*8], 2) )
        i += 1
    return pkt_msg


def main():
    data_srcs = data_allocation()
    """
    if len(sys.argv) != 4:
        print "Usage: send.py [this_host] [target_host] [pkt_type]"
        print "For example: send.py h1 h3 0"
        sys.exit(1)
    src, dst, pkt_type = sys.argv[1:]
    """
    # Start operations
    for label in data_srcs:
        file_path = DATA_DIR + os.sep + label + ".txt"
        int_values = data_loading(file_path)
        init_appID = data_srcs[label]
        for v in int_values:
            p = SrcRoute() / \
                    msg_assemble(init_appID, 8) / \
                    msg_assemble(init_appID, 8) / \
                    msg_assemble('0', 8) / \
                    msg_assemble(v, 64)

            print p.show()
            sendp(p, iface = "eth0")
            time.sleep(0.1)
            # print msg

    # Start results collection
    for appID in set(data_srcs.values()):
        p = SrcRoute() / \
                    msg_assemble(appID, 8) / \
                    msg_assemble(appID, 8) / \
                    msg_assemble('1', 8) / \
                    msg_assemble('0', 64)

        print p.show()
        sendp(p, iface = "eth0")
        time.sleep(0.1)

if __name__ == '__main__':
    main()
