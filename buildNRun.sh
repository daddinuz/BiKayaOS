#!/usr/bin/env bash

#abort in case of error
set -e

PROJECT_PATH="$(dirname `realpath "${BASH_SOURCE[0]}"`)"
OLDDIR="$(pwd)"

function build_uarm {
    cd ${PROJECT_PATH}/build/uarm/debug
    rm -rf "${PROJECT_PATH}/build/uarm/debug/*"
    cmake -DTARGET_ARCH=uARM ${PROJECT_PATH}
    make
    cp kernel* ${PROJECT_PATH}/emulators/config/uarm
    cd ${PROJECT_PATH}/emulators/config/uarm
    nohup uarm &
}

function build_umps {
    cd ${PROJECT_PATH}/build/umps/debug
    rm -rf "${PROJECT_PATH}/build/umps/debug/*"
    cmake -DTARGET_ARCH=uMPS ${PROJECT_PATH}
    make
    cp kernel* ${PROJECT_PATH}/emulators/config/umps
    cd ${PROJECT_PATH}/emulators/config/umps
    nohup umps2 &
}

case "${1}" in
    "uarm")
        build_uarm
        ;;

    "umps")
        build_umps
        ;;

    "")
        build_uarm
        build_umps
        ;;

    *)
        echo 'Usage: "uarm" to compile for uarm, "umps" to compile for umps, nothing to compile for both.'
        exit 1
        ;;
esac

cd ${OLDDIR}
