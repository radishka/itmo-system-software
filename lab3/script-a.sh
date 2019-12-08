#!/bin/bash

if [ -f $1 ]; then
  line=$(ls -n $1)
  uid=$(echo "$line" | gawk '{print $3}')
  gid=$(echo "$line" | gawk '{print $4}')
  up=$(echo "$line" | gawk -F '' '{print $3}')
  gp=$(echo "$line" | gawk -F '' '{print $6}')
  op=$(echo "$line" | gawk -F '' '{print $9}')

  getent passwd | gawk -v uid="$uid" -v gid="$gid" -v up="$up" -v gp="$gp" -v op="$op" -F: '{if($3==uid && up=="w") {print $1} else if ($4==gid && gp=="w"){print $1} else if (op=="w" && $3!=uid){print $1}}'

else
  echo "No such file."
  exit 1
fi