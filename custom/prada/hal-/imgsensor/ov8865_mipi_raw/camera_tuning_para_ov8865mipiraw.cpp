#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8865mipiraw.h"
#include "camera_info_ov8865mipiraw.h"
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
        57320,    // i4R_AVG
        16776,    // i4R_STD
        90360,    // i4B_AVG
        23418,    // i4B_STD
        {  // i4P00[9]
            4356000, -1776000, -16000, -740000, 3826000, -532000, 62000, -2668000, 5164000
        },
        {  // i4P10[9]
            1630062, -1661095, 43528, -60036, -317044, 383410, 39053, 426107, -463628
        },
        {  // i4P01[9]
            815725, -981279, 181740, -112660, -185865, 303944, 8355, -445645, 442494
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
            1144,    // u4MinGain, 1024 base = 1x
            8192,    // u4MaxGain, 16x
            80,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            27,    // u4PreExpUnit 
            31,    // u4PreMaxFrameRate
            18,    // u4VideoExpUnit  
            31,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            14,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            372    // u4FocusLength_100x
        },
        // rHistConfig
        {
            4,    // u4HistHighThres
            30,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            170,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 230, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {86, 108, 100, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 18}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            TRUE,    // bEnableCaptureThres
            TRUE,    // bEnableVideoThres
            TRUE,    // bEnableStrobeThres
            45,    // u4AETarget
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            5,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            4,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            70,    // u4InDoorEV = 9.0, 10 base 
            -7,    // i4BVOffset delta BV = value/10 
            4,    // u4PreviewFlareOffset
            4,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            4,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            2,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            8,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            8,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            56    // u4FlatnessStrength
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
                782,    // i4R
                512,    // i4G
                573    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                115,    // i4X
                -198    // i4Y
            },
            // Horizon
            {
                -419,    // i4X
                -192    // i4Y
            },
            // A
            {
                -308,    // i4X
                -220    // i4Y
            },
            // TL84
            {
                -171,    // i4X
                -232    // i4Y
            },
            // CWF
            {
                -104,    // i4X
                -315    // i4Y
            },
            // DNP
            {
                15,    // i4X
                -266    // i4Y
            },
            // D65
            {
                115,    // i4X
                -198    // i4Y
            },
            // DF
            {
                66,    // i4X
                -280    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                108,    // i4X
                -202    // i4Y
            },
            // Horizon
            {
                -426,    // i4X
                -177    // i4Y
            },
            // A
            {
                -316,    // i4X
                -209    // i4Y
            },
            // TL84
            {
                -179,    // i4X
                -226    // i4Y
            },
            // CWF
            {
                -115,    // i4X
                -311    // i4Y
            },
            // DNP
            {
                6,    // i4X
                -267    // i4Y
            },
            // D65
            {
                108,    // i4X
                -202    // i4Y
            },
            // DF
            {
                56,    // i4X
                -282    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                782,    // i4R
                512,    // i4G
                573    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                696,    // i4G
                1592    // i4B
            },
            // A 
            {
                512,    // i4R
                577,    // i4G
                1178    // i4B
            },
            // TL84 
            {
                557,    // i4R
                512,    // i4G
                884    // i4B
            },
            // CWF 
            {
                681,    // i4R
                512,    // i4G
                903    // i4B
            },
            // DNP 
            {
                749,    // i4R
                512,    // i4G
                719    // i4B
            },
            // D65 
            {
                782,    // i4R
                512,    // i4G
                573    // i4B
            },
            // DF 
            {
                819,    // i4R
                512,    // i4G
                684    // i4B
            }
        },
        // Rotation matrix parameter
        {
            2,    // i4RotationAngle
            256,    // i4Cos
            9    // i4Sin
        },
        // Daylight locus parameter
        {
            -135,    // i4SlopeNumerator
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
            -209,    // i4RightBound
            -879,    // i4LeftBound
            -143,    // i4UpperBound
            -243    // i4LowerBound
            },
            // Warm fluorescent
            {
            -209,    // i4RightBound
            -879,    // i4LeftBound
            -243,    // i4UpperBound
            -363    // i4LowerBound
            },
            // Fluorescent
            {
            -44,    // i4RightBound
            -209,    // i4LeftBound
            -132,    // i4UpperBound
            -268    // i4LowerBound
            },
            // CWF
            {
            -44,    // i4RightBound
            -209,    // i4LeftBound
            -268,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Daylight
            {
            163,    // i4RightBound
            -44,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Shade
            {
            493,    // i4RightBound
            163,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            163,    // i4RightBound
            -44,    // i4LeftBound
            -292,    // i4UpperBound
            -372    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            493,    // i4RightBound
            -879,    // i4LeftBound
            0,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Daylight
            {
            188,    // i4RightBound
            -44,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Cloudy daylight
            {
            288,    // i4RightBound
            113,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Shade
            {
            388,    // i4RightBound
            113,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Twilight
            {
            -44,    // i4RightBound
            -204,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Fluorescent
            {
            158,    // i4RightBound
            -279,    // i4LeftBound
            -152,    // i4UpperBound
            -361    // i4LowerBound
            },
            // Warm fluorescent
            {
            -216,    // i4RightBound
            -416,    // i4LeftBound
            -152,    // i4UpperBound
            -361    // i4LowerBound
            },
            // Incandescent
            {
            -216,    // i4RightBound
            -416,    // i4LeftBound
            -122,    // i4UpperBound
            -292    // i4LowerBound
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
            751,    // i4R
            512,    // i4G
            606    // i4B
            },
            // Cloudy daylight
            {
            889,    // i4R
            512,    // i4G
            507    // i4B
            },
            // Shade
            {
            948,    // i4R
            512,    // i4G
            472    // i4B
            },
            // Twilight
            {
            582,    // i4R
            512,    // i4G
            798    // i4B
            },
            // Fluorescent
            {
            678,    // i4R
            512,    // i4G
            779    // i4B
            },
            // Warm fluorescent
            {
            485,    // i4R
            512,    // i4G
            1113    // i4B
            },
            // Incandescent
            {
            453,    // i4R
            512,    // i4G
            1044    // i4B
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
            6751    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5320    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1750    // i4OffsetThr
            },
            // Daylight WB gain
            {
            679,    // i4R
            516,    // i4G
            512    // i4B
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
                -534,    // i4RotatedXCoordinate[0]
                -424,    // i4RotatedXCoordinate[1]
                -287,    // i4RotatedXCoordinate[2]
                -102,    // i4RotatedXCoordinate[3]
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


