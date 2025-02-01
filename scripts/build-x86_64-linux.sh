#!/bin/bash

# -------- CMake
PLATFORM_BUILD_DIR="x86_64-linux-release"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PACKAGE_ROOT_DIR=$(realpath ${SCRIPT_DIR}/..)
CMAKE_BINARY_DIR="${PACKAGE_ROOT_DIR}/build/${PLATFORM_BUILD_DIR}"
CMAKE_BUILD_TYPE="Release"
CMAKE_INSTALL_PREFIX="${PACKAGE_ROOT_DIR}/output/${PLATFORM_BUILD_DIR}"

cd "${PACKAGE_ROOT_DIR}"
rm -rf ${CMAKE_BINARY_DIR}
rm -rf ${CMAKE_INSTALL_PREFIX}

if [[ -v "HW3_DEPS" ]]; then
    echo "* local build"
    export PLATFORM_BUILD_DIR=x86_64-linux-release
    export HW3_SCRIPTS_CMAKE_DIR=$HW3_DEPS/scripts/cmake
    export LIBDIR=lib
fi

cmake -S ${PACKAGE_ROOT_DIR} -B "${CMAKE_BINARY_DIR}" \
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
            -DPLATFORM_BUILD_DIR=$PLATFORM_BUILD_DIR \
            -DHW3_SCRIPTS_CMAKE_DIR=$HW3_SCRIPTS_CMAKE_DIR -DLIBDIR=$LIBDIR\
            -DGENERATE_DEB=ON \
            -G "Unix Makefiles"
    result=$?
    if [ $result != 0 ]; then
        echo "CMake installation stage failed (error: ${result})"
        exit 3
    fi

cmake --build "${CMAKE_BINARY_DIR}" --config "${CMAKE_BUILD_TYPE}" -j $(nproc)
    result=$?
    if [ $result != 0 ]; then
        echo "CMake installation stage failed (error: ${result})"
        exit 3
    fi

cmake --build "${CMAKE_BINARY_DIR}" --target package --config "${CMAKE_BUILD_TYPE}" -j $(nproc)
    result=$?
    if [ $result != 0 ]; then
        echo "CMake installation stage failed (error: ${result})"
        exit 3
    fi

cmake --install "${CMAKE_BINARY_DIR}" --config "${CMAKE_BUILD_TYPE}"
    result=$?
    if [ $result != 0 ]; then
        echo "CMake installation stage failed (error: ${result})"
        exit 3
    fi

