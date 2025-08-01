#!/bin/bash

#TODO
DEBUG=0
TARGET_VERSION=15.0

BUILD_DIR=../build.iOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4ios() {
    PLATFORM=$1
    ARCHS=$2
    SYSTEM_VERSION=$3
    BUILD_TYPE=$4

    if [ $PLATFORM = "sim" ]; then
        PLATFORM="SIMULATOR64"
    else
        PLATFORM="OS64"
    fi

    #Generate xcode project for debug
    GEN_XCODE_PROJ=""
    if [ $BUILD_TYPE = "Debug" ]; then
        GEN_XCODE_PROJ="-G Xcode"
    fi
    
    cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=build-tools/toolchains/Apple/apple.toolchain.cmake \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DARCHS=${ARCHS} \
    -DDEPLOYMENT_TARGET=${SYSTEM_VERSION} \
    -DPLATFORM=${PLATFORM} \
    ${GEN_XCODE_PROJ}

    cmake --build . --config ${BUILD_TYPE}
}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

build4ios phone arm64 ${TARGET_VERSION} ${LIB_TYPE}

#Copy headers
HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp -R ../include/ ${HEADER_OUTPUT_DIR}/

#Copy libs
LIB_DIR=""
COMMAND="strip -x -S"
COMMAND_ARG="-o"
if [ $DEBUG == 1 ]; then
    LIB_DIR="Debug-iphoneos/"
    COMMAND="mv"
    COMMAND_ARG=""
fi
LIB_OUTPUT_DIR=output/libs
mkdir -p ${LIB_OUTPUT_DIR}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/NGenXX.a"
    "${LIB_OUTPUT_DIR}/curl.a"
    "${LIB_OUTPUT_DIR}/ssl.a"
    "${LIB_OUTPUT_DIR}/crypto.a"
    "${LIB_OUTPUT_DIR}/lua.a"
    "${LIB_OUTPUT_DIR}/qjs.a"
    "${LIB_OUTPUT_DIR}/spdlog.a"
    "${LIB_OUTPUT_DIR}/uv.a"
    "${LIB_OUTPUT_DIR}/cjson.a"
    "${LIB_OUTPUT_DIR}/mmkvcore.a"
    "${LIB_OUTPUT_DIR}/mmkv.a"
)
${COMMAND} ${LIB_DIR}libNGenXX.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/NGenXX.a
${COMMAND} curl-build/lib/${LIB_DIR}libcurl.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/curl.a
${COMMAND} openssl-build/ssl/${LIB_DIR}libssl.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/ssl.a
${COMMAND} openssl-build/crypto/${LIB_DIR}libcrypto.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/crypto.a
${COMMAND} lua-build/${LIB_DIR}liblua.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/lua.a
${COMMAND} quickjs-build/${LIB_DIR}libqjs.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/qjs.a
${COMMAND} spdlog-build/${LIB_DIR}libspdlog.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/spdlog.a
${COMMAND} libuv-build/${LIB_DIR}libuv.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/uv.a
${COMMAND} cjson-build/${LIB_DIR}libcjson.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/cjson.a
${COMMAND} mmkv-build/Core/${LIB_DIR}/libmmkvcore.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/mmkvcore.a
${COMMAND} mmkv-build/${LIB_DIR}libmmkv.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/mmkv.a
ADA_OUT_FILE=AdaURL-build/src/${LIB_DIR}libada.a
if [ -f "$ADA_OUT_FILE" ]; then
    libAda=${LIB_OUTPUT_DIR}/ada.a
    ${COMMAND} ${ADA_OUT_FILE} ${COMMAND_ARG} ${libAda}
    ARTIFACTS+=(${libAda})
fi

#Checking Artifacts
for a in "${ARTIFACTS[@]}"; do
    if [ ! -f "$a" ]; then
        echo "ARTIFACT NOT FOUND: $a"
        exit 1
    fi
done
