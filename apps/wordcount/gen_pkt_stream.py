#!/usr/bin/env python
import sys
from scapy.all import *

#For now we use port number to distinguish p4mr packets
HOST="127.0.0.1"
SPORT=30001
DPORT=30002

for l in sys.stdin:
    for word in l.strip().split(" "):
        p = IP(dst=HOST)/UDP(sport=SPORT, dport=DPORT)/Raw(word)
        send(p)
        print "\"{}\"\n".format(word)
