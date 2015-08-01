#include "AudioMATVResourceManager.h"

#include "AudioMTKStreamManager.h"
#include "AudioMTKVolumeController.h"

#define LOG_TAG "AudioMATVResourceManager"

namespace android
{

/*==============================================================================
 *                     Const Value
 *============================================================================*/

static const uint16_t kMatvChipSamplingRateHz = 32000;

static const uint16_t kMatvUplinkSamplingRateHz = kMatvChipSamplingRateHz;
static const uint16_t kMatvDownlinkSamplingRateHz = 44100;

static const AudioDigtalI2S::I2S_SRC kMatvChipClockSource = AudioDigtalI2S::MASTER_MODE;

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioMATVResourceManager::AudioMATVResourceManager()
{
    ALOGD("%s()", __FUNCTION__);
    mMatvSourceType = MATV_DIGITAL;
}

AudioMATVResourceManager::~AudioMATVResourceManager()
{
    ALOGD("%s()", __FUNCTION__);
}


/*==============================================================================
 *                     Audio HW Control
 *============================================================================*/

uint32_t AudioMATVResourceManager::GetMatvUplinkSamplingRate() const
{
    ALOGD("%s(), sampling rate = %d", __FUNCTION__, kMatvUplinkSamplingRateHz);
    return kMatvUplinkSamplingRateHz;
}

uint32_t AudioMATVResourceManager::GetMatvDownlinkSamplingRate() const
{
    ALOGD("%s(), sampling rate = %d", __FUNCTION__, kMatvDownlinkSamplingRateHz);
    return kMatvDownlinkSamplingRateHz;
}

const MATVTYPE AudioMATVResourceManager::GetMatvSourceType(void)
{
    ALOGD("%s(), mMatvSourceType = %d", __FUNCTION__, mMatvSourceType);
    return mMatvSourceType;

}

status_t AudioMATVResourceManager::SetMatvSourceType(const MATVTYPE mMatvType)
{
    mMatvSourceType = mMatvType;

    ALOGD("-%s(), mMatvSourceType = %d", __FUNCTION__, mMatvSourceType);
    return NO_ERROR;

}

status_t AudioMATVResourceManager::SetMatvSourceModuleEnable(const bool enable)
{
    ALOGD("+%s(), enable = %d, mMatvSourceType= %d", __FUNCTION__, enable, mMatvSourceType);

    if (enable == true)
    {
        if (mMatvSourceType == MATV_ANALOG)
        {
            //analog pinmux       ADC<- LINEIN
            ALOGD("%s(), Analog PinMux Line in and open", __FUNCTION__);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER);
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::DEVICE_PLATFORM_MACHINE);

            // Config ADC I2S
            AudioDigtalI2S mADCI2SInAttribute;
            memset((void *)&mADCI2SInAttribute, 0, sizeof(mADCI2SInAttribute));

            mADCI2SInAttribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
            mADCI2SInAttribute.mBuffer_Update_word = 8;
            mADCI2SInAttribute.mI2S_SLAVE = kMatvChipClockSource;
            mADCI2SInAttribute.mI2S_SAMPLERATE = kMatvChipSamplingRateHz;
            mADCI2SInAttribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
            mADCI2SInAttribute.mFpga_bit_test = 0;
            mADCI2SInAttribute.mFpga_bit = 0;
            mADCI2SInAttribute.mloopback = 0;
            mADCI2SInAttribute.mI2S_FMT = AudioDigtalI2S::I2S;
            mADCI2SInAttribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
            mAudioDigitalInstance->SetI2SAdcIn(&mADCI2SInAttribute);

            // Enable ADC I2S
            mAudioDigitalInstance->SetI2SAdcEnable(true);


        }
        else
        {
            // Config 2nd I2S
            AudioDigtalI2S m2ndI2SInAttribute;
            memset((void *)&m2ndI2SInAttribute, 0, sizeof(m2ndI2SInAttribute));

            m2ndI2SInAttribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
            m2ndI2SInAttribute.mI2S_SLAVE = kMatvChipClockSource;
            m2ndI2SInAttribute.mI2S_SAMPLERATE = kMatvChipSamplingRateHz;
            m2ndI2SInAttribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
            m2ndI2SInAttribute.mI2S_FMT = AudioDigtalI2S::I2S;
            m2ndI2SInAttribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
            mAudioDigitalInstance->Set2ndI2SOut(&m2ndI2SInAttribute); // 89,8135 use Set2ndI2SOut

            // Enable 2nd I2S
            mAudioDigitalInstance->Set2ndI2SEnable(true); // 89,8135 use Set2ndI2SEnable
        }
    }
    else
    {
        if (mMatvSourceType == MATV_ANALOG)
        {
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            // Disable ADC I2S
            mAudioDigitalInstance->SetI2SAdcEnable(false);

        }
        else
        {
            // Disable 2nd I2S
            mAudioDigitalInstance->Set2ndI2SEnable(false); // 89,8135 use Set2ndI2SEnable
        }
    }

    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;

}

status_t AudioMATVResourceManager::SetMatvDirectConnection(const bool enable)
{
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);

    if (enable == true)
    {
        // Set InterConnection
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I00, AudioDigitalType::O03);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I01, AudioDigitalType::O04);

        // Set DAC I2S Out
        if (AudioMTKStreamManager::getInstance()->IsOutPutStreamActive() == false)
        {
            mAudioDigitalInstance->SetI2SDacOutAttribute(GetMatvDownlinkSamplingRate());
            mAudioDigitalInstance->SetI2SDacEnable(true);
        }
    }
    else
    {
        // Disable InterConnection
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I00, AudioDigitalType::O03);
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I01, AudioDigitalType::O04);

        // Set DAC I2S Out
        if (AudioMTKStreamManager::getInstance()->IsOutPutStreamActive() == false)
        {
            mAudioDigitalInstance->SetI2SDacEnable(false);
        }
    }

    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;
}

} // end of namespace android
