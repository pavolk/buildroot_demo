#!/bin/sh

BOARD_DIR="$(dirname $0)"
TARGET_DIR=$1
# Generate a file identifying the build (git commit and build date)
echo $(git describe) $(date +%Y-%m-%d-%H:%M:%S) > $TARGET_DIR/etc/build-id

# Create /boot mountpoint, and adjust /etc/fstab
mkdir -p $TARGET_DIR/boot
echo "/dev/mmcblk0p1\t\t/boot\tvfat\tdefaults\t\t0\t0" >> $TARGET_DIR/etc/fstab
