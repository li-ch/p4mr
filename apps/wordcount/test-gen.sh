#!/bin/bash
head -n 10 data/huwikisource-latest-pages-meta-current.xml | python gen_pkt_stream.py
