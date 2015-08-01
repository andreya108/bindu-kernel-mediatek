#include "SpeechANCController.h"
#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include "AudioType.h"
#include "AudioIoctl.h"

#include "AudioALSAHardware.h"

#define LOG_TAG "SpeechANCController"
//#define param_anc_add
namespace android
{

AudioALSAHardware *mAudioHardware;

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechANCController *SpeechANCController::UniqueSpeechANCController = NULL;


SpeechANCController *SpeechANCController::getInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);
    ALOGD("%s()", __FUNCTION__);

    if (UniqueSpeechANCController == NULL)
    {
        UniqueSpeechANCController = new SpeechANCController();
    }
    ASSERT(UniqueSpeechANCController != NULL);
    return UniqueSpeechANCController;
}
/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechANCController::SpeechANCController()
{
    ALOGD("%s()", __FUNCTION__);
#ifdef param_anc_add  
    AUDIO_CUSTOM_ANC_PARAM_STRUCT pSphParamAnc;
#endif
    mEnabled       = false;
    mGroupANC      = false;
    //   mFd            = ::open(kAudioDeviceName, O_RDWR);
    //   ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Init);
#ifdef param_anc_add  
    GetANCSpeechParamFromNVRam(&pSphParamAnc);
    mLogEnable     = pSphParamAnc.ANC_log;
    mLogDownSample = pSphParamAnc.ANC_log_downsample;
    mApply         = pSphParamAnc.ANC_apply;

    SetCoefficients(pSphParamAnc.ANC_para);
#endif
}

SpeechANCController::~SpeechANCController()
{
    ALOGD("%s()", __FUNCTION__);
    if (mFd)
    {
        ::close(mFd);
        mFd = 0;
    }
}

/*==============================================================================
 *                     AudioANCControl Imeplementation
 *============================================================================*/
void SpeechANCController::SetCoefficients(void *buf)
{
    ALOGD("%s(), SetCoefficients:%d", __FUNCTION__);
    //   ::ioctl(mFd, SET_ANC_PARAMETER, buf);
}

void SpeechANCController::SetApplyANC(bool apply)
{
    ALOGD("%s(), SetApply:%d", __FUNCTION__, apply);

    if (apply ^ mApply)
    {
#ifdef param_anc_add  
        AUDIO_CUSTOM_ANC_PARAM_STRUCT pSphParamAnc;
        Mutex::Autolock _l(mMutex);
        GetANCSpeechParamFromNVRam(&pSphParamAnc);
        pSphParamAnc.ANC_apply = apply;
        SetANCSpeechParamToNVRam(&pSphParamAnc);
#endif
        mApply = apply;
    }

}

bool SpeechANCController::GetANCSupport(void)
{
    ALOGD("%s(), GetANCSupport:%d", __FUNCTION__, mApply);
    //TODO(Tina): return by project config
    return true;
}

bool SpeechANCController::GetApplyANC(void)
{
    ALOGD("%s(), GetApply:%d", __FUNCTION__, mApply);
    return mApply;
}

void SpeechANCController::SetEanbleANCLog(bool enable, bool downsample)
{
    ALOGD("%s(), GetEnableLog:%d", __FUNCTION__, enable);
    if (enable ^ mLogEnable || mLogDownSample ^ downsample)
    {
#ifdef param_anc_add  
        AUDIO_CUSTOM_ANC_PARAM_STRUCT pSphParamAnc;
        Mutex::Autolock _l(mMutex);
        GetANCSpeechParamFromNVRam(&pSphParamAnc);
        pSphParamAnc.ANC_log = enable;
        pSphParamAnc.ANC_log_downsample = downsample;
        SetANCSpeechParamToNVRam(&pSphParamAnc);
#endif
        mLogEnable = enable;
        mLogDownSample = downsample;
#if 0
        if (enable)
        {
            ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_EnableLog);
        }
        else
        {
            ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_DisableLog);
        }
#endif
    }
}

bool SpeechANCController::GetEanbleANCLog(void)
{
    ALOGD("%s(), GetEnableLog:%d", __FUNCTION__, mLogEnable);
    return mLogEnable;
}

bool SpeechANCController::GetEanbleANCLogDownSample(void)
{
    ALOGD("%s(), GetLogDownSample:%d", __FUNCTION__, mLogDownSample);
    return mLogDownSample;
}

bool SpeechANCController::EanbleANC(bool enable)
{
    int ret;
    ALOGD("%s(), Enable:%d %d", __FUNCTION__, mEnabled, enable);

    if (!mGroupANC)
    {
        ALOGD("%s(), EnableError, Not ANC group", __FUNCTION__);
        return false;
    }

    if (enable ^ mEnabled)
    {
        Mutex::Autolock _l(mMutex);
#if 0
        if (enable)
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Enable);
        }
        else
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Disable);
        }
#endif
        if (ret == -1)
        {
            ALOGD("%s(), EnableFail:%d", __FUNCTION__, ret);
            return false;
        }
        mEnabled = enable;
    }
    return true;
}

bool SpeechANCController::CloseANC(void)
{
    int ret;
    ALOGD("%s(), Close", __FUNCTION__);
    if (!mGroupANC)
    {
        ALOGD("%s(), CloseError, Not ANC group", __FUNCTION__);
        return false;
    }
    Mutex::Autolock _l(mMutex);
    //    ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Close);
    if (ret == -1)
    {
        ALOGD("%s(), EnableFail:%d", __FUNCTION__, ret);
        return false;
    }
    mEnabled = false;
    return true;
}

bool SpeechANCController::SwapANC(bool swap2anc)
{
    int ret;
    ALOGD("%s(), SWAP:%d %d", __FUNCTION__, mGroupANC, swap2anc);
    if (mGroupANC ^ swap2anc)
    {
#if 0
        if (swap2anc)
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_SwapToANC);
        }
        else
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_SwapFromANC);
        }
#endif
        if (ret == -1)
        {
            ALOGD("%s(), SWAPFail:%d", __FUNCTION__, ret);
            return false;
        }
        mGroupANC = swap2anc;
    }
    return true;
}


}   //namespace android
