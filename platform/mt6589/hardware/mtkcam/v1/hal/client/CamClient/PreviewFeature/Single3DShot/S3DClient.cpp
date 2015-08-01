/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "MtkCam/CamClient/S3dshotClient"

#include "S3DClient.h"

//
using namespace NSCamClient;

//
/******************************************************************************
*
*******************************************************************************/

S3dshotClient*   S3dshotClientObj;
sem_t            S3dshotAddImgDone;
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
//#define debug
#ifdef debug
#include <fcntl.h>
#include <sys/stat.h>
bool
SaveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    CAM_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    CAM_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        CAM_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    CAM_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            CAM_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    CAM_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
S3dshotClient::
S3dshotClient(int ShotNum)
    : S3dshotNum(ShotNum)
{
    MY_LOGD("+ this(%p) num %d", this,S3dshotNum);
    S3dshotClientObj = this;

    // create algorithm object
    mpS3dshotObj = NULL;
    mpS3dshotObj = hal3DFBase::createInstance(HAL_PANO3D_OBJ_NORMAL);
    if ( ! mpS3dshotObj )
    {
        MY_LOGE("[init] mpS3dshotObj==NULL \n");
    }

    mpMAVObj = NULL;
    mpMAVObj = hal3DFBase::createInstance(HAL_MAV_OBJ_NORMAL);
    if ( ! mpMAVObj )
    {
        MY_LOGE("[init] mpMAVObj==NULL \n");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
S3dshotClient::
~S3dshotClient()
{
    MY_LOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
S3dshotClient::
allocMem(IMEM_BUF_INFO &memBuf)
{
	mTotalMemSize += memBuf.size;
	MY_LOGD("allocMem size=%d\n", memBuf.size);
	MY_LOGD("allocMem total=%d\n", mTotalMemSize);

    if (mpIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error \n");
        return MFALSE;
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error \n");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
S3dshotClient::
deallocMem(IMEM_BUF_INFO &memBuf)
{
	if(memBuf.virtAddr == 0) {
		return MTRUE;
	}
	mTotalMemSize -= memBuf.size;
	MY_LOGD("deallocMem total=%d\n", mTotalMemSize);

    if (mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
S3dshotClient::
init(int bufwidth,int bufheight)
{
    bool ret = false;
    MINT32 err = NO_ERROR;

    status_t status = NO_ERROR;
    //
    MY_LOGD("+");

    mS3dshotFrameWidth  = bufwidth;
    mS3dshotFrameHeight = bufheight;
    mS3dshotFrameSize   =(mS3dshotFrameWidth * mS3dshotFrameHeight * 3 / 2);
    mCancel = MTRUE;
    mStop = MFALSE;
    //
    MINT32 const i4SensorDevId = 1;

    mpHal3A = Hal3ABase::createInstance(i4SensorDevId);

    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        return false;
    }
    MY_LOGD("mS3dshotFrameWidth %d mS3dshotFrameHeight %d mS3dshotFrameSize %d S3dshotNum %d",mS3dshotFrameWidth,mS3dshotFrameHeight,mS3dshotFrameSize,S3dshotNum);

    // (1) Create frame buffer buffer
    mpframeBuffer.size =  mS3dshotFrameSize * S3dshotNum;
    if(!(allocMem(mpframeBuffer)))
    {
        mpframeBuffer.size = 0;
        MY_LOGE("[init] mpframeBuffer alloc fail");
        return false;
    }

    // (2) Create Motion working buffer buffer
    mpMotionBuffer.size = MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 3;
    if(!(allocMem(mpMotionBuffer)))
    {
        mpMotionBuffer.size = 0;
        MY_LOGE("[init] mpMotionBuffer alloc fail");
        return false;
    }

    // (3) Create Warp working buffer buffer
    mpWarpBuffer.size = mS3dshotFrameSize * 2 + 2048;
    if(!(allocMem(mpWarpBuffer)))
    {
        mpWarpBuffer.size = 0;
        MY_LOGE("[init] mpMotionBuffer alloc fail");
        return false;
    }

    // (4) MAV Shot working memory
    err = mpMAVObj->mHal3dfInit((void*)mpMAVWorkingBuf.virtAddr, (void*)mpMotionBuffer.virtAddr, (void*)mpWarpBuffer.virtAddr, (void*)mpMAVWorkingBuf.virtAddr);
    if ( err < 0 ) {
        MY_LOGE("mpS3dshotObj->mHalMavinit() Err");
        return false;
    }

    mpMAVObj->mHal3dfGetWokSize(mS3dshotFrameWidth,mS3dshotFrameHeight,mpMAVWorkingBuf.size);
    MY_LOGD("[init] MAV working buffer size %d",mpMAVWorkingBuf.size);
    if(mpMAVWorkingBuf.size==0)
        mpMAVWorkingBuf.size = mS3dshotFrameSize * 4 * 10;

    if(!(allocMem(mpMAVWorkingBuf)))
    {
        mpMAVWorkingBuf.size = 0;
        MY_LOGE("[init] mpMAVWorkingBuf alloc fail");
        return false;
    }
    mpMAVObj->mHal3dfUninit();
    mpMAVObj->destroyInstance();
    // (5) S3D Shot working memory
    MINT32 initBufSize = 0;

    err = mpS3dshotObj->mHal3dfInit((void*)mpMAVWorkingBuf.virtAddr, (void*)mpMotionBuffer.virtAddr, (void*)mpWarpBuffer.virtAddr, (void*)mpMAVWorkingBuf.virtAddr);
    if ( err < 0 ) {
        MY_LOGE("mpS3dshotObj->mHalMavinit() Err");
        return false;
    }

    err = mpS3dshotObj->mHal3dfSetWokBuff((void*)mpMAVWorkingBuf.virtAddr);
    if ( err < 0 ) {
        MY_LOGE("mpS3dshotObj->mHal3dfSetWokBuff() Err");
        return false;
    }

    // (6) reset member parameter
    mS3dshotaddImgIdx = 0;
    mS3dshotFrameIdx = 0;
    mStitchDir = MTKPIPEAUTORAMA_DIR_NO;

    // (7) thread create
    sem_init(&S3dshotSemThread, 0, 0);
    sem_init(&S3dshotmergeDone, 0, 0);
    sem_init(&S3dshotAddImgDone, 0, 0);

    pthread_create(&S3dshotFuncThread, NULL, S3dshotthreadFunc, this);

    //
    ret = true;
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
S3dshotClient::
uninit()
{
    Mutex::Autolock lock(mLockUninit);
    		
    MY_LOGD("+");

    mpHal3A->destroyInstance();

    if(!(deallocMem(mpframeBuffer)))
    {
        mpframeBuffer.size = 0;
        MY_LOGE("[uninit] mpframeBuffer alloc fail");
        return  MFALSE;
    }
    if(!(deallocMem(mpMotionBuffer)))
    {
        mpMotionBuffer.size = 0;
        MY_LOGE("[uninit] mpMotionBuffer alloc fail");
        return  MFALSE;
    }

    if(!(deallocMem(mpMAVWorkingBuf)))
    {
        mpMAVWorkingBuf.size = 0;
        MY_LOGE("[uninit] mpMAVWorkingBuf alloc fail");
        return  MFALSE;
    }

    if(!(deallocMem(mpWarpBuffer)))
    {
        mpWarpBuffer.size = 0;
        MY_LOGE("[uninit] mpWarpBuffer alloc fail");
        return  MFALSE;
    }

    if (mpS3dshotObj) {
        mpS3dshotObj->mHal3dfUninit();
        mpS3dshotObj->destroyInstance();
        mpS3dshotObj = NULL;
    }

    MY_LOGD("-");
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
S3dshotClient::
setImgCallback(ImgDataCallback_t data_cb)
{
    MY_LOGD("(notify_cb)=(%p)", data_cb);
    mDataCb = data_cb;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
S3dshotClient::
stopFeature(int cancel)
{
    MY_LOGD("+");
    bool ret = false;
	  int err;
    MY_LOGD("CAM_CMD_STOP_S3DShot, do merge %d mS3dshotaddImgIdx %d S3dshotNum %d", cancel,mS3dshotaddImgIdx,S3dshotNum);
    mCancel = cancel;
    mStop = MTRUE;

    sem_post(&S3dshotSemThread);
    pthread_join(S3dshotFuncThread, NULL);
    mpHal3A->enableAELimiterControl(false);

    if(mpS3dshotObj)
    {
        if ((cancel == 1) || (mS3dshotaddImgIdx == S3dshotNum))
        {
            // Do merge

            MY_LOGD("  CAM_CMD_STOP_S3DShot: Merge Accidently ");
            err = mHalCamFeatureMerge();
            sem_post(&S3dshotmergeDone);
            if (err != NO_ERROR)
            {
                MY_LOGD("  mHalCamFeatureMerge fail");
                return false;
            }
        }
        else
        {
            MY_LOGD("  CAM_CMD_STOP_S3DShot: Cancel");
        }
    }
    else
    {
       MY_LOGE("S3D fail: mhal3DObj is NULL");
    }
    MY_LOGD("-");
    return  true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
S3dshotClient::
mHalCamFeatureAddImg()
{
    MINT32 err = NO_ERROR;
    Mutex::Autolock lock(mLock);
    if (mS3dshotaddImgIdx >= S3dshotNum){
        MY_LOGD("mHalCamS3dshotAddImg mS3dshotaddImgIdx %d S3dshotNum %d", mS3dshotaddImgIdx, S3dshotNum);
        return err;
    }
    if(!mCancel)
    {
        MY_LOGD("mHalCamS3dshotAddImg exit mCancel %d", mCancel);
        return err;
    }
    MY_LOGD("[mHalCamFeatureAddImg mS3dshotaddImgIdx %d",mS3dshotaddImgIdx);

    ImageInfo.ImgAddr     = mpframeBuffer.virtAddr + (mS3dshotaddImgIdx * mS3dshotFrameSize);
    ImageInfo.Width       = mS3dshotFrameWidth;
    ImageInfo.Height      = mS3dshotFrameHeight;
    ImageInfo.ControlFlow = 0;

    ImageInfo.MotionValue[0] = mp3dResult.ImageInfo[mS3dshotaddImgIdx].MotionValue[0];
    ImageInfo.MotionValue[1] = mp3dResult.ImageInfo[mS3dshotaddImgIdx].MotionValue[1];
    ImageInfo.MotionValue[0] *= (-1) * float(mS3dshotFrameWidth) / MOTION_MAX_IN_WIDTH;
    ImageInfo.MotionValue[1] *= (-1) * float(mS3dshotFrameHeight) / MOTION_MAX_IN_HEIGHT;

    mp3dResult.ImageInfo[mS3dshotaddImgIdx].Width = ImageInfo.Width;
    mp3dResult.ImageInfo[mS3dshotaddImgIdx].Height = ImageInfo.Height;
    mp3dResult.ImageInfo[mS3dshotaddImgIdx].ImgAddr = ImageInfo.ImgAddr;

    MY_LOGD("ImgAddr 0x%x, Width %d, Height %d, Motion: %d %d",
             ImageInfo.ImgAddr, ImageInfo.Width, ImageInfo.Height,
             ImageInfo.MotionValue[0], ImageInfo.MotionValue[1]);

    err = mpS3dshotObj->mHal3dfAddImg((MavPipeImageInfo*)&ImageInfo);
    if ( err!= NO_ERROR )
        return err;



    mS3dshotaddImgIdx++;
    MY_LOGD("mHalCamS3dshotAddImg X");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
S3dshotClient::
mHalCamFeatureMerge()
{
    MY_LOGD("mHalS3dshotMerge");

    MINT32 err = NO_ERROR;
    sem_wait(&S3dshotAddImgDone);
    MY_LOGD(" mHalAutoramaDoStitch");


    // (1) Merge
    MY_LOGD("[mHalCamFeatureMerge] Merge");
    err = mpS3dshotObj->mHal3dfMerge((MUINT32*)&mp3dResult);
    if ( err != NO_ERROR) return err;

    //MUINT32 result;
    //MUINT32 ClipW;
    //MUINT32 ClipH;
    //err = mpS3dshotObj->mHal3dfGetResult(result,ClipW,ClipH);
    //if ( err != NO_ERROR)
    //    return err;
    //MY_LOGD("mHalMavGetResult result %d",result);

    if(mp3dResult.RetCode!=NO_ERROR)
    {
        MY_LOGD("[mHalCamFeatureMerge] Merge fail then change frame number to 2 for ori");
        mS3dshotaddImgIdx=2;
    }
    else
    {
        // (2) Warp
        MY_LOGD("[mHalCamFeatureMerge] Warp");
        ImageInfo.ImgAddr = mpframeBuffer.virtAddr;
        err = mpS3dshotObj->mHal3dfWarp((MavPipeImageInfo*)&ImageInfo,(MUINT32*)&mp3dResult, mS3dshotaddImgIdx);
        if ( err != NO_ERROR) return err;
    }

    #ifdef debug
    char sourceFiles[80];
    sprintf(sourceFiles, "%s.raw", "/sdcard/Warp0");
    SaveBufToFile(sourceFiles, (MUINT8*)mpframeBuffer.virtAddr, (mS3dshotFrameWidth * mS3dshotFrameHeight * 3 / 2));
    sprintf(sourceFiles, "%s.raw", "/sdcard/Warp1");
    SaveBufToFile(sourceFiles, (MUINT8*)(mpframeBuffer.virtAddr+(mS3dshotFrameWidth * mS3dshotFrameHeight * 3 / 2)), (mS3dshotFrameWidth * mS3dshotFrameHeight * 3 / 2));
    #endif

    // (3) stitch
    MY_LOGD("[mHalCamFeatureMerge] Stitch");
    mp3dResult.ImageInfo[0].Width = mS3dshotFrameWidth;
    mp3dResult.ImageInfo[0].Height = mS3dshotFrameHeight;
    mp3dResult.ImageInfo[0].ImgAddr = mpframeBuffer.virtAddr;
    mp3dResult.ImageInfo[0].ClipY = mp3dResult.ImageInfo[0].ClipY;

    for(MUINT32 i = 0; i < mS3dshotaddImgIdx; i++)
    {
       MY_LOGD("[mHalCamFeatureMerge] i = %d GridX %d MinX %d MaxX %d", i,mp3dResult.ImageInfo[i].GridX,mp3dResult.ImageInfo[i].MinX,mp3dResult.ImageInfo[i].MaxX);
    	 mp3dResult.ImageInfo[i].GridX = mp3dResult.ImageInfo[i].GridX;
    	 mp3dResult.ImageInfo[i].MinX = mp3dResult.ImageInfo[i].MinX;
    	 mp3dResult.ImageInfo[i].MaxX = mp3dResult.ImageInfo[i].MaxX;
    }
    mp3dResult.ClipHeight = mp3dResult.ClipHeight;
    err = mpS3dshotObj->mHal3dfStitch((MUINT32 *)&mp3dResult, mS3dshotaddImgIdx);
    if ( err != NO_ERROR) return err;
    MY_LOGD("mHal3dfStitch done");


    err = mpS3dshotObj->mHal3dfGetStitchResult((void*) &MyPano3DResultInfo);
    if ( err != NO_ERROR) return err;
    MY_LOGD("PanoWidth %d  PanoHeight %d LeftPanoImageAddr 0x%x RightPanoImageAddr 0x%x", MyPano3DResultInfo.PanoWidth, MyPano3DResultInfo.PanoHeight, MyPano3DResultInfo.LeftPanoImageAddr, MyPano3DResultInfo.RightPanoImageAddr);

    mpResultBuffer[0].virtAddr = MyPano3DResultInfo.LeftPanoImageAddr;
    mpResultBuffer[0].size = MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight*3>>1;
    mpResultBuffer[0].memID = -1;

    mpResultBuffer[1].virtAddr = MyPano3DResultInfo.RightPanoImageAddr;
    mpResultBuffer[1].size = MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight*3>>1;
    mpResultBuffer[1].memID = -1;

    mpResultBuffer[2].virtAddr = mpframeBuffer.virtAddr;
    mpResultBuffer[2].size = mpframeBuffer.size;
    mpResultBuffer[2].memID = mpframeBuffer.memID;

    #if 0
    //  temporary save 2 images to 2 files
    char tmpName[80];
    sprintf(tmpName, "%s%d.raw", "/mnt/sdcard/Pano", 1);
    mHalMiscDumpToFile((char *) tmpName, (MUINT8 *) MyPano3DResultInfo.LeftPanoImageAddr, MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight*3>>1);
    sprintf(tmpName, "%s%d.raw", "/mnt/sdcard/Pano", 2);
    mHalMiscDumpToFile((char *) tmpName, (MUINT8 *) MyPano3DResultInfo.RightPanoImageAddr, MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight*3>>1);
    #endif

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
S3dshotClient::
mHalCamFeatureCompress()
{
    MY_LOGD("[mHalCamFeatureCompress]");

    MINT32 err = NO_ERROR;

    // (1) confirm merge is done; so mutex is not necessary

    sem_wait(&S3dshotmergeDone);
    MY_LOGD("get S3dshotmergeDone semaphore");

    mDataCb((MVOID*)mpResultBuffer, MyPano3DResultInfo.PanoWidth, MyPano3DResultInfo.PanoHeight);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID*
S3dshotClient::
S3dshotthreadFunc(void *arg)
{
    MY_LOGD("[S3dshotthreadFunc] +");

    ::prctl(PR_SET_NAME,"PanoTHREAD", 0, 0, 0);

    // (1) set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );

    // loop for thread until access uninit state
    while(!S3dshotClientObj->mStop
        || (S3dshotClientObj->mS3dshotaddImgIdx<S3dshotClientObj->mS3dshotFrameIdx)
        )
    {
        MY_LOGD("[S3dshot][S3dshotthreadFunc]: wait thread");
        int SemValue;
        sem_getvalue(&S3dshotClientObj->S3dshotSemThread, &SemValue);
        MY_LOGD("Semaphone value: %d", SemValue);
        sem_wait(&S3dshotClientObj->S3dshotSemThread);
        MY_LOGD("get S3dshotSemThread Semaphone");
        MINT32 err = S3dshotClientObj->mHalCamFeatureAddImg();
        if (err != NO_ERROR) {
             MY_LOGD("[mHalCamFeatureAddImg] fail");
        }
        MY_LOGD("[S3dshot][S3dshotthreadFunc]: after do merge");
    }
    sem_post(&S3dshotAddImgDone);
    MY_LOGD("[S3dshotthreadFunc] -");
    return NULL;
}
