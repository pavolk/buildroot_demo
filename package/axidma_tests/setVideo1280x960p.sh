#!/bin/sh

# vtc related parameters
# ==

ActiveHSize=1280
FrameHSize=1712
SyncHStart=1360
SyncHEnd=1496

ActiveVSize=960
FrameVSize=994
SyncVStart=961
SyncVEnd=964

vtc_offset=$(( 0x43c50000 )) # vtc offset as specified in Vivado address editor

# VDMA related parameters
# ==

vdma_offset=$(( 0x43020000 )) # VDMA offset as specified in Vivado address editor

channels=3 # 3 for RGB 24 bit, 1 for mono 8 bit

framepointer1=$(( 0x3d000000 )) # frame buffer 1 start address
framepointer2=$(( 0x3e000000 )) # frame buffer 2 start address
framepointer3=$(( 0x3f000000 )) # frame buffer 3 start address

# register configuration
# ==

echo "stop vtc operation"
echo "=="
devmem $(( $vtc_offset + 0x00 )) 32 0x0

echo "vtc init"
echo "=="

echo "setting ACTIVE_VSIZE and ACTIVE_HSIZE"
devmem $(( $vtc_offset + 0x60 )) 16 $ActiveHSize
devmem $(( $vtc_offset + 0x62 )) 16 $ActiveVSize

echo "setting FRAME_HSIZE"
devmem $(( $vtc_offset + 0x70 )) 32 $FrameHSize

echo "setting FRAME_VSIZE"
devmem $(( $vtc_offset + 0x74 )) 32 $FrameVSize

echo "setting HSYNC_END and HSYNC_START"
devmem $(( $vtc_offset + 0x78 )) 16 $SyncHStart
devmem $(( $vtc_offset + 0x7a )) 16 $SyncHEnd

echo "setting VBLANK_HEND and VBLANK_HSTART"
devmem $(( $vtc_offset + 0x7c )) 16 $ActiveHSize
devmem $(( $vtc_offset + 0x7e )) 16 $ActiveHSize

echo "setting VSYNC_VEND and VSYNC_VSTART"
devmem $(( $vtc_offset + 0x80 )) 16 $SyncVStart
devmem $(( $vtc_offset + 0x82 )) 16 $SyncVEnd

echo "setting VSYNC_HEND and VSYNC_HSTART"
devmem $(( $vtc_offset + 0x84 )) 16 $ActiveHSize
devmem $(( $vtc_offset + 0x86 )) 16 $ActiveHSize

echo "vtc init done."
echo "=="

echo "start vtc operation"
echo "=="
devmem $(( $vtc_offset + 0x00 )) 32 0x7
