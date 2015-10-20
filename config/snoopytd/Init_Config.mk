#Android forbiden developer use product level variables(PRODUCT_XXX, TARGET_XXX, BOARD_XXX) in Android.mk
#Because AndroidBoard.mk include by build/target/board/Android.mk
#split from AndroidBoard.mk for PRODUCT level variables definition.
#use MTK_ROOT_CONFIG_OUT instead of MTK_ROOT_CONFIG_OUT


PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk:system/data/misc/ProjectConfig.mk

TARGET_PROVIDES_INIT_RC := true

PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/mtk-kpd.kl:system/usr/keylayout/mtk-kpd.kl \
                      $(MTK_ROOT_CONFIG_OUT)/init.rc:root/init.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.modem.rc:root/init.modem.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.usb.rc:root/init.usb.rc \
                      $(MTK_ROOT_CONFIG_OUT)/init.xlog.rc:root/init.xlog.rc \
                      $(MTK_ROOT_CONFIG_OUT)/player.cfg:system/etc/player.cfg \
                      $(MTK_ROOT_CONFIG_OUT)/media_codecs.xml:system/etc/media_codecs.xml \
                      $(MTK_ROOT_CONFIG_OUT)/mtk_omx_core.cfg:system/etc/mtk_omx_core.cfg \
                      $(MTK_ROOT_CONFIG_OUT)/meta_init.rc:root/meta_init.rc \
                      $(MTK_ROOT_CONFIG_OUT)/meta_init.modem.rc:root/meta_init.modem.rc \
                      $(MTK_ROOT_CONFIG_OUT)/factory_init.rc:root/factory_init.rc \
                      $(MTK_ROOT_CONFIG_OUT)/audio_policy.conf:system/etc/audio_policy.conf \
                      $(MTK_ROOT_CONFIG_OUT)/init.protect.rc:root/init.protect.rc \
                      $(MTK_ROOT_CONFIG_OUT)/ACCDET.kl:system/usr/keylayout/ACCDET.kl \
                      $(MTK_ROOT_CONFIG_OUT)/fstab:root/fstab \
                      $(MTK_ROOT_CONFIG_OUT)/fstab:root/fstab.nand \
                      $(MTK_ROOT_CONFIG_OUT)/fstab:root/fstab.fat.nand \
		      $(MTK_ROOT_CONFIG_OUT)/enableswap.sh:root/enableswap.sh \


ifeq ($(MTK_SMARTBOOK_SUPPORT),yes)
PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/sbk-kpd.kl:system/usr/keylayout/sbk-kpd.kl \
                      $(MTK_ROOT_CONFIG_OUT)/sbk-kpd.kcm:system/usr/keychars/sbk-kpd.kcm
endif

#Lenovo-sw wengjun1 add,20131117,begin
PRODUCT_COPY_FILES += $(LOCAL_PATH)/mtk-tpd.kl:system/usr/keylayout/mtk-tpd.kl
#Lenovo-sw wengjun1 add,20131117,end

#lenovo-sw jixj 2013.6.7 add begin
PRODUCT_COPY_FILES += $(LOCAL_PATH)/init.res.sh:system/etc/init.res.sh
#lenovo-sw jixj 2013.6.7 add end

#lenovo-sw jixj 2013.5.27 add begin
ifeq ($(LENOVO_RECOVERY_ONLINE),yes)
PRODUCT_COPY_FILES += $(LOCAL_PATH)/init2.rc:root/init2.rc
PRODUCT_COPY_FILES += $(LOCAL_PATH)/meta2_init.rc:root/meta2_init.rc
PRODUCT_COPY_FILES += $(LOCAL_PATH)/factory2_init.rc:root/factory2_init.rc
PRODUCT_COPY_FILES += $(LOCAL_PATH)/init2.charging.rc:root/init2.charging.rc
PRODUCT_COPY_FILES += $(LOCAL_PATH)/misc2.img:misc2.img
PRODUCT_COPY_FILES += $(LOCAL_PATH)/format.sh:system/etc/format.sh
endif
#lenovo-sw jixj 2013.5.27 add end

ifeq ($(MTK_KERNEL_POWER_OFF_CHARGING),yes)
PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.charging.rc:root/init.charging.rc 
endif

_init_project_rc := $(MTK_ROOT_CONFIG_OUT)/init.project.rc
ifneq ($(wildcard $(_init_project_rc)),)
PRODUCT_COPY_FILES += $(_init_project_rc):root/init.project.rc
endif

_meta_init_project_rc := $(MTK_ROOT_CONFIG_OUT)/meta_init.project.rc
ifneq ($(wildcard $(_meta_init_project_rc)),)
PRODUCT_COPY_FILES += $(_meta_init_project_rc):root/meta_init.project.rc
endif


_factory_init_project_rc := $(MTK_ROOT_CONFIG_OUT)/factory_init.project.rc
ifneq ($(wildcard $(_factory_init_project_rc)),)
PRODUCT_COPY_FILES += $(_factory_init_project_rc):root/factory_init.project.rc
endif

PRODUCT_COPY_FILES += $(strip \
                        $(foreach file,$(wildcard $(MTK_ROOT_CONFIG_OUT)/*.xml), \
                          $(addprefix $(MTK_ROOT_CONFIG_OUT)/$(notdir $(file)):system/etc/permissions/,$(notdir $(file))) \
                         ) \
                       )


ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/srs_processing.cfg:system/data/srs_processing.cfg
endif

ifeq ($(MTK_SHARED_SDCARD),yes)
ifeq ($(MTK_2SDCARD_SWAP),yes)
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.ssd_nomuser.rc:root/init.ssd_nomuser.rc
else
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.ssd.rc:root/init.ssd.rc
endif
else
  PRODUCT_COPY_FILES += $(MTK_ROOT_CONFIG_OUT)/init.no_ssd.rc:root/init.no_ssd.rc
endif

$(info MTK_MODEM_INSTALLED_MODULES = $(MTK_MODEM_INSTALLED_MODULES))


##### INSTALL ht120.mtc ##########

_ht120_mtc := $(MTK_ROOT_CONFIG_OUT)/configs/ht120.mtc
ifneq ($(wildcard $(_ht120_mtc)),)
PRODUCT_COPY_FILES += $(_ht120_mtc):system/etc/.tp/.ht120.mtc
endif

##################################

##### INSTALL thermal.conf ##########

_thermal_conf := $(MTK_ROOT_CONFIG_OUT)/configs/thermal.conf
ifneq ($(wildcard $(_thermal_conf)),)
PRODUCT_COPY_FILES += $(_thermal_conf):system/etc/.tp/thermal.conf
endif

##################################

##### INSTALL thermal.off.conf ##########

_thermal_off_conf := $(MTK_ROOT_CONFIG_OUT)/configs/thermal.off.conf
ifneq ($(wildcard $(_thermal_off_conf)),)
PRODUCT_COPY_FILES += $(_thermal_off_conf):system/etc/.tp/thermal.off.conf
endif

##################################

##### INSTALL throttle.sh ##########

_throttle_sh := $(MTK_ROOT_CONFIG_OUT)/configs/throttle.sh
ifneq ($(wildcard $(_throttle_sh)),)
PRODUCT_COPY_FILES += $(_throttle_sh):system/etc/throttle.sh
endif

##################################

#lenovo-sw zhouwl, 2014-01-26, add for nxp-tfa9887
######NXP setting files#########

ifeq ($(LENOVO_NXP_SMARTPA_SUPPORT),yes)
PRODUCT_COPY_FILES += $(LOCAL_PATH)/nxp/coldboot.patch:system/data/nxp/coldboot.patch \
	$(LOCAL_PATH)/nxp/TFA9887_N1D2_4_1_1.patch:system/data/nxp/TFA9887_N1D2_4_1_1.patch \
	$(LOCAL_PATH)/nxp/Setup_0618.config:system/data/nxp/Setup_0618.config \
	$(LOCAL_PATH)/nxp/AAC_0618.speaker:system/data/nxp/AAC_0618.speaker \
	$(LOCAL_PATH)/nxp/MUSIC_LOUD_0916.eq:system/data/nxp/MUSIC_LOUD_0916.eq \
	$(LOCAL_PATH)/nxp/MUSIC_LOUD_0916.preset:system/data/nxp/MUSIC_LOUD_0916.preset \
	$(LOCAL_PATH)/nxp/SPEECH_LOUD_0916.eq:system/data/nxp/SPEECH_LOUD_0916.eq \
	$(LOCAL_PATH)/nxp/SPEECH_LOUD_0916.preset:system/data/nxp/SPEECH_LOUD_0916.preset \
	$(LOCAL_PATH)/nxp/SPEECH_LOUD2_0917.eq:system/data/nxp/SPEECH_LOUD2_0917.eq \
	$(LOCAL_PATH)/nxp/SPEECH_LOUD2_0917.preset:system/data/nxp/SPEECH_LOUD2_0917.preset \
	$(LOCAL_PATH)/nxp/climax:system/bin/climax
endif	
##################################
#lenovo-sw zhouwl, 2014-01-26, add for nxp-tfa9887
