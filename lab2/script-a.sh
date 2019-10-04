#!/usr/bin/bash

YEAR=$(date +%Y)
MONTH=$(date +%b)
DAY=$(date +%d)

/usr/lib/acct/fwtmp < /var/adm/wtmpx | grep "  6 .*$MONTH.*$DAY.*$YEAR" | grep -v "LOGIN" |  cut -d" " -f1 | sort -u
