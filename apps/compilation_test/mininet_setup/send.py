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


import sys

class SrcRoute(Packet):
    name = "SrcRoute"
    fields_desc = [
        LongField("preamble", 0),
    ]


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
    if len(sys.argv) != 4:
        print "Usage: send.py [this_host] [target_host] [pkt_type]"
        print "For example: send.py h1 h3 0"
        sys.exit(1)

    src, dst, pkt_type = sys.argv[1:]


    while(1):
        msg = raw_input("What do you want to send (integer only): ")

        p = SrcRoute() / \
                flag_assemble(pkt_type) / \
                msg_assemble(msg, 64)

        print p.show()
        sendp(p, iface = "eth0")
        # print msg

if __name__ == '__main__':
    main()
