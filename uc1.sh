#!/usr/bin/env bash

ROOT_DIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
OUT_DIR="${ROOT_DIR}/out"
RESOURCES_DIR="${ROOT_DIR}/resources/building-bao-hypervisor"
ZEPHYR_WORKSPACE="${ROOT_DIR}/../"

BM_DIR="${ROOT_DIR}/bao-baremetal-guest"
HV_DIR="${ROOT_DIR}/CROSSCON-Hypervisor"

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
    build         build HV and VMs: VM0 (bare-metal), VM1 (ZEPHYR_APP) - target more useful for testing
    build_puf     build HV and VMs: VM0 (Zephyr VM0), VM1 (Zephyr VM1) - default target for UC1.1 integration
    flash         flash HV and VMs
    hv_start      start HV via GDB (HV still cannot boot by itself?) 
    gdb_start     start GDB session with no extra commands
    no_hv_zephyr  build and flash ZEPHYR_APP as bare-metal (no hypervisor) - useful for debugging and comparing

  Environment variables:
    ZEPHYR_APP    name of the Zephyr app to be included in Zephyr VM:
                    - hello_world - simple hello world app,
                    - hello_at    - simple app sending AT command to modem and receiving response, useful to verify basic communication with modem,
                    - echo_bot    - echo over main console, based on echo_bot from samples, useful to verify UART RX & IRQ,
                    - timer_test  - test timer (related to: https://github.com/crosscon/CROSSCON-Hypervisor-and-TEE-Isolation-Demos/issues/22),
                    - wifi_app    - WiFi module support on USART2 (WiFi does not work yet)
    HV_CONFIG    hypervisor configuration to be used when using "build" (not "build_puf") command:
                   - single_bm - single VM running default bare-metal app
                   - single_zephyr - single VM running zephyr app selected by ZEPHYR_APP
                   - two_bm_zephyr - two VMs (VM0 - bare-metal, VM1 - Zephyr)

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
  [ -z "${ZEPHYR_APP}" ] && usage
  export DOCKER_IMAGE="ghcr.io/zephyrproject-rtos/zephyr-build:v0.27.5"
  if pushd "${ZEPHYR_WORKSPACE}" &> /dev/null; then
    case "${ZEPHYR_APP}" in
      "hello_world")
        docker_run west build -b "${ZEPHYR_TARGET}" --shield mikroe_wifi_bt_click_mikrobus ./uc1-integration/hello_world/ -p
        ;;
      "hello_at")
        docker_run west build -b "${ZEPHYR_TARGET}" --shield mikroe_wifi_bt_click_mikrobus ./uc1-integration/hello_at/ -p
        ;;
      "echo_bot")
        docker_run west build -b "${ZEPHYR_TARGET}" ./uc1-integration/echo_bot/ -p
        ;;
      "timer_test")
        docker_run west build -b "${ZEPHYR_TARGET}" ./uc1-integration/timer_test -p
        ;;
      "wifi_app")
        docker_run west build -b "${ZEPHYR_TARGET}" --shield mikroe_wifi_bt_click_mikrobus ./uc1-integration/wifi_app/ -p
        ;;
      *)
        echo "Unsupported ZEPHYR_APP: \"${ZEPHYR_APP}\""
        usage
        ;;
    esac
    cp "build/zephyr/zephyr.bin" "${OUT_DIR}/vm1.bin"
    cp "build/zephyr/zephyr.elf" "${OUT_DIR}/vm1.elf"
    export VM1_START=$(printf "0x%08x\n" $((0x$(nm build/zephyr/zephyr.elf | awk '/__start/ {print $1}') - 1)))
    export VM0_START=$(printf "0x%08x\n" $((0x$(nm build/zephyr/zephyr.elf | awk '/__start/ {print $1}') - 1)))
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

build_puf_vms() {
  export DOCKER_IMAGE="ghcr.io/zephyrproject-rtos/zephyr-build:v0.27.5"
  if pushd "${ZEPHYR_WORKSPACE}" &> /dev/null; then
    docker_run west build -b "${ZEPHYR_TARGET}" ./uc1-integration/puf_vm0/application -p
    cp "build/zephyr/zephyr.bin" "${OUT_DIR}/vm0.bin"
    cp "build/zephyr/zephyr.elf" "${OUT_DIR}/vm0.elf"
    export VM0_START=$(printf "0x%08x\n" $((0x$(nm build/zephyr/zephyr.elf | awk '/__start/ {print $1}') - 1)))
    echo "VM0_START extracted from zephyr.elf: ${VM0_START}"

    docker_run west build -b "${ZEPHYR_TARGET}" ./uc1-integration/puf_vm1/application -p
    cp "build/zephyr/zephyr.bin" "${OUT_DIR}/vm1.bin"
    cp "build/zephyr/zephyr.elf" "${OUT_DIR}/vm1.elf"
    export VM1_START=$(printf "0x%08x\n" $((0x$(nm build/zephyr/zephyr.elf | awk '/__start/ {print $1}') - 1)))
    echo "VM1_START extracted from zephyr.elf: ${VM1_START}"

    popd &> /dev/null
  fi
  export -n DOCKER_IMAGE
}

build_hv() {
  [ -z "${HV_CONFIG}" ] && usage
  [ -z "${VM1_START}" ] && errorExit "VM1_START not set, build Zephyr first to retrieve it"

  export DOCKER_IMAGE="bao-hypervisor-image"
  if pushd "${HV_DIR}" &> /dev/null; then
    case "${HV_CONFIG}" in
      "single_bm")
        cp "${RESOURCES_DIR}/config_single_vm_bm.c" "${HV_DIR}/configs/lpc55.c"
        ;;
      "single_zephyr")
        cp "${RESOURCES_DIR}/config_single_vm_zephyr.c" "${HV_DIR}/configs/lpc55.c"
        ;;
      "two_bm_zephyr")
        cp "${RESOURCES_DIR}/config.c" "${HV_DIR}/configs/lpc55.c"
        ;;
      *)
        cp "${RESOURCES_DIR}/${HV_CONFIG}.c" "${HV_DIR}/configs/lpc55.c"
        ;;
    esac
    # If VM0 is also Zephyr, replace entrypoint as well
    sed -i "${HV_DIR}/configs/lpc55.c" -e "s/@@ZEPHYR_VM0_ENTRY@@/${VM0_START}/"
    sed -i "${HV_DIR}/configs/lpc55.c" -e "s/@@ZEPHYR_VM1_ENTRY@@/${VM1_START}/"
    docker_run make clean
    docker_run make PLATFORM=lpc55s69 CONFIG=lpc55 DEBUG=y
    cp "${HV_DIR}/bin/lpc55s69/lpc55/crossconhyp.bin" "${OUT_DIR}"
    cp "${HV_DIR}/bin/lpc55s69/lpc55/crossconhyp.elf" "${OUT_DIR}"
    popd &> /dev/null
  fi
  export -n DOCKER_IMAGE
  export -n VM1_START
  export -n VM0_START
}

flash() {
  export DOCKER_IMAGE="bao-hypervisor-image"
  docker_run LinkServer flash LPC55S69:LPCXpresso55S69 load ${OUT_DIR}/crossconhyp.elf
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

no_hv_flash() {
  local _zephyr_bin="${OUT_DIR}/vm1.bin"

  which lpc55 &> /dev/null
  errorCheck "Make sure to install https://github.com/lpc55/lpc55-host into PATH"

  echo "Waiting for device to enter bootloader mode..."
  echo "Press and hold ISP, then press RESET on your LPC55 board."
  echo "Make sure that udev rules are installed if it does not work!"
  echo "https://spsdk.readthedocs.io/en/latest/examples/_knowledge_base/installation_guide.html#usb-under-linux"

  while true; do
      OUTPUT=$(lpc55 ls)
      if echo "$OUTPUT" | grep -Eq 'Bootloader\s+\{.*vid:.*pid:.*uuid:.*\}'; then
          echo "Bootloader detected:"
          echo "$OUTPUT" | grep -E 'Bootloader\s+\{.*vid:.*pid:.*uuid:.*\}'
          break
      fi
      sleep 1
  done

  echo "Flashing  ${_zephyr_bin} ..."
  lpc55 write-flash "${_zephyr_bin}"
  errorCheck "Flashing failed"

  echo "Press RESET button to start the firmware"
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
    export ZEPHYR_TARGET="lpcxpresso55s69/lpc55s69/cpu0/ns"
    rm -rf "${OUT_DIR}"
    mkdir -p "${OUT_DIR}"
    build_zephyr
    build_bm
    build_hv
    ;;
  "build_puf")
    export ZEPHYR_TARGET="lpcxpresso55s69/lpc55s69/cpu0/ns"
    export HV_CONFIG="puf_integration_without_wifi"
    rm -rf "${OUT_DIR}"
    mkdir -p "${OUT_DIR}"
    build_puf_vms
    build_hv
    ;;
  "no_hv_zephyr")
    export ZEPHYR_TARGET="lpcxpresso55s69/lpc55s69/cpu0"
    rm -rf "${OUT_DIR}"
    mkdir -p "${OUT_DIR}"
    build_zephyr
    no_hv_flash
    ;;
  *)
    echo "Invalid command: \"${CMD}\""
    usage
    ;;
esac
