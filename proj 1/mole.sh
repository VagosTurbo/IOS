#!/bin/bash

export POSIXLY_CORRECT=yes

print_help(){
    echo "Mole program"
    echo 
    echo "sposoby pouzita:"
    echo "mole -h"
    echo "mole [-g GROUP] FILE"
    echo "mole [-m] [FILTERS] [DIRECTORY]"
    echo "mole list [FILTERS] [DIRECTORY]"
    echo "mole secret-log [-b DATE] [-a DATE] [DIRECTORY1 [DIRECTORY2 [...]]]"
}

EDITOR=
VISUAL=
GROUP="-"
DIRECTORY=$(pwd)
DATE=$(date +%Y-%m-%d)
TIME=$(date +%H-%M)

while [ "$#" -gt 0 ]; do
    case $1 in
    "-h") print_help
    exit 0
    ;;
    "-g")
    GROUP=$2
    shift
    shift
    ;;
    "-m")
    ;;
    esac
    break
done
FILE=$1


if [ -z "${EDITOR}" ]; then
    if [ -z "${VISUAL}" ]; then
        EDITOR=vi
    else 
        EDITOR=${VISUAL}
    fi
fi    

eval ${EDITOR} ${FILE}
echo ${GROUP}";"${DIRECTORY}";"${FILE}";"${DATE}"_"${TIME}>> ${MOLE_RC} 
