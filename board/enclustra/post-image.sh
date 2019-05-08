#!/bin/sh

BOARD_DIR="$(dirname $0)"

MKIMAGE=${HOST_DIR}/bin/mkimage
DTC=${HOST_DIR}/bin/dtc
MKBOOTIMAGE=${BUILD_DIR}/../../utils/mkbootimage

cp ${BOARD_DIR}/pre-built/system_top.bit ${BINARIES_DIR}/fpga.bit
cp ${BOARD_DIR}/pre-built/fsbl.elf ${BINARIES_DIR}
cp ${BINARIES_DIR}/u-boot ${BINARIES_DIR}/u-boot.elf

echo "Creating boot.bin..."
cp ${BOARD_DIR}/boot.bif ${BINARIES_DIR}
(cd ${BINARIES_DIR}; ${MKBOOTIMAGE} boot.bif boot.bin)

echo "Compiling the boot-script..."
${MKIMAGE} -A arm -O linux -T script -C none \
	 	-d ${BOARD_DIR}/uboot/mmcboot-ramdisk \
	 	${BINARIES_DIR}/uboot.scr

echo "Compiling the device-tree..."
${DTC} -I dts -O dtb -o ${BINARIES_DIR}/devicetree.dtb ${BOARD_DIR}/devicetree.dts

echo "Creating the sd-card image..."
support/scripts/genimage.sh -c ${BOARD_DIR}/genimage.cfg
