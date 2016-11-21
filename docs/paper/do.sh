#!/bin/bash

make clean
make
cp main.pdf draft.tmp
make clean
mv draft.tmp main.pdf

