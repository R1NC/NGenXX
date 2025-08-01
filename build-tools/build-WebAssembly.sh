#!/bin/bash

EMSCRIPTEN_ROOT=${EMSCRIPTEN_ROOT:-"~/dev/emsdk/upstream/emscripten"}
DEBUG=0

BUILD_DIR=../build.WebAssembly
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4wasm() {
    BUILD_TYPE=$1
    ABI=$2

    cmake .. \
    -DEMSCRIPTEN_ROOT_PATH=${EMSCRIPTEN_ROOT} \
    -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DEMSCRIPTEN_SYSTEM_PROCESSOR=${ABI}

    cmake --build . --config ${BUILD_TYPE}

    OUTPUT_DIR=output/${ABI}
    mkdir -p ${OUTPUT_DIR}
    cp NGenXX.wasm ${OUTPUT_DIR}
    cp NGenXX.js ${OUTPUT_DIR}
    cp NGenXX.html ${OUTPUT_DIR}

    #Checking Artifacts
    ARTIFACTS=(
        "${OUTPUT_DIR}/NGenXX.wasm"
        "${OUTPUT_DIR}/NGenXX.js"
        "${OUTPUT_DIR}/NGenXX.html"
    )
    for a in "${ARTIFACTS[@]}"; do
        if [ ! -f "$a" ]; then
            echo "ARTIFACT NOT FOUND: $a"
            exit 1
        fi
    done
}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

build4wasm $LIB_TYPE arm