#!/bin/bash

USER_DETAILS=$(getent passwd $1)

if [[ ! -z $1 && ! -z $USER_DETAILS ]]
then
  USER_NAME=$(echo "$USER_DETAILS" | cut -d: -f1)
  USER_ID=$(echo "$USER_DETAILS" | cut -d: -f3)
  USER_GROUPS=($(groups "$USER_NAME" | tr " " "\n"))

  for i in "${!USER_GROUPS[@]}"
  do
    USER_GROUPS[i]=$(getent group "${USER_GROUPS[i]}" | cut -d: -f3)
  done

  for file in $(ls -a)
  do
    if [[ "$file" != ".." && -d "$file" && ! -L "$file" ]]
    then
      DIRECTORY_DETAILS=$(ls -ldan -- "$file" | tr -s ' ')

      DIRECTOTY_UID=$(echo "$DIRECTORY_DETAILS" | cut -d ' ' -f3)
      DIRECTOTY_GID=$(echo "$DIRECTORY_DETAILS" | cut -d ' ' -f4)
      DIRECTOTY_OWNER_RIGHTS=$(echo "$DIRECTORY_DETAILS" | cut -c2-4)
      DIRECTOTY_GROUP_RIGHTS=$(echo "$DIRECTORY_DETAILS" | cut -c5-7)
      DIRECTOTY_OTHERS_RIGHTS=$(echo "$DIRECTORY_DETAILS" | cut -c8-10)

      MATCH=1

      if [[ $USER_ID == $DIRECTOTY_UID ]]
      then
        [[ ${DIRECTOTY_OWNER_RIGHTS} =~ ..x ]] && MATCH=0;
      elif [[ "${USER_GROUPS[*]}" =~ ${DIRECTOTY_GID} ]]
      then
        [[ ${DIRECTOTY_GROUP_RIGHTS} =~ ..x ]] && MATCH=0;
      else
        [[ ${DIRECTOTY_OTHERS_RIGHTS} =~ ..x ]] && MATCH=0;
      fi

      if [[ $MATCH == 0 ]]
      then
        if [[ $file == '.' ]]
        then
          echo "$file"
        else
          echo "./$file"
        fi
      fi
    fi
  done
fi
