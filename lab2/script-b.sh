#!/usr/bin/bash

USAGE="Usage: $0 directory_name";
[[ ! "$#" -eq 1 ]] && { echo "$USAGE"; exit 1; }

gfind "$1" -name "*[^a-zA-Z]*" -printf "%s %f\n" | sort -n -k 1 | cut -d" " -f2-
