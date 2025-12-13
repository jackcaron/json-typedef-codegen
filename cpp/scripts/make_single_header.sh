#!/bin/bash

error_echo() {
  echo "$@" >&2
  exit 1
}

SRC_DIR=$(realpath $1)
DST_DIR=$(realpath $2)
ALL_DEFINES=$3
COMPILER=$4

[[ -z $SRC_DIR ]] && error_echo "ERROR: missing source directory"
[[ -d $SRC_DIR ]] || error_echo "ERROR: cannot find source directory ${SRC_DIR}"
[[ -z $DST_DIR ]] && error_echo "ERROR: missing destination directory"
[[ -d $DST_DIR ]] || error_echo "ERROR: cannot find destination directory ${DST_DIR}"
[[ -z $ALL_DEFINES ]] && error_echo "ERROR: missing defines"
[[ -z $COMPILER ]] && error_echo "ERROR: missing compiler"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
SRC_INC_DIR="$( readlink -m "${SCRIPT_DIR}/../include" )"

# base source file
BASE_SRC_FILE=$(realpath "${SRC_DIR}/base.cpp")
printf "" > $BASE_SRC_FILE # reset the base file
for HDR in "${SRC_INC_DIR}"/*; do
    F_NAME=$(basename $HDR)
    # echo "header file: -> ${F_NAME}"
    printf "#include \"${F_NAME}\"\n" >> $BASE_SRC_FILE

    # parse the system headers
    mapfile -t SYS_HDRS <<< $(cat $HDR | grep "^\s*#include <\(.*\)>\s*$")
    for LINE in "${SYS_HDRS[@]}"; do
        if [[ -n $LINE ]]; then
            F_NAME=$(echo "$LINE" | sed -n 's/^#include <\(.*\)>$/\1/p')
            B_NAME=$(basename $F_NAME)
            if [[ $F_NAME != $B_NAME ]]; then
                D_NAME=$(dirname $F_NAME)
                mkdir -p "${SRC_DIR}/${D_NAME}"
            fi
            touch "${SRC_DIR}/${F_NAME}"
        fi
    done
done

IFS=';' read -r -a DEFINES <<< "$ALL_DEFINES"

CMD="${COMPILER} -E -P -nostdinc++ -isystem ${SRC_DIR} -I${SRC_INC_DIR} -I ${SRC_DIR}"
for d in "${DEFINES[@]}"; do
    CMD="${CMD} -D${d}"
done
CMD="${CMD} ${BASE_SRC_FILE}"

# start single file
SINGLE_FILE="${DST_DIR}/json_typedef.hpp"
GUARD="JSON_TYPEDEF"
printf "\n#ifndef ${GUARD}\n#define ${GUARD}\n\n" > $SINGLE_FILE # reset the file

# grab the used header files, based on the preprocessor
MULTI_INC_GUARD="\"Multiple include guards\""
BASE_0="\"base.o\""
mapfile -t USED_HDRS <<< $(eval "${CMD} -M -H 2>&1 | grep -ws ${MULTI_INC_GUARD} -A 200 | grep -v ${MULTI_INC_GUARD} | grep -ws ${BASE_0} -B 200 | grep -v ${BASE_0}")
PREFIX="${SRC_DIR}/"
for d in "${USED_HDRS[@]}"; do
    printf "#include <${d#$PREFIX}>\n" >> $SINGLE_FILE
done

# grab the combined header files
eval "${CMD} -C" >> $SINGLE_FILE

printf "\n#endif\n" >> $SINGLE_FILE
