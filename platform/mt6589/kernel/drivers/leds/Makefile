obj-$(CONFIG_MTK_LEDS) := leds.o
ifeq ($(LENOVO_LP5560_SUPPORT),yes)
obj-y += lp5560_leds.o
endif
ifeq ($(LENOVO_LEDS_GPIO_SUPPORT),yes)
obj-y += leds_gpio.o
endif
ifeq ($(LENOVO_SN3193_LED_SUPPORT),yes)
obj-y += sn3193_leds.o
endif
ifeq ($(LENOVO_SN3193_LED_SUPPORT_NEW),yes)
obj-y += sn3193_leds_new.o
endif
ifeq ($(LENOVO_SHINELED_SUPPORT),yes)
obj-y += shineled_driver.o
endif
