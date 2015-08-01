# drivers/video/mtk/Makefile
#
# Makefile for MT6516 video driver.
#

obj-$(CONFIG_MTK_FB) += disp_drv_dbi.o              \
         disp_drv_dpi.o                             \
		 disp_drv_dsi.o								\
         disp_drv.o                                 \
         mtkfb_console.o                            \
         disp_assert_layer.o                        \
         mtkfb.o                                    \
		 mtkfb_vsync.o								\
         debug.o                                    \
         tv_out.o                                   \
         mtkfb_proc.o                               \
         disp_sync.o

ccflags-y += -I$(MTK_PATH_SOURCE)/drivers/video/        \
             -I$(MTK_PATH_PLATFORM)/drivers/video/      \
             -I$(MTK_PATH_PLATFORM)/drivers/videox/     \
             -I$(MTK_PATH_PLATFORM)/drivers/dispsys/    \
             -I$(MTK_PATH_SOURCE)/drivers/sync/

ifeq ($(MTK_DITHERING_SUPPORT), yes)
ccflags-y += -DDITHERING_SUPPORT
endif

ifeq ($(EVB), yes)
ccflags-y += -DEVB
endif

ifeq ($(MTK_S3D_SUPPORT), yes)
ccflags-y += -DMTK_LCD_HW_3D_SUPPORT
endif

ifeq ($(MTK_GPU_SUPPORT), yes)
ccflags-y += -DHWGPU_SUPPORT
endif

ifeq ($(CONFIG_MMPROFILE),y)
ccflags-y += -DDEFAULT_MMP_ENABLE
endif
