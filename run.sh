#!/bin/bash

input_dir="input"
output_dir="out"

rm -r "$output_dir"/*

cp -r "$input_dir"/* in/

gcc -Wall -o p main.c

./p in out "$1"

exit 0
