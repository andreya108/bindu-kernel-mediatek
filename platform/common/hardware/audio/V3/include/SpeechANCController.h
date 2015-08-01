#ifndef _SPEECH_ANC_CONTROLLER_H_
#define _SPEECH_ANC_CONTROLLER_H_


#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <utils/String8.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <media/AudioSystem.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>


namespace android
{


class SpeechANCController
{
    public:
        static SpeechANCController *getInstance();
        bool GetANCSupport(void);
        void SetApplyANC(bool apply);
        bool CloseANC(void);
        bool GetApplyANC(void);
        void SetEanbleANCLog(bool enable, bool downsample);
        bool GetEanbleANCLog(void);
        bool EanbleANC(bool enable);
        void SetCoefficients(void *buf);
        bool SwapANC(bool swap2anc);
        bool GetEanbleANCLogDownSample(void);

    private:
        SpeechANCController();
        ~SpeechANCController();

        static SpeechANCController *UniqueSpeechANCController;
        Mutex   mMutex;
        int     mFd;
        bool mEnabled;
        bool mLogEnable;
        bool mLogDownSample;
        bool mApply;
        bool mGroupANC;
};   //SpeechANCControl

}   //namespace android

#endif   //_SPEECH_ANC_CONTROL_H_
