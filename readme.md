## Overview

This document describes how to use this repository to build a minimal Linux system for a Zynq-board (demonstrated with Avnet MicroZed board) with buildroot.

## Preparation

1. Clone this repository with the buildroot as submodule

```
git clone https://github.com/pavolk/buildroot_demo.git
cd buildroot_demo
git submodule update --init
```

## Initial Build

```
cd buildroot_demo
mkdir build
cd build
make -C ../buildroot O=$(pwd) BR2_EXTERNAL=.. demo_zynq_microzed_defconfig
make
```

## Create SD-card

```
cd buildroot_demo/build
umount /dev/mmcblk0p*
sudo dd if=images/sdcard.img of=/dev/mmcblk0
```

**NOTE:** The output device name (of=<device-name>) may vary e.g. /dev/sdc


## Setup Eclipse/Buildroot development environment

This section describes the setup of the eclipse-based environment to cross-develop and run/debug applications remotely on the target-system using the buildroot-eclipse support/plug-in.

The plug-in is supported by the older version of eclipse (luna), which only runs with java-runtime not newer than java8. An issue with the buildroot's package-repository prevents the regular plug-in installation, so we'll describe, how to overcome this too.

1. Download eclipse-luna

```
cd buidroot_demo
mkdir eclipse
cd eclipse
wget https://www.eclipse.org/downloads/download.php?file=/technology/epp/downloads/release/luna/R/eclipse-cpp-luna-R-linux-gtk-x86_64.tar.gz -O eclipse-luna.tar.gz
tar xzvf eclipse-luna.tar.gz
```

2. If necessary install "openjdk-8" to support eclipse-luna

For a debian-based system you can run this...
```
sudo apt install openjdk-8-jre
```

**NOTE:** If you have multiple java environments installed use ```update-java-alternatives``` or [jEnv](http://www.jenv.be/) to select the correct one.

3. Download the Buildroot-Plugin

Using the buildroot package-repository directly doesn't seem to work at this moment. To workaround this, download the package and install it from the local package-directory as suggested [here](https://github.com/mbats/eclipse-buildroot-bundle/issues/20).

```
cd buildroot_demo/eclipse
wget -r --no-parent  http://buildroot.org/downloads/eclipse/luna/
```

4. Install Buildroot-Plugin

Start eclipse and select the package-directory for the installation of the plugin:

*"Help" -> "Install new software" -> "Add" -> "Local"*.

Restart eclipse.

Useful documentation on how to use the Eclipse/Buildroot development environment you'll find [here](https://github.com/mbats/eclipse-buildroot-bundle/wiki).
