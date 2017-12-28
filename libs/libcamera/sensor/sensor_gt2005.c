	/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <utils/Log.h>
#include "sensor.h"
#include "jpeg_exif_header.h"
#include "sensor_drv_u.h"

#define GT2005_I2C_ADDR_W 0x3c
#define GT2005_I2C_ADDR_R 0x3c
#define SENSOR_GAIN_SCALE 16

 typedef enum
{
	FLICKER_50HZ = 0,
	FLICKER_60HZ,
	FLICKER_MAX
}FLICKER_E;

static uint32_t set_GT2005_ae_enable(uint32_t enable);
static uint32_t GT2005_PowerOn(uint32_t power_on);
static uint32_t set_preview_mode(uint32_t preview_mode);
static uint32_t GT2005_Identify(uint32_t param);
static uint32_t GT2005_BeforeSnapshot(uint32_t param);
static uint32_t GT2005_After_Snapshot(uint32_t param);
static uint32_t set_brightness(uint32_t level);
static uint32_t set_contrast(uint32_t level);
static uint32_t set_sharpness(uint32_t level);
static uint32_t set_saturation(uint32_t level);
static uint32_t set_image_effect(uint32_t effect_type);
static uint32_t read_ev_value(uint32_t value);
static uint32_t write_ev_value(uint32_t exposure_value);
static uint32_t read_gain_value(uint32_t value);
static uint32_t write_gain_value(uint32_t gain_value);
static uint32_t read_gain_scale(uint32_t value);
static uint32_t set_frame_rate(uint32_t param);
static uint32_t set_GT2005_ev(uint32_t level);
static uint32_t set_GT2005_awb(uint32_t mode);
static uint32_t set_GT2005_anti_flicker(uint32_t mode);
static uint32_t set_GT2005_video_mode(uint32_t mode);
static void GT2005_set_shutter();
static uint32_t set_sensor_flip(uint32_t param);

static SENSOR_REG_T GT2005_YUV_COMMON[]=
{
#if 1
	{0x0101, 0x00},
	{0x0103, 0x00},

	//Hcouou,
	{0x0105, 0x00},
	{0x0106, 0xF0},
	{0x0107, 0x00},
	{0x0108, 0x1C},

	//Binnes, on
	//1600  ,
	{0x0109, 0x01},
	{0x010A, 0x00},
	{0x010B, 0x00},
	{0x010C, 0x00},
	{0x010D, 0x08},
	{0x010E, 0x00},
	{0x010F, 0x08},
	{0x0110, 0x06},
	{0x0111, 0x40},
	{0x0112, 0x04},
	{0x0113, 0xB0},

	//YUV   ,
	{0x0114, 0x04},//YUYV

	//Pictff,
	{0x0115, 0x00},

	//PLL& R,
	{0x0116, 0x02},
	{0x0117, 0x00},
        {0x0118, 0x40},
	{0x0119, 0x01},
	{0x011A, 0x04},
	{0x011B, 0x00},

	//DCLKri,
	{0x011C, 0x00},

	//Do nan,
	{0x011D, 0x02},
	{0x011E, 0x00},

	{0x011F, 0x00},
	{0x0120, 0x1C},
	{0x0121, 0x00},
	{0x0122, 0x04},
	{0x0123, 0x00},
	{0x0124, 0x00},
	{0x0125, 0x00},
	{0x0126, 0x00},
	{0x0127, 0x00},
	{0x0128, 0x00},

	//Cont  ,
	{0x0200, 0x00},

	//Brigs ,
	{0x0201, 0x00},

	//Satun ,
	{0x0202, 0x40},

	//Do nan,
	{0x0203, 0x00},
	{0x0204, 0x03},
	{0x0205, 0x1F},
	{0x0206, 0x0B},
	{0x0207, 0x20},
	{0x0208, 0x00},
	{0x0209, 0x2A},
	{0x020A, 0x01},

	//Shar  ,
	{0x020B, 0x48},
	{0x020C, 0x64},

	//Do nan,
	{0x020D, 0xC8},
	{0x020E, 0xBC},
	{0x020F, 0x08},
	{0x0210, 0xD6},
	{0x0211, 0x00},
	{0x0212, 0x20},
	{0x0213, 0x81},
	{0x0214, 0x15},
	{0x0215, 0x00},
	{0x0216, 0x00},
	{0x0217, 0x00},
	{0x0218, 0x46},
	{0x0219, 0x30},
	{0x021A, 0x03},
	{0x021B, 0x28},
	{0x021C, 0x02},
	{0x021D, 0x60},
	{0x021E, 0x00},
	{0x021F, 0x00},
	{0x0220, 0x08},
	{0x0221, 0x08},
	{0x0222, 0x04},
	{0x0223, 0x00},
	{0x0224, 0x1F},
	{0x0225, 0x1E},
	{0x0226, 0x18},
	{0x0227, 0x1D},
	{0x0228, 0x1F},
	{0x0229, 0x1F},
	{0x022A, 0x01},
	{0x022B, 0x04},
	{0x022C, 0x05},
	{0x022D, 0x05},
	{0x022E, 0x04},
	{0x022F, 0x03},
	{0x0230, 0x02},
	{0x0231, 0x1F},
	{0x0232, 0x1A},
	{0x0233, 0x19},
	{0x0234, 0x19},
	{0x0235, 0x1B},
	{0x0236, 0x1F},
	{0x0237, 0x04},
	{0x0238, 0xEE},
	{0x0239, 0xFF},
	{0x023A, 0x00},
	{0x023B, 0x00},
	{0x023C, 0x00},
	{0x023D, 0x00},
	{0x023E, 0x00},
	{0x023F, 0x00},
	{0x0240, 0x00},
	{0x0241, 0x00},
	{0x0242, 0x00},
	{0x0243, 0x21},
	{0x0244, 0x42},
	{0x0245, 0x53},
	{0x0246, 0x54},
	{0x0247, 0x54},
	{0x0248, 0x54},
	{0x0249, 0x33},
	{0x024A, 0x11},
	{0x024B, 0x00},
	{0x024C, 0x00},
	{0x024D, 0xFF},
	{0x024E, 0xEE},
	{0x024F, 0xDD},
	{0x0250, 0x00},
	{0x0251, 0x00},
	{0x0252, 0x00},
	{0x0253, 0x00},
	{0x0254, 0x00},
	{0x0255, 0x00},
	{0x0256, 0x00},
	{0x0257, 0x00},
	{0x0258, 0x00},
	{0x0259, 0x00},
	{0x025A, 0x00},
	{0x025B, 0x00},
	{0x025C, 0x00},
	{0x025D, 0x00},
	{0x025E, 0x00},
	{0x025F, 0x00},
	{0x0260, 0x00},
	{0x0261, 0x00},
	{0x0262, 0x00},
	{0x0263, 0x00},
	{0x0264, 0x00},
	{0x0265, 0x00},
	{0x0266, 0x00},
	{0x0267, 0x00},
	{0x0268, 0x8F},
	{0x0269, 0xA3},
	{0x026A, 0xB4},
	{0x026B, 0x90},
	{0x026C, 0x00},
	{0x026D, 0xD0},
	{0x026E, 0x60},
	{0x026F, 0xA0},
	{0x0270, 0x40},
	{0x0300, 0x81},
	{0x0301, 0x80},
	{0x0302, 0x22},
	{0x0303, 0x06},
	{0x0304, 0x03},
	{0x0305, 0x83},
	{0x0306, 0x00},
	{0x0307, 0x22},
	{0x0308, 0x00},
	{0x0309, 0x55},
	{0x030A, 0x55},
	{0x030B, 0x55},
	{0x030C, 0x54},
	{0x030D, 0x1F},
	{0x030E, 0x0A},
	{0x030F, 0x10},
	{0x0310, 0x04},
	{0x0311, 0xFF},
	{0x0312, 0x98},
	{0x0313, 0x26},
	{0x0314, 0xff},
	{0x0315, 0x96},
	{0x0316, 0x26},
	{0x0317, 0x02},
	{0x0318, 0x08},
	{0x0319, 0x0C},

	//NormB , ng
	{0x031A, 0x81},
	{0x031B, 0x00},
	{0x031C, 0x3D},
	{0x031D, 0x00},
	{0x031E, 0xF9},
	{0x031F, 0x00},
	{0x0320, 0x24},
	{0x0321, 0x14},
	{0x0322, 0x1A},
	{0x0323, 0x24},
	{0x0324, 0x08},
	{0x0325, 0xF0},
	{0x0326, 0x30},
	{0x0327, 0x17},
	{0x0328, 0x11},
	{0x0329, 0x22},
	{0x032A, 0x2F},
	{0x032B, 0x21},
	{0x032C, 0xDA},
	{0x032D, 0x10},
	{0x032E, 0xEA},
	{0x032F, 0x18},
	{0x0330, 0x29},
	{0x0331, 0x25},
	{0x0332, 0x12},
	{0x0333, 0x0F},
	{0x0334, 0xE0},
	{0x0335, 0x13},
	{0x0336, 0xFF},
	{0x0337, 0x20},
	{0x0338, 0x46},
	{0x0339, 0x04},
	{0x033A, 0x04},
	{0x033B, 0xFF},
	{0x033C, 0x01},
	{0x033D, 0x00},

	//Do nan,
	{0x033E, 0x03},
	{0x033F, 0x28},
	{0x0340, 0x02},
	{0x0341, 0x60},
	{0x0342, 0xAC},
	{0x0343, 0x97},
	{0x0344, 0x7F},
	{0x0400, 0xE8},
	{0x0401, 0x40},
	{0x0402, 0x00},
	{0x0403, 0x00},
	{0x0404, 0xF8},
	{0x0405, 0x03},
	{0x0406, 0x03},
	{0x0407, 0x85},
	{0x0408, 0x44},
	{0x0409, 0x1F},
	{0x040A, 0x40},
	{0x040B, 0x33},

	//Lensin, rectin
	{0x040C, 0xA0},
	{0x040D, 0x00},
	{0x040E, 0x00},
	{0x040F, 0x00},
	{0x0410, 0x0D},
	{0x0411, 0x0D},
	{0x0412, 0x0C},
	{0x0413, 0x04},
	{0x0414, 0x00},
	{0x0415, 0x00},
	{0x0416, 0x07},
	{0x0417, 0x09},
	{0x0418, 0x16},
	{0x0419, 0x14},
	{0x041A, 0x11},
	{0x041B, 0x14},
	{0x041C, 0x07},
	{0x041D, 0x07},
	{0x041E, 0x06},
	{0x041F, 0x02},
	{0x0420, 0x42},
	{0x0421, 0x42},
	{0x0422, 0x47},
	{0x0423, 0x39},
	{0x0424, 0x3E},
	{0x0425, 0x4D},
	{0x0426, 0x46},
	{0x0427, 0x3A},
	{0x0428, 0x21},
	{0x0429, 0x21},
	{0x042A, 0x26},
	{0x042B, 0x1C},
	{0x042C, 0x25},
	{0x042D, 0x25},
	{0x042E, 0x28},
	{0x042F, 0x20},
	{0x0430, 0x3E},
	{0x0431, 0x3E},
	{0x0432, 0x33},
	{0x0433, 0x2E},
	{0x0434, 0x54},
	{0x0435, 0x53},
	{0x0436, 0x3C},
	{0x0437, 0x51},
	{0x0438, 0x2B},
	{0x0439, 0x2B},
	{0x043A, 0x38},
	{0x043B, 0x22},
	{0x043C, 0x3B},
	{0x043D, 0x3B},
	{0x043E, 0x31},
	{0x043F, 0x37},

	//PWB   ,
	{0x0440, 0x00},
	{0x0441, 0x4B},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x31},

	{0x0445, 0x00},
	{0x0446, 0x00},
	{0x0447, 0x00},
	{0x0448, 0x00},
	{0x0449, 0x00},
	{0x044A, 0x00},
	{0x044D, 0xE0},
	{0x044E, 0x05},
	{0x044F, 0x07},
	{0x0450, 0x00},
	{0x0451, 0x00},
	{0x0452, 0x00},
	{0x0453, 0x00},
	{0x0454, 0x00},
	{0x0455, 0x00},
	{0x0456, 0x00},
	{0x0457, 0x00},
	{0x0458, 0x00},
	{0x0459, 0x00},
	{0x045A, 0x00},
	{0x045B, 0x00},
	{0x045C, 0x00},
	{0x045D, 0x00},
	{0x045E, 0x00},
	{0x045F, 0x00},

	//GAMMre,
	{0x0460, 0x80},
	{0x0461, 0x10},
	{0x0462, 0x10},
	{0x0463, 0x10},
	{0x0464, 0x08},
	{0x0465, 0x08},
	{0x0466, 0x11},
	{0x0467, 0x09},
	{0x0468, 0x23},
	{0x0469, 0x2A},
	{0x046A, 0x2A},
	{0x046B, 0x47},
	{0x046C, 0x52},
	{0x046D, 0x42},
	{0x046E, 0x36},
	{0x046F, 0x46},
	{0x0470, 0x3A},
	{0x0471, 0x32},
	{0x0472, 0x32},
	{0x0473, 0x38},
	{0x0474, 0x3D},
	{0x0475, 0x2F},
	{0x0476, 0x29},
	{0x0477, 0x48},

	//Do nan,
	{0x0600, 0x00},
	{0x0601, 0x24},
	{0x0602, 0x45},
	{0x0603, 0x0E},
	{0x0604, 0x14},
	{0x0605, 0x2F},
	{0x0606, 0x01},
	{0x0607, 0x0E},
	{0x0608, 0x0E},
	{0x0609, 0x37},
	{0x060A, 0x18},
	{0x060B, 0xA0},
	{0x060C, 0x20},
	{0x060D, 0x07},
	{0x060E, 0x47},
	{0x060F, 0x90},
	{0x0610, 0x06},
	{0x0611, 0x0C},
	{0x0612, 0x28},
	{0x0613, 0x13},
	{0x0614, 0x0B},
	{0x0615, 0x10},
	{0x0616, 0x14},
	{0x0617, 0x19},
	{0x0618, 0x52},
	{0x0619, 0xA0},
	{0x061A, 0x11},
	{0x061B, 0x33},
	{0x061C, 0x56},
	{0x061D, 0x20},
	{0x061E, 0x28},
	{0x061F, 0x2B},
	{0x0620, 0x22},
	{0x0621, 0x11},
	{0x0622, 0x75},
	{0x0623, 0x49},
	{0x0624, 0x6E},
	{0x0625, 0x80},
	{0x0626, 0x02},
	{0x0627, 0x0C},
	{0x0628, 0x51},
	{0x0629, 0x25},
	{0x062A, 0x01},
	{0x062B, 0x3D},
	{0x062C, 0x04},
	{0x062D, 0x01},
	{0x062E, 0x0C},
	{0x062F, 0x2C},
	{0x0630, 0x0D},
	{0x0631, 0x14},
	{0x0632, 0x12},
	{0x0633, 0x34},
	{0x0634, 0x00},
	{0x0635, 0x00},
	{0x0636, 0x00},
	{0x0637, 0xB1},
	{0x0638, 0x22},
	{0x0639, 0x32},
	{0x063A, 0x0E},
	{0x063B, 0x18},
	{0x063C, 0x88},
	{0x0640, 0xB2},
	{0x0641, 0xC0},
	{0x0642, 0x01},
	{0x0643, 0x26},
	{0x0644, 0x13},
	{0x0645, 0x88},
	{0x0646, 0x64},
	{0x0647, 0x00},
	{0x0681, 0x1B},
	{0x0682, 0xA0},
	{0x0683, 0x28},
	{0x0684, 0x00},
	{0x0685, 0xB0},
	{0x0686, 0x6F},
	{0x0687, 0x33},
	{0x0688, 0x1F},
	{0x0689, 0x44},
	{0x068A, 0xA8},
	{0x068B, 0x44},
	{0x068C, 0x08},
	{0x068D, 0x08},
	{0x068E, 0x00},
	{0x068F, 0x00},
	{0x0690, 0x01},
	{0x0691, 0x00},
	{0x0692, 0x01},
	{0x0693, 0x00},
	{0x0694, 0x00},
	{0x0695, 0x00},
	{0x0696, 0x00},
	{0x0697, 0x00},
	{0x0698, 0x2A},
	{0x0699, 0x80},
	{0x069A, 0x1F},
	{0x069B, 0x00},
	{0x069C, 0x02},
	{0x069D, 0xF5},
	{0x069E, 0x03},
	{0x069F, 0x6D},
	{0x06A0, 0x0C},
	{0x06A1, 0xB8},
	{0x06A2, 0x0D},
	{0x06A3, 0x74},
	{0x06A4, 0x00},
	{0x06A5, 0x2F},
	{0x06A6, 0x00},
	{0x06A7, 0x2F},
	{0x0F00, 0x00},
	{0x0F01, 0x00},

	//Outpab,
	{0x0100, 0x01},
	{0x0102, 0x02},
	{0x0104, 0x03},
#else
	{0x0102 , 0x01},
	{0x0103 , 0x00},
	{0x0105 , 0x00},
	{0x0106 , 0xF0},
	{0x0107 , 0x00},
	{0x0108 , 0x1C},
	{0x0109 , 0x01},
	{0x010A , 0x00},
	{0x010B , 0x0B},
	{0x010C , 0x00},
	{0x010D , 0x08},
	{0x010E , 0x00},
	{0x010F , 0x08},
	{0x0110 , 0x06},
	{0x0111 , 0x40},
	{0x0112 , 0x04},
	{0x0113 , 0xB0},
	{0x0114 , 0x04},
	{0x0115 , 0x00},
	{0x0116 , 0x02},
	{0x0117 , 0x00},
	//{0x0118 , 0x68}, //30 fps
	//{0x0118 , 0x4c}, //22 fps
	{0x0118 , 0x40}, //18 fps
	{0x0119 , 0x01},
	{0x011A , 0x04},
	{0x011B , 0x00},
	{0x011C , 0x01},
	{0x011D , 0x01},
	{0x011E , 0x00},
	{0x011F , 0x00},
	{0x0120 , 0x1C},
	{0x0121 , 0x00},
	{0x0122 , 0x04},
	{0x0123 , 0x00},
	{0x0124 , 0x00},
	{0x0125 , 0x00},
	{0x0126 , 0x00},
	{0x0127 , 0x00},
	{0x0128 , 0x00},
	{0x0200 , 0x20},
	{0x0201 , 0x00},
	{0x0202 , 0x4f},
	{0x0203 , 0x00},
	{0x0204 , 0x00},
	{0x0205 , 0x09},
	{0x0206 , 0x0A},
	{0x0207 , 0x20},
	{0x0208 , 0x00},
	{0x0209 , 0x59},
	{0x020A , 0x01},
	{0x020B , 0x28},
	{0x020C , 0x44},
	{0x020D , 0xE2},
	{0x020E , 0xB0},
	{0x020F , 0x08},
	{0x0210 , 0xF6},
	{0x0211 , 0x00},
	{0x0212 , 0x20},
	{0x0213 , 0x81},
	{0x0214 , 0x15},
	{0x0215 , 0x00},
	{0x0216 , 0x00},
	{0x0217 , 0x00},
	{0x0218 , 0x44},
	{0x0219 , 0x30},
	{0x021A , 0x03},
	{0x021B , 0x28},
	{0x021C , 0x02},
	{0x021D , 0x60},
	{0x0220 , 0x00},
	{0x0221 , 0x00},
	{0x0222 , 0x00},
	{0x0223 , 0x00},
	{0x0224 , 0x00},
	{0x0225 , 0x00},
	{0x0226 , 0x00},
	{0x0227 , 0x00},
	{0x0228 , 0x00},
	{0x0229 , 0x00},
	{0x022A , 0x00},
	{0x022B , 0x00},
	{0x022C , 0x00},
	{0x022D , 0x00},
	{0x022E , 0x00},
	{0x022F , 0x00},
	{0x0230 , 0x00},
	{0x0231 , 0x00},
	{0x0232 , 0x00},
	{0x0233 , 0x00},
	{0x0234 , 0x00},
	{0x0235 , 0x00},
	{0x0236 , 0x00},
	{0x0237 , 0x00},
	{0x0238 , 0x00},
	{0x0239 , 0x00},
	{0x023A , 0x00},
	{0x023B , 0x00},
	{0x023C , 0x00},
	{0x023D , 0x00},
	{0x023E , 0x00},
	{0x023F , 0x00},
	{0x0240 , 0x00},
	{0x0241 , 0x00},
	{0x0242 , 0x00},
	{0x0243 , 0x00},
	{0x0244 , 0x00},
	{0x0245 , 0x00},
	{0x0246 , 0x00},
	{0x0247 , 0x00},
	{0x0248 , 0x00},
	{0x0249 , 0x00},
	{0x024A , 0x00},
	{0x024B , 0x00},
	{0x024C , 0x00},
	{0x024D , 0x00},
	{0x024E , 0x00},
	{0x024F , 0x00},
	{0x0250 , 0x01},
	{0x0251 , 0x00},
	{0x0252 , 0x00},
	{0x0253 , 0x00},
	{0x0254 , 0x00},
	{0x0255 , 0x00},
	{0x0256 , 0x00},
	{0x0257 , 0x00},
	{0x0258 , 0x00},
	{0x0259 , 0x00},
	{0x025A , 0x00},
	{0x025B , 0x00},
	{0x025C , 0x00},
	{0x025D , 0x00},
	{0x025E , 0x00},
	{0x025F , 0x00},
	{0x0260 , 0x00},
	{0x0261 , 0x00},
	{0x0262 , 0x00},
	{0x0263 , 0x00},
	{0x0264 , 0x00},
	{0x0265 , 0x00},
	{0x0266 , 0x00},
	{0x0267 , 0x00},
	{0x0268 , 0x8F},
	{0x0269 , 0xA3},
	{0x026A , 0xB4},
	{0x026B , 0x90},
	{0x026C , 0x00},
	{0x026D , 0xD0},
	{0x026E , 0x60},
	{0x026F , 0xA0},
	{0x0270 , 0x40},
	{0x0300 , 0x81},
	{0x0301 , 0xA8},
	{0x0302 , 0x0D},
	{0x0303 , 0x05},
	{0x0304 , 0x03},
	{0x0305 , 0x43},
	{0x0306 , 0x00},
	{0x0307 , 0x0D},
	{0x0308 , 0x00},
	{0x0309 , 0x55},
	{0x030A , 0x55},
	{0x030B , 0x55},
	{0x030C , 0x54},
	{0x030D , 0x13},
	{0x030E , 0x0A},
	{0x030F , 0x10},
	{0x0310 , 0x04},
	{0x0311 , 0xFF},
	{0x0312 , 0x98},
	{0x0313 , 0x35}, //18 fps
	{0x0314 , 0x36}, //18 fps
	{0x0315 , 0x16},
	{0x0316 , 0x26},
	{0x0317 , 0x02},
	{0x0318 , 0x08},
	{0x0319 , 0x0C},
	{0x031A , 0x81},
	{0x031B , 0x00},
	{0x031C , 0x1D},
	{0x031D , 0x00},
	{0x031E , 0xFD},
	{0x031F , 0x00},
	{0x0320 , 0xE1},
	{0x0321 , 0x1A},
	{0x0322 , 0xDE},
	{0x0323 , 0x11},
	{0x0324 , 0x1A},
	{0x0325 , 0xEE},
	{0x0326 , 0x50},
	{0x0327 , 0x18},
	{0x0328 , 0x25},
	{0x0329 , 0x37},
	{0x032A , 0x24},
	{0x032B , 0x32},
	{0x032C , 0xA9},
	{0x032D , 0x32},
	{0x032E , 0xDB},
	{0x032F , 0x42},
	{0x0330 , 0x30},
	{0x0331 , 0x55},
	{0x0332 , 0x7F},
	{0x0333 , 0x15},
	{0x0334 , 0x99},
	{0x0335 , 0x20},
	{0x0336 , 0xFF},
	{0x0337 , 0x20},
	{0x0338 , 0x46},
	{0x0339 , 0x04},
	{0x033A , 0x04},
	{0x033B , 0x00},
	{0x033C , 0x00},
	{0x033D , 0x00},
	{0x033E , 0x03},
	{0x033F , 0x28},
	{0x0340 , 0x02},
	{0x0341 , 0x60},
	{0x0400 , 0xE8},
	{0x0401 , 0x40},
	{0x0402 , 0x00},
	{0x0403 , 0x00},
	{0x0404 , 0xF8},
	{0x0405 , 0x08},
	{0x0406 , 0x08},
	{0x0407 , 0x89},
	{0x0408 , 0x44},
	{0x0409 , 0x1F},
	{0x040A , 0x00},
	{0x040B , 0x33},
	{0x040C , 0xE0},
	{0x040D , 0x00},
	{0x040E , 0x00},
	{0x040F , 0x00},
	{0x0410 , 0x00},
	{0x0411 , 0x00},
	{0x0412 , 0x01},
	{0x0413 , 0x00},
	{0x0414 , 0x04},
	{0x0415 , 0x04},
	{0x0416 , 0x01},
	{0x0417 , 0x00},
	{0x0418 , 0x02},
	{0x0419 , 0x02},
	{0x041A , 0x01},
	{0x041B , 0x01},
	{0x041C , 0x04},
	{0x041D , 0x04},
	{0x041E , 0x00},
	{0x041F , 0x08},
	{0x0420 , 0x40},
	{0x0421 , 0x40},
	{0x0422 , 0x40},
	{0x0423 , 0x35},
	{0x0424 , 0x40},
	{0x0425 , 0x40},
	{0x0426 , 0x42},
	{0x0427 , 0x3B},
	{0x0428 , 0x22},
	{0x0429 , 0x22},
	{0x042A , 0x21},
	{0x042B , 0x20},
	{0x042C , 0x22},
	{0x042D , 0x22},
	{0x042E , 0x1F},
	{0x042F , 0x1C},
	{0x0430 , 0x24},
	{0x0431 , 0x24},
	{0x0432 , 0x25},
	{0x0433 , 0x00},
	{0x0434 , 0x00},
	{0x0435 , 0x00},
	{0x0436 , 0x00},
	{0x0437 , 0x00},
	{0x0438 , 0x18},
	{0x0439 , 0x18},
	{0x043A , 0x00},
	{0x043B , 0x00},
	{0x043C , 0x18},
	{0x043D , 0x18},
	{0x043E , 0x10},
	{0x043F , 0x10},
	{0x0440 , 0x00},
	{0x0441 , 0x4b},
	{0x0442 , 0x00},
	{0x0443 , 0x00},
	{0x0444 , 0x28},
	{0x0445 , 0x00},
	{0x0446 , 0x00},
	{0x0447 , 0x00},
	{0x0448 , 0x00},
	{0x0449 , 0x00},
	{0x044A , 0x00},
	{0x044D , 0xE0},
	{0x044E , 0x05},
	{0x044F , 0x07},
	{0x0450 , 0x00},
	{0x0451 , 0x00},
	{0x0452 , 0x00},
	{0x0453 , 0x00},
	{0x0454 , 0x00},
	{0x0455 , 0x00},
	{0x0456 , 0x00},
	{0x0457 , 0x00},
	{0x0458 , 0x00},
	{0x0459 , 0x00},
	{0x045A , 0x00},
	{0x045B , 0x00},
	{0x045C , 0x00},
	{0x045D , 0x00},
	{0x045E , 0x00},
	{0x045F , 0x00},
	{0x0460 , 0x80},
	{0x0461 , 0x00},
	{0x0462 , 0x00},
	{0x0463 , 0x00},
	{0x0464 , 0x00},
	{0x0465 , 0x00},
	{0x0466 , 0x25},
	{0x0467 , 0x25},
	{0x0468 , 0x40},
	{0x0469 , 0x30},
	{0x046A , 0x28},
	{0x046B , 0x45},
	{0x046C , 0x3A},
	{0x046D , 0x33},
	{0x046E , 0x2F},
	{0x046F , 0x49},
	{0x0470 , 0x46},
	{0x0471 , 0x3D},
	{0x0472 , 0x30},
	{0x0473 , 0x3E},
	{0x0474 , 0x3D},
	{0x0475 , 0x32},
	{0x0476 , 0x34},
	{0x0477 , 0x40},
	{0x0600 , 0x00},
	{0x0601 , 0x24},
	{0x0602 , 0x45},
	{0x0603 , 0x0E},
	{0x0604 , 0x14},
	{0x0605 , 0x2F},
	{0x0606 , 0x01},
	{0x0607 , 0x0E},
	{0x0608 , 0x0E},
	{0x0609 , 0x37},
	{0x060A , 0x18},
	{0x060B , 0xA0},
	{0x060C , 0x20},
	{0x060D , 0x07},
	{0x060E , 0x47},
	{0x060F , 0x90},
	{0x0610 , 0x06},
	{0x0611 , 0x0C},
	{0x0612 , 0x28},
	{0x0613 , 0x13},
	{0x0614 , 0x0B},
	{0x0615 , 0x10},
	{0x0616 , 0x14},
	{0x0617 , 0x19},
	{0x0618 , 0x52},
	{0x0619 , 0xA0},
	{0x061A , 0x11},
	{0x061B , 0x33},
	{0x061C , 0x56},
	{0x061D , 0x20},
	{0x061E , 0x28},
	{0x061F , 0x2B},
	{0x0620 , 0x22},
	{0x0621 , 0x11},
	{0x0622 , 0x75},
	{0x0623 , 0x49},
	{0x0624 , 0x6E},
	{0x0625 , 0x80},
	{0x0626 , 0x02},
	{0x0627 , 0x0C},
	{0x0628 , 0x51},
	{0x0629 , 0x25},
	{0x062A , 0x01},
	{0x062B , 0x3D},
	{0x062C , 0x04},
	{0x062D , 0x01},
	{0x062E , 0x0C},
	{0x062F , 0x2C},
	{0x0630 , 0x0D},
	{0x0631 , 0x14},
	{0x0632 , 0x12},
	{0x0633 , 0x34},
	{0x0634 , 0x00},
	{0x0635 , 0x00},
	{0x0636 , 0x00},
	{0x0637 , 0xB1},
	{0x0638 , 0x22},
	{0x0639 , 0x32},
	{0x063A , 0x0E},
	{0x063B , 0x18},
	{0x063C , 0x88},
	{0x0640 , 0xB2},
	{0x0641 , 0xC0},
	{0x0642 , 0x01},
	{0x0643 , 0x26},
	{0x0644 , 0x13},
	{0x0645 , 0x88},
	{0x0646 , 0x64},
	{0x0647 , 0x00},
	{0x0681 , 0x1B},
	{0x0682 , 0xA0},
	{0x0683 , 0x28},
	{0x0684 , 0x00},
	{0x0685 , 0xB0},
	{0x0686 , 0x6F},
	{0x0687 , 0x33},
	{0x0688 , 0x1F},
	{0x0689 , 0x44},
	{0x068A , 0xA8},
	{0x068B , 0x44},
	{0x068C , 0x08},
	{0x068D , 0x08},
	{0x068E , 0x00},
	{0x068F , 0x00},
	{0x0690 , 0x01},
	{0x0691 , 0x00},
	{0x0692 , 0x01},
	{0x0693 , 0x00},
	{0x0694 , 0x00},
	{0x0695 , 0x00},
	{0x0696 , 0x00},
	{0x0697 , 0x00},
	{0x0698 , 0x2A},
	{0x0699 , 0x80},
	{0x069A , 0x1F},
	{0x069B , 0x00},
	{0x0F00 , 0x00},
	{0x0F01 , 0x00},
	{0x0100 , 0x01},
	{0x0102 , 0x02},
	{0x0104 , 0x03},

	//{0x0101 , 0x03},
	{0x0101 , 0x00},
#endif
};

static SENSOR_REG_T GT2005_YUV_1600x1200[]=
{
	//Resoltion Setting : 1600*1200
	{0x0109, 0x01},
	{0x010A, 0x00},
	{0x010B, 0x00},
	{0x0110, 0x06},
	{0x0111, 0x40},
	{0x0112, 0x04},
	{0x0113, 0xb0},
};

//__align(4) const SENSOR_REG_T GT2005_YUV_1280X960[] =
static SENSOR_REG_T GT2005_YUV_1280X960[] =
{
    {0x0109 , 0x01},
    {0x010A , 0x00},
    {0x010B , 0x0B},
    {0x0110 , 0x05},
    {0x0111 , 0x00},
    {0x0112 , 0x03},
    {0x0113 , 0xC0},

  //PLL Setting 30FPS Under 24MHz PCLK
    {0x0116 , 0x02},
    {0x0118 , 0x53},//0x40
    {0x0119 , 0x01},
    {0x011A , 0x04},
    {0x011B , 0x00},//PCLK DIV
    {0x011D , 0x02},
    {0x011E , 0x00},//fifo delay

    //banding
    {0x0315 , 0x16},
    {0x0312 , 0x08},
    {0x0313 , 0x36},//0x35
    {0x0314 , 0xc1},//0x36
    {0x0300 , 0x81},
};


static SENSOR_REG_T GT2005_YUV_1280x720[]=
{
	//Resolution Setting : 1280*720
	{0x0109, 0x00},
	{0x010A, 0x00},
	{0x010B, 0x03},
	{0x0110, 0x05},
	{0x0111, 0x00},
	{0x0112, 0x02},
	{0x0113, 0xD0},

	//PLL in15PS Uner 24MHz PCLK
	{0x0116, 0x02},
	{0x0118, 0x69},
	{0x0119, 0x01},
	{0x011a, 0x04},
	{0x011B, 0x00},//PCLK DIV

	//banding
	{0x0315, 0x16},
	{0x0312, 0x08},
	{0x0313, 0x38},
	{0x0314, 0x8B},
	{0x0300, 0x81},   // alc on  2012-2-17
};

static SENSOR_REG_T GT2005_YUV_800x600[] = {
	//Resooneting :800*600
	{0x0109, 0x00},
	{0x010A, 0x04},
	{0x010B, 0x03},
	{0x0110, 0x03},
	{0x0111, 0x20},
	{0x0112, 0x02},
	{0x0113, 0x58},

	{0x0300, 0x81},
};


static SENSOR_REG_T GT2005_YUV_640x480[]=
{
	{0x0109 , 0x00},
	{0x010A , 0x04},
	{0x010B , 0x0B},
	{0x0110 , 0x02},
	{0x0111 , 0x80},
	{0x0112 , 0x01},
	{0x0113 , 0xE0},

	//PLL Setting 30FPS Under 24MHz PCLK
	{0x0116 , 0x02},
	{0x0118 , 0x40},//0x40
	{0x0119 , 0x01},
	{0x011a , 0x04},
	{0x011B , 0x00},//PCLK DIV

	//banding
	{0x0315 , 0x16},
	{0x0312 , 0x98},
	{0x0313 , 0x35},//0x35
	{0x0314 , 0x36},//0x36
	{0x0300 , 0x81},
};

static SENSOR_REG_TAB_INFO_T s_GT2005_resolution_Tab_YUV[]=
{
	// COMMON INIT
	{ADDR_AND_LEN_OF_ARRAY(GT2005_YUV_COMMON), 0, 0, 24, SENSOR_IMAGE_FORMAT_YUV422},

	// YUV422 PREVIEW 1
	{ADDR_AND_LEN_OF_ARRAY(GT2005_YUV_640x480), 640, 480, 24, SENSOR_IMAGE_FORMAT_YUV422},
	{ADDR_AND_LEN_OF_ARRAY(GT2005_YUV_1280X960), 1280, 960, 24, SENSOR_IMAGE_FORMAT_YUV422},
	{ADDR_AND_LEN_OF_ARRAY(GT2005_YUV_1600x1200), 1600, 1200, 24, SENSOR_IMAGE_FORMAT_YUV422},
	{PNULL, 0, 0, 0, 0, 0},
//	{PNULL, 0, 0, 0, 0, 0},
//	{PNULL, 0, 0, 0, 0, 0},

	// YUV422 PREVIEW 2
	{PNULL, 0, 0, 0, 0, 0},
	{PNULL, 0, 0, 0, 0, 0},
	{PNULL, 0, 0, 0, 0, 0},
	{PNULL, 0, 0, 0, 0, 0}
};

LOCAL SENSOR_VIDEO_INFO_T s_GT2005_video_info[] = {
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
	{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL}
};

static SENSOR_IOCTL_FUNC_TAB_T s_GT2005_ioctl_func_tab =
{
	// Internal
	PNULL,
	GT2005_PowerOn,
	PNULL,
	GT2005_Identify,

	PNULL,// write register
	PNULL,// read  register
	PNULL, //set_sensor_flip
	PNULL,

	// External
	PNULL,//set_GT2005_ae_enable,
	PNULL,
	PNULL,

	set_brightness,
	set_contrast,
	PNULL,//set_sharpness,
	PNULL,//set_saturation,

	PNULL,//set_preview_mode,
	set_image_effect,

	GT2005_BeforeSnapshot,
	GT2005_After_Snapshot,

	PNULL,

	PNULL,//read_ev_value,
	PNULL,//write_ev_value,
	PNULL,//read_gain_value,
	PNULL,//write_gain_value,
	PNULL,//read_gain_scale,
	PNULL,//set_frame_rate,
	PNULL,
	PNULL,
	set_GT2005_awb,
	PNULL,
	PNULL,
	set_GT2005_ev,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,//set_GT2005_anti_flicker,
	PNULL,//set_GT2005_video_mode,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
};

SENSOR_INFO_T g_GT2005_yuv_info =
{
	GT2005_I2C_ADDR_W,				//salve i2c write address
	GT2005_I2C_ADDR_R,				//salve i2c read address

	SENSOR_I2C_REG_16BIT | SENSOR_I2C_FREQ_400,						//bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
							//bit2: 0: i2c register addr  is 8 bit, 1: i2c register addr  is 16 bit
							//other bit: reseved
	SENSOR_HW_SIGNAL_PCLK_N|\
	SENSOR_HW_SIGNAL_VSYNC_P|\
	SENSOR_HW_SIGNAL_HSYNC_P,			//bit0: 0:negative; 1:positive -> polarily of pixel clock
							//bit2: 0:negative; 1:positive -> polarily of horizontal synchronization signal
							//bit4: 0:negative; 1:positive -> polarily of vertical synchronization signal
							//other bit: reseved

	// preview mode
	SENSOR_ENVIROMENT_NORMAL|\
	SENSOR_ENVIROMENT_NIGHT|\
	SENSOR_ENVIROMENT_SUNNY,

	// image effect
	SENSOR_IMAGE_EFFECT_NORMAL|\
	SENSOR_IMAGE_EFFECT_BLACKWHITE|\
	SENSOR_IMAGE_EFFECT_RED|\
	SENSOR_IMAGE_EFFECT_GREEN|\
	SENSOR_IMAGE_EFFECT_BLUE|\
	SENSOR_IMAGE_EFFECT_YELLOW|\
	SENSOR_IMAGE_EFFECT_NEGATIVE|\
	SENSOR_IMAGE_EFFECT_CANVAS,

	// while balance mode
	0,

	7,						//bit[0:7]: count of step in brightness, contrast, sharpness, saturation
							//bit[8:31] reseved

	SENSOR_LOW_PULSE_RESET,			//reset pulse level
	100,						//reset pulse width(ms)

	SENSOR_LOW_LEVEL_PWDN,				//power donw pulse level

	2,						//count of identify code
	{{0x0000, 0x51},					//supply two code to identify sensor.
	{0x0001, 0x38}},					//for Example: index = 0-> Device id, index = 1 -> version id

	SENSOR_AVDD_2800MV,				//voltage of avdd

	1600,						//max width of source image
	1200,						//max height of source image
	"GT2005",					//name of sensor

	SENSOR_IMAGE_FORMAT_YUV422,			//define in SENSOR_IMAGE_FORMAT_E enum,
							//if set to SENSOR_IMAGE_FORMAT_MAX here, image format depent on SENSOR_REG_TAB_INFO_T
	SENSOR_IMAGE_PATTERN_YUV422_YUYV,		//pattern of input image form sensor;

	s_GT2005_resolution_Tab_YUV,			//point to resolution table information structure
	&s_GT2005_ioctl_func_tab,				//point to ioctl function table

	PNULL,						//information and table about Rawrgb sensor
	PNULL,						//extend information about sensor
	SENSOR_AVDD_1800MV,				//iovdd
	SENSOR_AVDD_1500MV,				//dvdd
	3,						//skip frame num before preview
	1,						//skip frame num before capture
	0,						//deci frame num during preview
	0,						//deci frame num during video preview
	0,						//threshold enable(only analog TV)
	0,						//atv output mode 0 fix mode 1 auto mode
	0,						//atv output start postion
	0,						//atv output end postion
	0,
	{SENSOR_INTERFACE_TYPE_CCIR601, 8, 16, 1},
	s_GT2005_video_info,
	4,						//skip frame num while change setting
	48,						//horizontal view angle
	48,						//vertical view angle
};

static void GT2005_WriteReg(uint16_t  subaddr, uint8_t data )
{
	Sensor_WriteReg(subaddr, data);
}

static uint8_t GT2005_ReadReg( uint8_t subaddr)
{
	uint8_t value = 0;

	value = Sensor_ReadReg( subaddr);

	return value;
}

static uint32_t GT2005_PowerOn(uint32_t power_on)
{
	SENSOR_AVDD_VAL_E dvdd_val = g_GT2005_yuv_info.dvdd_val;
	SENSOR_AVDD_VAL_E avdd_val = g_GT2005_yuv_info.avdd_val;
	SENSOR_AVDD_VAL_E iovdd_val = g_GT2005_yuv_info.iovdd_val;
	BOOLEAN power_down = g_GT2005_yuv_info.power_down_level;
	BOOLEAN reset_level = g_GT2005_yuv_info.reset_pulse_level;

	if (SENSOR_TRUE == power_on) {
		Sensor_PowerDown(power_down);
		Sensor_SetResetLevel(reset_level);
		usleep(1000);
		Sensor_SetVoltage(dvdd_val, avdd_val, iovdd_val);
		usleep(2000);
		Sensor_SetResetLevel(!reset_level);
		usleep(2000);
		Sensor_SetMCLK(SENSOR_DEFALUT_MCLK);
		usleep(2000);
		Sensor_PowerDown(!power_down);
		usleep(10*1000);
	} else {
		Sensor_PowerDown(power_down);
		usleep(1000);
		Sensor_SetResetLevel(reset_level);
		usleep(1000);
		Sensor_SetMCLK(SENSOR_DISABLE_MCLK);
		usleep(5000);
		Sensor_SetVoltage(SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED,
				SENSOR_AVDD_CLOSED);
	}

	SENSOR_PRINT("(1:on, 0:off): %d", power_on);
	return (uint32_t)SENSOR_SUCCESS;
}

static uint32_t GT2005_Identify(uint32_t param)
{
#define GT2005_PID_ADDR1 0x0000
#define GT2005_PID_ADDR2 0x0001
#define GT2005_SENSOR_ID 0x5138

	uint16_t sensor_id = 0;
	uint8_t pid_value = 0;
	uint8_t ver_value = 0;
	int i;
	BOOLEAN ret_value = SENSOR_FAIL;

	for (i=0; i<3; i++) {
		sensor_id = Sensor_ReadReg(GT2005_PID_ADDR1) << 8;
		sensor_id |= Sensor_ReadReg(GT2005_PID_ADDR2);
		SENSOR_PRINT("%s sensor_id is %x\n", __func__, sensor_id);

		if (sensor_id == GT2005_SENSOR_ID) {
			SENSOR_PRINT("the main sensor is GT2005\n");
			return SENSOR_SUCCESS;
		}
	}

	return ret_value;
}

static uint32_t set_GT2005_ae_enable(uint32_t enable)
{
	uint32_t temp_AE_reg =0;

	if (enable == 1) {
		temp_AE_reg = Sensor_ReadReg(0xb6);
		Sensor_WriteReg(0xb6, temp_AE_reg| 0x01);
	} else {
		temp_AE_reg = Sensor_ReadReg(0xb6);
		Sensor_WriteReg(0xb6, temp_AE_reg&~0x01);
	}

	return 0;
}


static void GT2005_set_shutter()
{
	uint32_t shutter = 0 ;

	Sensor_WriteReg(0xfe,0x00);
	Sensor_WriteReg(0xb6,0x00);
	shutter = (Sensor_ReadReg(0x03)<<8 )|( Sensor_ReadReg(0x04));

	shutter = shutter / 2 ;

	if  (shutter < 1)
		shutter = 1;

	Sensor_WriteReg(0x03, (shutter >> 8)&0xff);
	Sensor_WriteReg(0x04, shutter&0xff);
}

static SENSOR_REG_T GT2005_brightness_tab[][2]=
{
	{{0x0201 , 0xa0},{0xff , 0xff}},
	{{0x0201 , 0xc0},{0xff , 0xff}},
	{{0x0201 , 0xd0},{0xff , 0xff}},
	{{0x0201 , 0x00},{0xff , 0xff}},
	{{0x0201 , 0x20},{0xff , 0xff}},
	{{0x0201 , 0x40},{0xff , 0xff}},
	{{0x0201 , 0x60},{0xff , 0xff}}
};

static uint32_t set_brightness(uint32_t level)
{
	uint16_t i;
	SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GT2005_brightness_tab[level];

	if (level > 6)
		return 0;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) && (0xFF != sensor_reg_ptr[i].reg_value); i++)
		GT2005_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);

	return 0;
}


static SENSOR_REG_T GT2005_ev_tab[][4]=
{
	{{0x0300 , 0x81},{0x0301 , 0x50},{0xff , 0xff}},
	{{0x0300 , 0x81},{0x0301 , 0x60},{0xff , 0xff}},
	{{0x0300 , 0x81},{0x0301 , 0x70},{0xff , 0xff}},
	{{0x0300 , 0x81},{0x0301 , 0x80},{0xff , 0xff}},
	{{0x0300 , 0x81},{0x0301 , 0x90},{0xff , 0xff}},
	{{0x0300 , 0x81},{0x0301 , 0xa0},{0xff , 0xff}},
	{{0x0300 , 0x81},{0x0301 , 0xb0},{0xff , 0xff}}
};

static uint32_t set_GT2005_ev(uint32_t level)
{
	uint16_t i;
	SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GT2005_ev_tab[level];

	if (level > 6)
		return 0;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) ||(0xFF != sensor_reg_ptr[i].reg_value) ; i++)
		GT2005_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);

	return 0;
}

static uint32_t set_GT2005_anti_flicker(uint32_t param )
{
	//PLL Setting 15FPS Under 19.5MHz PCLK
	GT2005_WriteReg(0x0116 , 0x02);
	GT2005_WriteReg(0x0118 , 0x40);
	GT2005_WriteReg(0x0119 , 0x01);
	GT2005_WriteReg(0x011a , 0x04);
	GT2005_WriteReg(0x011B , 0x00);

	GT2005_WriteReg(0x0313 , 0x35);
	GT2005_WriteReg(0x0314 , 0x36);

	switch(param)
	{
		case FLICKER_50HZ:
		GT2005_WriteReg(0x0315 , 0x16);
		break;

		case FLICKER_60HZ:
		GT2005_WriteReg(0x0315 , 0x56);
		break;

		default:
		return 0;
	}

	SENSOR_PRINT("set_GT2005_anti_flicker-mode=%d",param);
//	usleep(200*1000);

	return 0;
}

static SENSOR_REG_T GT2005_video_mode_nor_tab[][8]=
{
	// normal mode
	{{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}},
	//vodeo mode     10fps Under 13MHz MCLK
	{{0x0116 , 0x02},{0x0118 , 0x40},{0x0119 , 0x01},{0x011a , 0x04},{0x011B , 0x00},{0x0313 , 0x35}, {0x0314 , 0x36}, {0xff , 0xff}},
	// UPCC  mode	  10fps Under 13MHz MCLK
	{{0x0116 , 0x02},{0x0118 , 0x40},{0x0119 , 0x01},{0x011a , 0x04},{0x011B , 0x00},{0x0313 , 0x35}, {0x0314 , 0x36}, {0xff , 0xff}}
};

static uint32_t set_GT2005_video_mode(uint32_t mode)
{
	uint16_t i;
	SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GT2005_video_mode_nor_tab[mode];
	SENSOR_PRINT(" E set_GT2005_video_mode=%d",mode);

	if(mode>2) return 0;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) ||(0xFF != sensor_reg_ptr[i].reg_value) ; i++)
		GT2005_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);

	SENSOR_PRINT("X set_GT2005_video_mode=%d",mode);

	return 0;
}

static SENSOR_REG_T GT2005_awb_tab[][10] =
{
        //Auto
        {
                {0x031a , 0x81},
                {0x0320 , 0x24},
                {0x0321 , 0x14},
                {0x0322 , 0x1a},
                {0x0323 , 0x24},
                {0x0441 , 0x4B},
                {0x0442 , 0x00},
                {0x0443 , 0x00},
                {0x0444 , 0x31},
                {0xff , 0xff}
        },
        //Office
        {
                {0x0320 , 0x02},
                {0x0321 , 0x02},
                {0x0322 , 0x02},
                {0x0323 , 0x02},
                {0x0441 , 0x20},
                {0x0442 , 0x20},
                {0x0443 , 0x20},
                {0x0444 , 0x70},
                {0xff , 0xff},
                {0xff , 0xff}
        },
        //U30  //not use
        {
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff}
        },
        //CWF  //not use
        {
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff},
                {0xff , 0xff}
        },
        //HOME
        {
                {0x0320 , 0x02},
                {0x0321 , 0x02},
                {0x0322 , 0x02},
                {0x0323 , 0x02},
                {0x0441 , 0x00},
                {0x0442 , 0x00},
                {0x0443 , 0x00},
                {0x0444 , 0x20},
                {0xff, 0xff},
                {0xff, 0xff}
        },

        //SUN:
        {
                {0x0320 , 0x02},
                {0x0321 , 0x02},
                {0x0322 , 0x02},
                {0x0323 , 0x02},
                {0x0441 , 0x40},
                {0x0442 , 0x10},
                {0x0443 , 0x10},
                {0x0444 , 0x20},
                {0xff , 0xff},
                {0xff , 0xff}
        },

        //CLOUD:
        {
                {0x0320 , 0x02},
                {0x0321 , 0x02},
                {0x0322 , 0x02},
                {0x0323 , 0x02},
                {0x0441 , 0xc0},
                {0x0442 , 0x20},
                {0x0443 , 0x20},
                {0x0444 , 0x20},
                {0xff , 0xff},
                {0xff , 0xff}
        }
};

static uint32_t set_GT2005_awb(uint32_t mode)
{
	uint8_t awb_en_value;
	uint16_t i;

	SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GT2005_awb_tab[mode];

	if (mode > 6)
		return 0;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value); i++)
		GT2005_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);

	return 0;
}

static SENSOR_REG_T GT2005_contrast_tab[][2]=
{
        {{0x0200 , 0xa0},{0xff , 0xff}},
        {{0x0200 , 0xc0},{0xff , 0xff}},
        {{0x0200 , 0xd0},{0xff , 0xff}},
        {{0x0200 , 0x00},{0xff , 0xff}},
        {{0x0200 , 0x20},{0xff , 0xff}},
        {{0x0200 , 0x40},{0xff , 0xff}},
        {{0x0200 , 0x60},{0xff , 0xff}}
};

static uint32_t set_contrast(uint32_t level)
{
	uint16_t i;
	SENSOR_REG_T* sensor_reg_ptr;

	sensor_reg_ptr = (SENSOR_REG_T*)GT2005_contrast_tab[level];

	if (level > 6)
		return 0;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) && (0xFF != sensor_reg_ptr[i].reg_value); i++)
		GT2005_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);

	return 0;
}

static uint32_t set_sharpness(uint32_t level)
{
	return 0;
}

static uint32_t set_saturation(uint32_t level)
{
	return 0;
}

static uint32_t set_preview_mode(uint32_t preview_mode)
{
	SENSOR_PRINT("set_preview_mode: preview_mode = %d\n", preview_mode);

	return 0;
}

static SENSOR_REG_T GT2005_image_effect_tab[][4] =
{
	//Normal
	{{0x0115 , 0x00},{0xff , 0xff},{0xff , 0xff},{0xff , 0xff}},
	//BLACK&WHITE
	{{0x0115 , 0x06},{0xff , 0xff},{0xff , 0xff},{0xff , 0xff}},
	//RED
	{{0x0115 , 0x0a},{0x026e , 0x70},{0x026f , 0xf0},{0xff , 0xff}},
	//GREEN
	{{0x0115 , 0x0a},{0x026e , 0x20},{0x026f , 0x00},{0xff , 0xff}},
	//BLUE
	{{0x0115 , 0x0a},{0x026e , 0xf0},{0x026f , 0x00},{0xff , 0xff}},
	//YELLOW
	{{0x0115 , 0x0a},{0x026e , 0x00},{0x026f , 0x80},{0xff , 0xff}},
	//NEGATIVE
	{{0x0115 , 0x09},{0xff , 0xff},{0xff , 0xff},{0xff , 0xff}},
	//SEPIA
	{{0x0115 , 0x0a},{0x026e , 0x60},{0x026f , 0xa0},{0xff , 0xff}}
};

static uint32_t set_image_effect(uint32_t effect_type)
{
	uint16_t i;

	SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GT2005_image_effect_tab[effect_type];

	if(effect_type>7) return 0;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) && (0xFF != sensor_reg_ptr[i].reg_value); i++)
		GT2005_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);

	SENSOR_PRINT("set_GT2005_image_effect: effect_type = %d", effect_type);
	return 0;
}

static uint32_t GT2005_After_Snapshot(uint32_t param)
{
	SENSOR_PRINT("GT2005_After_Snapshot param = %x \n",param);

	Sensor_SetMode(param);

	return 0;
}

static uint32_t GT2005_BeforeSnapshot(uint32_t sensor_snapshot_mode)
{
	sensor_snapshot_mode &= 0xffff;
	Sensor_SetMode(sensor_snapshot_mode);
	Sensor_SetMode_WaitDone();

	switch (sensor_snapshot_mode) {
	case SENSOR_MODE_PREVIEW_ONE:
		SENSOR_PRINT("Capture VGA Size");
		break;
	case SENSOR_MODE_SNAPSHOT_ONE_FIRST:
	case SENSOR_MODE_SNAPSHOT_ONE_SECOND:
		SENSOR_PRINT("Capture 1.3M&2M Size");
		break;
	default:
		break;
	}

	SENSOR_PRINT("SENSOR_GT2005: Before Snapshot");

	return 0;

}

static uint32_t read_ev_value(uint32_t value)
{
	return 0;
}

static uint32_t write_ev_value(uint32_t exposure_value)
{
	return 0;
}

static uint32_t read_gain_value(uint32_t value)
{
	return 0;
}

static uint32_t write_gain_value(uint32_t gain_value)
{
	return 0;
}

static uint32_t read_gain_scale(uint32_t value)
{
	return SENSOR_GAIN_SCALE;
}


static uint32_t set_frame_rate(uint32_t param)
{
	return 0;
}
static uint32_t set_sensor_flip(uint32_t param)
{
	return 0;
}