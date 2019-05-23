#!/bin/sh

DIR=$(dirname "$(readlink -f "$0")")

$DIR/setPxclk102_1.sh
$DIR/setVideo1280x960p.sh
