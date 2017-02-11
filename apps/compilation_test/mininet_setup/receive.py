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

#from scapy.all import sniff, sendp
from scapy.all import sniff
#from scapy.all import Packet
#from scapy.all import ShortField, IntField, LongField, BitField
#from datetime import datetime

import re
import sys
#import struct

LOG_FILE = "myresults.txt"
def record_log(log_file, content):
    with open(log_file, "a") as fstream:
        fstream.write(content + "\n")

def str2binary(character):
    bin_original = bin(ord(character))[2:]
    length = len(bin_original)
    return '0'*(8-length) + bin_original

def parse_hex(hex_str):
    parsed_msg = ""
    hex_units = re.findall('0x[0-9a-fA-F][0-9a-fA-F]', hex_str)
    for c in hex_units:
        parsed_msg += chr( int(c, 0) )
    return parsed_msg

def assemble_fields(fields):
    return " | ".join( [str(ord(f)) for f in fields] )


def handle_pkt(pkt):
    pkt = str(pkt)
    # if len(pkt) < 12: return
    preamble = pkt[:8]
    preamble_exp = "\x00" * 8
    if preamble != preamble_exp: return
    """
    num_valid = struct.unpack("<L", pkt[8:12])[0]
    if num_valid != 0:
        print "received incorrect packet"
    """
    msg = pkt[10:]
    bitstream = "".join([str2binary(c) for c in msg])
    print bitstream
    print str(int(bitstream, 2))
    """
    print int(msg)
    print "The LENGTH of this PKT", len(msg)
    print bin(ord(msg[0]))
    print parse_hex(msg)
    """
    sys.stdout.flush()
    #record_log(LOG_FILE, str(int(msg)))

def main():
    #record_log(LOG_FILE, "\n" )
    #record_log(LOG_FILE, datetime.now().strftime('%Y-%m-%d %H:%M:%S') )
    sniff(iface = "eth0",
          prn = lambda x: handle_pkt(x))

if __name__ == '__main__':
    main()
