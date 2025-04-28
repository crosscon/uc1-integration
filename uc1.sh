#!/usr/bin/env bash

ROOT_DIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
OUT_DIR="${ROOT_DIR}/out"
RESOURCES_DIR="${ROOT_DIR}/resources/building-bao-hypervisor"
ZEPHYR_WORKSPACE="${ROOT_DIR}/../"

BM_DIR="${ROOT_DIR}/bao-baremetal-guest"
HV_DIR="${ROOT_DIR}/bao-hypervisor"

DOCKER_IMAGE_BAO="bao-hypervisor-image"


errorExit() {
    errorMessage="$@"
    echo "$errorMessage"
    exit 1
}

errorCheck() {
    errorCode=$?
    errorMessage="$@"
    [ "$errorCode" -ne 0 ] && errorExit "$errorMessage : ($errorCode)"
}

usage() {
cat <<EOF
Usage: ./$(basename ${0}) command [options]

This is wrapper for building/flashing/running UC1 stages

  Commands:
    build         build HV and VMs
    flash         flash HV and VMs
    hv_start      start HV via GDB (HV still cannot boot by itself?) 
    gdb_start     start GDB session with no extra commands
EOF
  exit 0
}

docker_run() {
  [ -z "${DOCKER_IMAGE}" ] && errorExit "DOCKER_IMAGE not set"

  docker run \
    --rm -it \
    -v ${PWD}:${PWD} \
    -w ${PWD} \
    --privileged \
    -v /dev:/dev \
    -e ZEPHYR_BASE=${PWD}/zephyr \
    ${DOCKER_IMAGE} "$@"
  errorCheck "Failed to run '$@' in Docker image: ${DOCKER_IMAGE}"
}

build_zephyr() {
  export DOCKER_IMAGE="ghcr.io/zephyrproject-rtos/zephyr-build:v0.27.5"
  if pushd "${ZEPHYR_WORKSPACE}" &> /dev/null; then
    # TODO: add script argument/env variable for this
    #
    # Uncomment one of these to select which Zephyr app should be built as a VM
    # WIFI APP with shield
    docker_run west build -b lpcxpresso55s69/lpc55s69/cpu0/ns --shield mikroe_wifi_bt_click_mikrobus ./crosscon-uc1-1/wifi_app/ -p
    # Hello world from our repo
    # docker_run west build -b lpcxpresso55s69/lpc55s69/cpu0/ns ./crosscon-uc1-1/hello_world/ -p
    # Hello world from Zephyr repo
    # docker_run west build -b lpcxpresso55s69/lpc55s69/cpu0/ns zephyr/samples/hello_world -p
    cp "build/zephyr/zephyr.bin" "${OUT_DIR}/vm1.bin"
    cp "build/zephyr/zephyr.elf" "${OUT_DIR}/vm1.elf"
    export VM1_START=$(printf "0x%08x\n" $((0x$(nm build/zephyr/zephyr.elf | awk '/__start/ {print $1}') - 1)))
    echo "VM1_START extracted from zephyr.elf: ${VM1_START}"
    popd &> /dev/null
  fi
  export -n DOCKER_IMAGE
}

build_bm() {
  export DOCKER_IMAGE="bao-hypervisor-image"
  if pushd "${BM_DIR}" &> /dev/null; then
    docker_run make clean
    docker_run make PLATFORM=lpc55s69 GUEST=0
    cp "${BM_DIR}/build/lpc55s69/baremetal.bin" "${OUT_DIR}/vm0.bin"
    cp "${BM_DIR}/build/lpc55s69/baremetal.elf" "${OUT_DIR}/vm0.elf"
    popd &> /dev/null
  fi
  export -n DOCKER_IMAGE
}

build_hv() {
  [ -z "${VM1_START}" ] && errorExit "VM1_START not set, build Zephyr first to retrieve it"

  export DOCKER_IMAGE="bao-hypervisor-image"
  if pushd "${HV_DIR}" &> /dev/null; then
    # cp "${RESOURCES_DIR}/config.c" "${HV_DIR}/configs/lpc55.c"
    cp "${RESOURCES_DIR}/config_single_vm_zephyr.c" "${HV_DIR}/configs/lpc55.c"
    sed -i "${HV_DIR}/configs/lpc55.c" -e "s/@@ZEPHYR_VM_ENTRY@@/${VM1_START}/" 
    docker_run make clean
    docker_run make PLATFORM=lpc55s69 CONFIG=lpc55 DEBUG=y
    cp "${HV_DIR}/bin/lpc55s69/lpc55/bao.bin" "${OUT_DIR}"
    cp "${HV_DIR}/bin/lpc55s69/lpc55/bao.elf" "${OUT_DIR}"
    popd &> /dev/null
  fi
  export -n DOCKER_IMAGE
  export -n VM1_START
}

flash() {
  export DOCKER_IMAGE="bao-hypervisor-image"
  docker_run LinkServer flash LPC55S69:LPCXpresso55S69 load ${OUT_DIR}/bao.elf
  docker_run LinkServer flash LPC55S69:LPCXpresso55S69 load ${OUT_DIR}/vm0.elf
  docker_run LinkServer flash LPC55S69:LPCXpresso55S69 load ${OUT_DIR}/vm1.elf
  export -n DOCKER_IMAGE
}

gdb_start() {
  export DOCKER_IMAGE="bao-hypervisor-image"
  docker_run ./gdb_start.sh
  export -n DOCKER_IMAGE
}

hv_start() {
  export DOCKER_IMAGE="bao-hypervisor-image"
  docker_run ./hv_start.sh
  export -n DOCKER_IMAGE
}

CMD="$1"

case "$CMD" in
  "flash")
    flash
    ;;
  "hv_start")
    hv_start
    ;;
  "gdb_start")
    gdb_start
    ;;
  "build")
    rm -rf "${OUT_DIR}"
    mkdir -p "${OUT_DIR}"
    build_zephyr
    build_bm
    build_hv
    ;;
  *)
    echo "Invalid command: \"${CMD}\""
    usage
    ;;
esac

