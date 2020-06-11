#!/usr/bin/env bash

# abort in case of error
set -e

PROJECT_PATH="$(dirname `realpath "${BASH_SOURCE[0]}"`)"
SCRIPT_NAME="$(basename `realpath "${BASH_SOURCE[0]}"`)"
OLDDIR="$(pwd)"

OPTION="${1}"

USAGE="
Usage: ${SCRIPT_NAME} [option | target]

Options:
    -a, --aliases               display aliases for these functions and exit
                                (redirect this output to your .bash_aliases)
    -h, --help                  display this help and exit

Targets:
    --uarm, --uarm=action       select uarm target and action
    --umps, --umps=action       select umps target and action
    --all,  --all=action        select all targets and action

Actions:
    b, build                    build for the selected architecture
    r, run                      run the emulator of the selected architecture

    If the action is not specified it builds and runs the selected target.

If no target is specified, by default it will build and run all.
"

ALIASES="
alias bikaya=\"${PROJECT_PATH}/${SCRIPT_NAME}\"
"

INVALID_ARGUMENT="
${SCRIPT_NAME}: Invalid argument.
Try '${SCRIPT_NAME} --help' for more information."

function build_uarm {
    cd ${PROJECT_PATH}/build/uarm/debug
    rm -rf "${PROJECT_PATH}/build/uarm/debug/*"
    cmake -DTARGET_ARCH=uARM ${PROJECT_PATH}
    make
    cp kernel* ${PROJECT_PATH}/emulators/config/uarm
}

function run_uarm {
    cd ${PROJECT_PATH}/emulators/config/uarm
    nohup uarm &
}

function build_umps {
    cd ${PROJECT_PATH}/build/umps/debug
    rm -rf "${PROJECT_PATH}/build/umps/debug/*"
    cmake -DTARGET_ARCH=uMPS ${PROJECT_PATH}
    make
    cp kernel* ${PROJECT_PATH}/emulators/config/umps
}

function run_umps {
    cd ${PROJECT_PATH}/emulators/config/umps
    nohup umps2 &
}

case "${OPTION}" in
    "-a" | "--aliases")
        echo "${ALIASES}"
        exit 0
        ;;

    "-h" | "--help")
        echo "${USAGE}"
        exit 0
        ;;

    "--uarm")
       build_uarm
       run_uarm
       ;;

    "--uarm=b" | "--uarm=build")
       build_uarm
       ;;

    "--uarm=r" | "--uarm=run")
       run_uarm
       ;;

    "--umps")
       build_umps
       run_umps
       ;;

    "--umps=b" | "--umps=build")
       build_umps
       ;;

    "--umps=r" | "--umps=run")
       run_umps
       ;;

    "" | "--all")
      build_uarm
      build_umps
      run_uarm
      run_umps
      ;;

    "--all=b" | "--all=build")
      build_uarm
      build_umps
      ;;

    "--all=r" | "--all=run")
      run_uarm
      run_umps
      ;;

     *)
       echo "${INVALID_ARGUMENT}"
       exit 1
       ;;
esac

cd ${OLDDIR}
