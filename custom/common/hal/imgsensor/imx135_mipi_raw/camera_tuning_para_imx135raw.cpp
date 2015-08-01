#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx135raw.h"
#include "camera_info_imx135raw.h"
#include "camera_custom_AEPlinetable.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,
    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
        },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        80125,    // i4R_AVG
        18390,    // i4R_STD
        97700,    // i4B_AVG
        21579,    // i4B_STD
        {  // i4P00[9]
            4512500, -1855000, -102500, -910000, 3962500, -490000, -210000, -1910000, 4680000
        },
        {  // i4P10[9]
            2033293, -2626505, 603090, -206254, -275533, 455559, 217869, 492543, -710412
        },
        {  // i4P01[9]
            1401262, -1769602, 382411, -404504, -274781, 648702, 30163, -67441, 37278
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1136,    // u4MinGain, 1024 base = 1x
            32768,    // u4MaxGain, 16x
            85,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            21,    // u4PreExpUnit 
            31,    // u4PreMaxFrameRate
            21,    // u4VideoExpUnit  
            31,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            22,    // u4CapExpUnit 
            15,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            20,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            4,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {82, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            47,    // u4AETarget
            47,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            4,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -10,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            0,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            0,    // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            0,    // u4StrobeFlareThres
            0,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            0,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                1030,    // i4R
                512,    // i4G
                685    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                151,    // i4X
                -365    // i4Y
            },
            // Horizon
            {
                -415,    // i4X
                -343    // i4Y
            },
            // A
            {
                -286,    // i4X
                -359    // i4Y
            },
            // TL84
            {
                -77,    // i4X
                -385    // i4Y
            },
            // CWF
            {
                -80,    // i4X
                -439    // i4Y
            },
            // DNP
            {
                8,    // i4X
                -396    // i4Y
            },
            // D65
            {
                151,    // i4X
                -365    // i4Y
            },
            // DF
            {
                -29,    // i4X
                -433    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                151,    // i4X
                -365    // i4Y
            },
            // Horizon
            {
                -415,    // i4X
                -343    // i4Y
            },
            // A
            {
                -286,    // i4X
                -359    // i4Y
            },
            // TL84
            {
                -77,    // i4X
                -385    // i4Y
            },
            // CWF
            {
                -80,    // i4X
                -439    // i4Y
            },
            // DNP
            {
                8,    // i4X
                -396    // i4Y
            },
            // D65
            {
                151,    // i4X
                -365    // i4Y
            },
            // DF
            {
                -29,    // i4X
                -433    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                1030,    // i4R
                512,    // i4G
                685    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                564,    // i4G
                1575    // i4B
            },
            // A 
            {
                566,    // i4R
                512,    // i4G
                1226    // i4B
            },
            // TL84 
            {
                777,    // i4R
                512,    // i4G
                957    // i4B
            },
            // CWF 
            {
                832,    // i4R
                512,    // i4G
                1034    // i4B
            },
            // DNP 
            {
                886,    // i4R
                512,    // i4G
                865    // i4B
            },
            // D65 
            {
                1030,    // i4R
                512,    // i4G
                685    // i4B
            },
            // DF 
            {
                885,    // i4R
                512,    // i4G
                956    // i4B
            }
        },
        // Rotation matrix parameter
        {
            0,    // i4RotationAngle
            256,    // i4Cos
            0    // i4Sin
        },
        // Daylight locus parameter
        {
            -123,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -127,    // i4RightBound
            -777,    // i4LeftBound
            -301,    // i4UpperBound
            -401    // i4LowerBound
            },
            // Warm fluorescent
            {
            -127,    // i4RightBound
            -777,    // i4LeftBound
            -401,    // i4UpperBound
            -521    // i4LowerBound
            },
            // Fluorescent
            {
            -42,    // i4RightBound
            -127,    // i4LeftBound
            -293,    // i4UpperBound
            -412    // i4LowerBound
            },
            // CWF
            {
            -42,    // i4RightBound
            -127,    // i4LeftBound
            -412,    // i4UpperBound
            -489    // i4LowerBound
            },
            // Daylight
            {
            176,    // i4RightBound
            -42,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Shade
            {
            536,    // i4RightBound
            176,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            176,    // i4RightBound
            -42,    // i4LeftBound
            -445,    // i4UpperBound
            -550    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            536,    // i4RightBound
            -777,    // i4LeftBound
            0,    // i4UpperBound
            -550    // i4LowerBound
            },
            // Daylight
            {
            201,    // i4RightBound
            -42,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Cloudy daylight
            {
            301,    // i4RightBound
            126,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Shade
            {
            401,    // i4RightBound
            126,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Twilight
            {
            -42,    // i4RightBound
            -202,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Fluorescent
            {
            201,    // i4RightBound
            -180,    // i4LeftBound
            -315,    // i4UpperBound
            -489    // i4LowerBound
            },
            // Warm fluorescent
            {
            -186,    // i4RightBound
            -386,    // i4LeftBound
            -315,    // i4UpperBound
            -489    // i4LowerBound
            },
            // Incandescent
            {
            -186,    // i4RightBound
            -386,    // i4LeftBound
            -285,    // i4UpperBound
            -445    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            935,    // i4R
            512,    // i4G
            754    // i4B
            },
            // Cloudy daylight
            {
            1120,    // i4R
            512,    // i4G
            629    // i4B
            },
            // Shade
            {
            1199,    // i4R
            512,    // i4G
            587    // i4B
            },
            // Twilight
            {
            711,    // i4R
            512,    // i4G
            990    // i4B
            },
            // Fluorescent
            {
            895,    // i4R
            512,    // i4G
            870    // i4B
            },
            // Warm fluorescent
            {
            599,    // i4R
            512,    // i4G
            1300    // i4B
            },
            // Incandescent
            {
            570,    // i4R
            512,    // i4G
            1236    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            7399    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            4788    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1341    // i4OffsetThr
            },
            // Daylight WB gain
            {
            848,    // i4R
            512,    // i4G
            830    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -566,    // i4RotatedXCoordinate[0]
                -437,    // i4RotatedXCoordinate[1]
                -228,    // i4RotatedXCoordinate[2]
                -143,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T)};

    if (CameraDataType > CAMERA_DATA_AE_PLINETABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        default:
            break;
    }
    return 0;
}}; // NSFeature


