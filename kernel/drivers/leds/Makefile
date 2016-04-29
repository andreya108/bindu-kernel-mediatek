include $(MTK_ROOT_BUILD)/Makefile
LEDS_PLATFORM_DRIVER_DIR :=  $(srctree)/mediatek/platform/$(call lc,$(MTK_PLATFORM))/kernel/drivers/leds
ifeq ($(USE_VANZO_LEDS),)
	obj-$(CONFIG_MTK_LEDS) += leds_drv.o
	ccflags-y  += -I$(LEDS_PLATFORM_DRIVER_DIR)
else
	obj-y += dummy.o
endif
