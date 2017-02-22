#!/bin/bash
SHADER_SOURCE_DIR=$1
SHADER_COMPILER=$2
GLSL_VERSION=$3
BUF_COUNT=$4
SHADER_BINARY_DIR=shaders

if [ ! -d ${SHADER_BINARY_DIR} ]; then
    mkdir ${SHADER_BINARY_DIR}
fi

for filename in ${SHADER_SOURCE_DIR}/*.comp; do
    TMP_FILE="${filename}.tmp"
    cp ${filename} ${TMP_FILE}
    echo "#version ${GLSL_VERSION} core" > ${filename}
    echo "#define BUFFER_COUNT ${BUF_COUNT}" >> ${filename}
    cat ${TMP_FILE} >> ${filename}

    SPV_FILE="${filename}.spv"
    $2 -V "${filename}" -o "${SPV_FILE}"
    mv ${TMP_FILE} ${filename}
done

for filename in ${SHADER_SOURCE_DIR}/*.vert; do
    TMP_FILE="${filename}.tmp"
    cp ${filename} ${TMP_FILE}
    echo "#version ${GLSL_VERSION} core" > ${filename}
    echo "#define BUFFER_COUNT ${BUF_COUNT}" >> ${filename}
    cat ${TMP_FILE} >> ${filename}

    SPV_FILE="${filename}.spv"
    $2 -V "${filename}" -o "${SPV_FILE}"
    mv ${TMP_FILE} ${filename}
done

mv ${SHADER_SOURCE_DIR}/*.spv ${SHADER_BINARY_DIR}
