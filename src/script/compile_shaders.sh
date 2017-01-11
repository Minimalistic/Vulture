#!/bin/bash
SHADER_SOURCE_DIR=$1
SHADER_COMPILER=$2
SHADER_BINARY_DIR=shaders

if [ ! -d ${SHADER_BINARY_DIR} ]; then
    mkdir ${SHADER_BINARY_DIR}
fi

for filename in ${SHADER_SOURCE_DIR}/*.comp; do
    SPV_FILE="${filename}.spv"
    $2 -V "${filename}" -o "${SPV_FILE}"
done
mv ${SHADER_SOURCE_DIR}/*.spv ${SHADER_BINARY_DIR}
