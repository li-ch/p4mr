#!/bin/bash
# This script need sudo priviledges
head -n 10 data/huwikisource-latest-pages-meta-current.xml | python gen_pkt_stream.py
