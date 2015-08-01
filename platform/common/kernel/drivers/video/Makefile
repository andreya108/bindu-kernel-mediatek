# drivers/video/mtk/Makefile
#
#

obj-$(CONFIG_MTK_FB) +=                             \
         mtkfb_console.o                            \
         disp_assert_layer.o                        \
		 mtkfb_vsync.o								\
         debug.o

#FIXME: NEED OR NOT to consider TEE or IN_HOUSE_TEE?
#ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)),yes)
#obj-$(CONFIG_MTK_FB) += 							\
#		disp_drv_ext.o 								\
#		mtkfb_ext.o 								\
#		disp_sync_ext.o 							\
#		disp_mgr.o
#else
obj-$(CONFIG_MTK_FB) += 							\
		disp_drv.o 									\
		mtkfb.o 									\
		disp_sync.o
#endif


#ifeq ($(MTK_DISP_CONFIG_SUPPORT), yes)
obj-$(CONFIG_MTK_FB) += fbconfig_kdebug.o
#endif

ccflags-y += -I$(MTK_PATH_SOURCE)/drivers/video/  \
			 -I$(MTK_PATH_PLATFORM)/drivers/video/  \
			 -I$(MTK_PATH_PLATFORM)/drivers/dispsys/ \
			 -I$(MTK_PATH_PLATFORM)/core/include/mach/ \
			 -I$(MTK_PATH_SOURCE)/drivers/sync/

ifeq ($(MTK_INTERNAL_HDMI_SUPPORT), yes)
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/hdmi/
endif  
ifeq ($(MTK_INTERNAL_MHL_SUPPORT), yes)
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/mhltx/
endif   
ifeq ($(MTK_DITHERING_SUPPORT), yes)
ccflags-y += -DDITHERING_SUPPORT
endif

ifeq ($(MTK_GPU_SUPPORT), yes)
ccflags-y += -DHWGPU_SUPPORT
endif

ifeq ($(CONFIG_MMPROFILE),y)
ccflags-y += -DDEFAULT_MMP_ENABLE
endif


