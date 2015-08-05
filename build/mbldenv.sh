#!/bin/bash
# ##########################################################
# ALPS(Android4.1 based) build environment profile setting
# ##########################################################

#TOOLCHAIN=linaro-4.9.4
#TOOLCHAIN=linaro
#TOOLCHAIN=gcc
TOOLCHAIN=uber

TARGET_BUILD_VARIANT=user
#TARGET_BUILD_VARIANT=eng
export TARGET_BUILD_VARIANT

# Overwrite JAVA_HOME environment variable setting if already exists
JAVA_HOME=/usr/lib/jvm/java-6-oracle
export JAVA_HOME

# Overwrite ANDROID_JAVA_HOME environment variable setting if already exists
ANDROID_JAVA_HOME=$JAVA_HOME
export ANDROID_JAVA_HOME
#export KBUILD_BUILD_TIMESTAMP="$KV `date +'%F %R'`"
export KBUILD_BUILD_USER=bindu-kernel
export KBUILD_BUILD_HOST=`./mkversion host`

export EXTRAVERSION=`./mkversion`

echo "Building kernel version: $KBUILD_BUILD_HOST"

# Overwrite PATH environment setting for JDK & arm-eabi if already exists

case "$TOOLCHAIN" in

    gcc)
	echo "Building with GCC 4.7"
	PATH=$PWD/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin:$PWD/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin:$PWD/prebuilts/misc/linux-x86/make:$PATH
	;;
    uber)
	echo "Building with UBER 4.9 arm-eabi"
	PATH=$PWD/prebuilts/UBERTC-arm-eabi-4.9-addfce38ffe9/bin:$PWD/prebuilts/misc/linux-x86/make:$PATH
	export CROSS_COMPILE=arm-eabi-
	;;
    linaro)
	echo "Building with LINARO 4.9.3"
	PATH=$PWD/prebuilts/android-toolchain-eabi/bin:$PWD/prebuilts/misc/linux-x86/make:$PATH
	export CROSS_COMPILE=arm-eabi-
	;;
    linaro-4.9.4)
	echo "Building with LINARO 4.9.4"
	PATH=$PWD/prebuilts/arm-cortex_a7-linux-gnueabihf-linaro_4.9.4-2015.06/bin:$PWD/prebuilts/misc/linux-x86/make:$PATH
	export CROSS_COMPILE=arm-cortex_a7-linux-gnueabihf-
	;;
    *)
	echo "Unknown toolchain $TOOLCHAIN selected"
	;;
esac

PATH=$JAVA_HOME/bin:$PATH
#PATH=$JAVA_HOME/bin:~/AndroidKernel/ea89/prebuilts/gcc/linux-x86/arm/android-toolchain-eabi-4.8/bin:$PWD/prebuilts/misc/linux-x86/make:$PATH
export PATH

#export CROSS_COMPILE=$PWD/prebuilts/gcc/linux-x86/arm/android-linux-androideabi-4.7/prebuilt/linux-x86_64/bin/arm-linux-androideabi-
#export CROSS_COMPILE=~/AndroidKernel/ea89/prebuilts/gcc/linux-x86/arm/android-toolchain-eabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-


# Add MediaTek developed Python libraries path into PYTHONPATH
if [ -z "$PYTHONPATH" ]; then
  PYTHONPATH=$PWD/mediatek/build/tools
else
  PYTHONPATH=$PWD/mediatek/build/tools:$PYTHONPATH
fi
export PYTHONPATH

