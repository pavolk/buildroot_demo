#!/bin/sh

BOARD_DIR="$(dirname $0)"

MKIMAGE=${HOST_DIR}/bin/mkimage
DTC=${HOST_DIR}/bin/dtc
MKBOOTIMAGE=${BUILD_DIR}/../../utils/mkbootimage

cp ${BOARD_DIR}/pre-built/linux/implementation/download.bit ${BINARIES_DIR}/fpga.bit
cp ${BOARD_DIR}/pre-built/linux/images/zynq_fsbl.elf ${BINARIES_DIR}/fsbl.elf
cp ${BOARD_DIR}/pre-built/linux/images/u-boot.elf ${BINARIES_DIR}/u-boot.elf
cp ${BOARD_DIR}/pre-built/linux/images/system.dtb ${BINARIES_DIR}/devicetree.dtb 

echo "Creating boot.bin..."
cp ${BOARD_DIR}/boot.bif ${BINARIES_DIR}
(cd ${BINARIES_DIR}; ${MKBOOTIMAGE} boot.bif boot.bin)

echo "Creating FIT-image..."
cp ${BOARD_DIR}/image.its ${BINARIES_DIR}
(cd ${BINARIES_DIR}; ${MKIMAGE} -f image.its image.ub)

echo "Creating the sd-card image..."
support/scripts/genimage.sh -c ${BOARD_DIR}/genimage.cfg
