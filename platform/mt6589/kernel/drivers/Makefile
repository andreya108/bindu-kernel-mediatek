include $(MTK_PATH_BUILD)/common.mk
#Fix-me: modified for early porting
#$(call all-subdir-src-or-makefile)
obj-y   += uart/
ifneq	($(MTK_GPU_SUPPORT),no)
obj-y   += gpu/
endif
obj-y   += pmic_wrap/
obj-y   += pmt/
obj-y   += gpio/
obj-y   += pwm/
obj-y   += power/
obj-y   += m4u/
obj-y   += smi/
obj-y   += usb20/
obj-y   += hdmitx/
obj-y   += usb11/
obj-y   += mmc-host/
obj-y   += sound/
ifneq ($(MTK_EMMC_SUPPORT),yes)
obj-y   += nand/
endif
ifneq ($(MTK_PROJECT), mt6589fpga_ca7_ldvt)
obj-y   += keypad/
obj-y   += auxadc/
obj-y   += videox/
obj-y   += video/
endif
ifneq ($(CONFIG_EARLY_LINUX_PORTING),y)
obj-y   += touchpanel/
obj-y   += spi/
obj-y   += ldvt/
obj-y   += mmc-host/
obj-y   += videocodec/
obj-y   += rtc/

obj-y   += jpeg/
obj-y   += gdma/
endif

obj-y   += i2c/
obj-y   += dispsys/
obj-y   += dual_ccci/
#lenovo-sw, chenlj2, 2013-6-21, add for hall
ifeq ($(LENOVO_HALL_SENSOR_SUPPORT),yes)
obj-y   += hall/
endif
obj-y   += accdet/
obj-y   += leds/
obj-y   += vibrator/
obj-y   += wdt/
obj-y	+= devinfo/
obj-y	+= masp/
obj-y	+= devapc/
obj-$(CONFIG_THERMAL) += thermal/
include $(MTK_PATH_BUILD)/common.mk
obj-y   += $(call all-subdirs-with-makefile,cpu_stress)

obj-y   += met/
