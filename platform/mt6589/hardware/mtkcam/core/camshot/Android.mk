#
# libcam.camshot
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#-----------------------------------------------------------
sinclude $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES := \
    CamShotImp.cpp \
    $(call all-c-cpp-files-under, SingleShot) \
    $(call all-c-cpp-files-under, MultiShot) \
    $(call all-c-cpp-files-under, SampleSingleShot) \
    $(call all-c-cpp-files-under, BurstShot) \

#    $(call all-c-cpp-files-under, SImager) \
#    $(call all-c-cpp-files-under, SImager/ImageTransform) \
#    $(call all-c-cpp-files-under, SImager/JpegCodec) \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
#
LOCAL_C_INCLUDES += $(TOP)/bionic
LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
# 
# camera Hardware 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/hal/adapter/inc

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc
LOCAL_C_INCLUDES += $(TOP)/external/jpeg

# jpeg encoder 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/jpeg/inc \
# m4u 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/m4u \

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libstlport \
    libcam.campipe \
    libcamdrv \
    libutils \
    libdpframework \
    libjpeg \

LOCAL_SHARED_LIBRARIES += libcam_mmp
	  
# for jpeg enc use 
LOCAL_SHARED_LIBRARIES += \
    libm4u \
    libJpgEncPipe \

# for 3A 
LOCAL_SHARED_LIBRARIES +=\
    libfeatureio \

# camUtils 
LOCAL_SHARED_LIBRARIES +=\
    libcam.utils \
    

LOCAL_STATIC_LIBRARIES := \
 
#
LOCAL_WHOLE_STATIC_LIBRARIES := \
     libcam.camshot.simager \
     libcam.camshot.utils \
     
#
LOCAL_MODULE := libcam.camshot

#
LOCAL_MODULE_TAGS := optional

#
include $(BUILD_SHARED_LIBRARY)

#
include $(call all-makefiles-under,$(LOCAL_PATH))
