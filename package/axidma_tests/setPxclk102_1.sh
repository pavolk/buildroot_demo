#!/bin/sh

# clk_wiz related parameters
# ==

clkfboutMultInt=30
clkfboutMultFrac=625
DivClkDivide=3

ClkOut0Divide=10
ClkOut1Divide=2

clk_wiz_offset=$(( 0x43c40000 )) # clk wizard offset as specified in Vivado address editor
 
# register configuration
# ==

echo "clk_wiz init"
echo "=="

echo "clk_wiz software reset"
devmem $(( $clk_wiz_offset + 0x0 )) 32 0xa
sleep .1

echo "setting DIVCLK_DIVIDE, CLKFBOUT_MULT and CLKFBOUT_FRAC"
devmem $(( $clk_wiz_offset + 0x200 )) 8 $DivClkDivide
devmem $(( $clk_wiz_offset + 0x201 )) 8 $clkfboutMultInt
devmem $(( $clk_wiz_offset + 0x202 )) 16 $clkfboutMultFrac
devmem $(( $clk_wiz_offset + 0x200 ))

echo "setting CLKOUT0_DIVIDE"
devmem $(( $clk_wiz_offset + 0x208 )) 32 $ClkOut0Divide

echo "setting CLKOUT1_DIVIDE"
devmem $(( $clk_wiz_offset + 0x214 )) 32 $ClkOut1Divide

if [ $(( $( devmem $(( $clk_wiz_offset + 0x4 )) ) )) -ne 1 ] 
	then
		echo "status register should be 0x1 before starting reconfiguration but is actually"
		echo $( devmem $(( $clk_wiz_offset + 0x4 )) )
fi	

echo "start clk_wiz reconfiguration"
devmem $(( $clk_wiz_offset + 0x25c )) 32 0x3
