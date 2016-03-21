#!/bin/bash
# ##########################################################
# ALPS(Android4.1 based) build environment profile setting
# ##########################################################

TARGET_BUILD_VARIANT=user
#TARGET_BUILD_VARIANT=eng
export TARGET_BUILD_VARIANT

# Overwrite JAVA_HOME environment variable setting if already exists
JAVA_HOME=/usr/lib/jvm/java-6-oracle
export JAVA_HOME

# Overwrite ANDROID_JAVA_HOME environment variable setting if already exists
ANDROID_JAVA_HOME=$JAVA_HOME
export ANDROID_JAVA_HOME

#export KBUILD_BUILD_USER=bindu-kernel
#export KBUILD_BUILD_HOST=`./mkversion host`
export LOCAL_NO_LENOVO_RES=true
export TARGET_ARCH_VARIANT=armv7-a-neon

#export EXTRAVERSION=`./mkversion`

#echo "Building kernel version: $KBUILD_BUILD_HOST"

export TOOLCHAIN_PREFIX=$CROSS_COMPILE

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

