#!/usr/bin/env bash

PROJECT_PATH="$(pwd)"
SCP_USER="${1}"
SCP_HOST="${2}"
SUBMIT_DIR="${3}"
TAR_GZ_FILE="${PROJECT_PATH}/dist/BiKayaOS.tar.gz"

if [[ -z "${SCP_USER}" || -z "${SCP_HOST}" || -z "${SUBMIT_DIR}" ]];
then
    echo "Usage:"
    echo "  deploy.sh <user> <host> <submit_dir>"
    exit 1
fi

# exit on any error
set -e

# clean previous dist
rm -rf "${PROJECT_PATH}/dist/*"

# build the new dist
tar -pczf "${TAR_GZ_FILE}"                                  \
    --exclude="BiKayaOS/dist"                               \
    --exclude="BiKayaOS/.idea"                              \
    --exclude="BiKayaOS/.git"                               \
    --exclude="BiKayaOS/.gitkeep"                           \
    --exclude="BiKayaOS/.gitignore"                         \
    --exclude="BiKayaOS/emulators/uARM"                     \
    --exclude="BiKayaOS/emulators/umps"                     \
    --exclude="BiKayaOS/emulators/umps_uarm_hello_world"    \
    --exclude="BiKayaOS/emulators/config/uarm/*"            \
    --exclude="BiKayaOS/emulators/config/umps/*"            \
    --exclude="BiKayaOS/build/uarm/debug/*"                 \
    --exclude="BiKayaOS/build/umps/debug/*"                 \
    -C $(dirname "${PROJECT_PATH}") BiKayaOS

scp "${TAR_GZ_FILE}" "${SCP_USER}@${SCP_HOST}:/home/students/LABSO/2020/${SUBMIT_DIR}/lso20az06"
