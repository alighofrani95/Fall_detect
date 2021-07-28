/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS},
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include "gc0328.h"
#include "dvp.h"
#include "plic.h"
#include "sleep.h"
#include "printf.h"
#include "cambus.h"

enum
{
    GC0328_RGB_Gamma_m0 = 0,
    GC0328_RGB_Gamma_m1,
    GC0328_RGB_Gamma_m2,
    GC0328_RGB_Gamma_m3,
    GC0328_RGB_Gamma_m4,
    GC0328_RGB_Gamma_m5,
    GC0328_RGB_Gamma_m6,
    GC0328_RGB_Gamma_night,
	GC0328_RGB_Gamma_cap,
	GC0328_RGB_Gamma_test
};

enum
{
	GC0328_Y_Gamma_default,
	GC0328_Y_Gamma_10,
    GC0328_Y_Gamma_09,
    GC0328_Y_Gamma_08,
    GC0328_Y_Gamma_07,
    GC0328_Y_Gamma_06,
    GC0328_Y_Gamma_05,

};

#define GC0328_RGB_Gamma GC0328_RGB_Gamma_test
#define GC0328_Y_Gamma  GC0328_Y_Gamma_default

/** The default register settings**/
uint8_t sensor_default_regs[][2] = {
	{0xfe , 0x80},  //reset
	{0xfe , 0x80},  //reset
	{0xfc , 0x16},  //set digital clock
	{0xfc , 0x16},  //set digital clock
	{0xfc , 0x16},  //set digital clock
	{0xfc , 0x16},  //set digital clock

	{0xfe , 0x00},  //select page0
	{0x4f , 0x00},  //AEC disable
	{0x42 , 0x00},  //ABW disable
	{0x03 , 0x00},  //clear Exposure time MSB[11:8]
	{0x04 , 0xc0},  //clear Exposure time LSB[7:0]
	{0x77 , 0x62},  //set AWB red gain
	{0x78 , 0x40},  //set AWB green gain 
	{0x79 , 0x4d},  //set AWB blue gain

	{0x05, 0x00}, 	
	{0x06, 0x80},
	{0x07, 0x00},
	{0x08, 0x10},

	{0xfe, 0x01},
	{0x29, 0x00},
	{0x2a, 0x96},

	{0x2b, 0x02},
	{0x2c, 0x00},
	{0x2d, 0x02},
	{0x2e, 0x00},
	{0x2f, 0x02},
	{0x30, 0x00},
	{0x31, 0x02},
	{0x32, 0x00},
	{0xfe, 0x00},

	{0xfe , 0x01},  
	{0x4f , 0x00},  
	{0x4c , 0x01},  
	{0xfe , 0x00},  
	//////////////////////////////
	///////////AWB///////////
	////////////////////////////////
	{0xfe , 0x01},
	{0x51 , 0x80},
	{0x52 , 0x12},
	{0x53 , 0x80},
	{0x54 , 0x60},
	{0x55 , 0x01},
	{0x56 , 0x06},
	{0x5b , 0x02},
	{0x61 , 0xdc},
	{0x62 , 0xdc},
	{0x7c , 0x71},
	{0x7d , 0x00},
	{0x76 , 0x00},
	{0x79 , 0x20},
	{0x7b , 0x00},  
	{0x70 , 0xFF},
	{0x71 , 0x00},
	{0x72 , 0x10},
	{0x73 , 0x40},
	{0x74 , 0x40},

	{0x50 , 0x00},
	{0xfe , 0x01},  
	{0x4f , 0x00},  
	{0x4c , 0x01},  
	{0x4f , 0x00},  
	{0x4f , 0x00},  
	{0x4f , 0x00},  
	{0x4d , 0x36},  
	{0x4e , 0x02},  
	{0x4e , 0x02},  
	{0x4d , 0x44},  
	{0x4e , 0x02},
	{0x4e , 0x02},
	{0x4e , 0x02},  
	{0x4e , 0x02},  
	{0x4d , 0x53},  
	{0x4e , 0x08},  
	{0x4e , 0x08},  
	{0x4e , 0x02},  
	{0x4d , 0x63},  
	{0x4e , 0x08},  
	{0x4e , 0x08},  
	{0x4d , 0x73},  
	{0x4e , 0x20},  
	{0x4d , 0x83},  
	{0x4e , 0x20},  
	{0x4f , 0x01},  

	{0x50 , 0x88},
	{0xfe , 0x00},  

	////////////////////////////////////////////////
	////////////     BLK      //////////////////////
	////////////////////////////////////////////////
	{0x27 , 0x00},  
	{0x2a , 0x40},  
	{0x2b , 0x40},  
	{0x2c , 0x40},  
	{0x2d , 0x40},  


	//////////////////////////////////////////////
	////////// page  0    ////////////////////////
	//////////////////////////////////////////////
	{0xfe , 0x00},  
	{0x0d , 0x01},  
	{0x0e , 0xe8},  
	{0x0f , 0x02},  
	{0x10 , 0x88},  
	{0x09 , 0x00},  
	{0x0a , 0x00},  
	{0x0b , 0x00},  
	{0x0c , 0x00},  
	{0x16 , 0x00},
	{0x17 , 0x16},//0x14=0001 0100 高位在前，第0位是mirror，第1位是flip,0x16=0001 0110
	{0x18 , 0x0e},  
	{0x19 , 0x06},  

	{0x1b , 0x48},  
	{0x1f , 0xC8},  
	{0x20 , 0x01},  
	{0x21 , 0x78},  
	{0x22 , 0xb0},  
	{0x23 , 0x04}, 
	{0x24 , 0x11},  //output pin abilityof driver config
	{0x26 , 0x00},  


	//crop mode | It should be disable
	{0x50 , 0x01}, 
		{0x51, 0x00},
		{0x52, 0x00},
		{0x53, 0x00},
		{0x54, 0x00},
		{0x55, 0x00},
		{0x56, 0xf0},
		{0x57, 0x01},
		{0x58, 0x40},
		{0x59, 0x22},
		{0x5a, 0x03},
		{0x5b, 0x00},
		{0x5c, 0x00},
		{0x5d, 0x00},
		{0x5e, 0x00},
		{0x5f, 0x00},
		{0x60, 0x00},
		{0x61, 0x00},
		{0x62, 0x00},

	//global gain for range
	{0x70 , 0x85},  

	////////////////////////////////////////////////
	////////////     block enable      /////////////
	////////////////////////////////////////////////
	{0x40 , 0x7f},
	{0x41 , 0x26},
	{0x42 , 0xff},
	{0x45 , 0x00},  
	{0x44 , 0x06},
	{0x46 , 0x03},

	{0x4b , 0x01},  
	{0x50 , 0x01},

	//DN & EEINTP
	{0x7e , 0x0a}, 
	{0x7f , 0x03}, 
	{0x80 , 0x27}, 
	{0x81 , 0x15}, 
	{0x82 , 0x90},
	{0x83 , 0x02},
	{0x84 , 0x23}, 
	{0x90 , 0x2c}, 
	{0x92 , 0x02},
	{0x94 , 0x02},
	{0x95 , 0x35},  

	///////YCP
	{0xd1 , 0x32},
	{0xd2 , 0x32},
	{0xdd , 0x18},
	{0xde , 0x32},
	{0xe4 , 0x88},
	{0xe5 , 0x40},  
	{0xd7 , 0x0e},  

	//ABB

	/////////////////////////////
	//////////////// GAMMA //////
	/////////////////////////////

	//GC0328_RGB_Gamma
	#if (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m0)
		{0xfe, 0x00},
		{0xBF, 0x0E},
		{0xc0, 0x1C},
		{0xc1, 0x34},
		{0xc2, 0x48},
		{0xc3, 0x5A},
		{0xc4, 0x6B},
		{0xc5, 0x7B},
		{0xc6, 0x95},
		{0xc7, 0xAB},
		{0xc8, 0xBF},
		{0xc9, 0xCE},
		{0xcA, 0xD9},
		{0xcB, 0xE4},
		{0xcC, 0xEC},
		{0xcD, 0xF7},
		{0xcE, 0xFD},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m1)
		//smallest gamma curve
		{0xfe, 0x00},
		{0xbf, 0x06},
		{0xc0, 0x12},
		{0xc1, 0x22},
		{0xc2, 0x35},
		{0xc3, 0x4b},
		{0xc4, 0x5f},
		{0xc5, 0x72},
		{0xc6, 0x8d},
		{0xc7, 0xa4},
		{0xc8, 0xb8},
		{0xc9, 0xc8},
		{0xca, 0xd4},
		{0xcb, 0xde},
		{0xcc, 0xe6},
		{0xcd, 0xf1},
		{0xce, 0xf8},
		{0xcf, 0xfd},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m2)
		{0xfe , 0x00},
		{0xBF, 0x08},
		{0xc0, 0x0F},
		{0xc1, 0x21},
		{0xc2, 0x32},
		{0xc3, 0x43},
		{0xc4, 0x50},
		{0xc5, 0x5E},
		{0xc6, 0x78},
		{0xc7, 0x90},
		{0xc8, 0xA6},
		{0xc9, 0xB9},
		{0xcA, 0xC9},
		{0xcB, 0xD6},
		{0xcC, 0xE0},
		{0xcD, 0xEE},
		{0xcE, 0xF8},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m3)
		{0xfe , 0x00},
		{0xBF, 0x0B},
		{0xc0, 0x16},
		{0xc1, 0x29},
		{0xc2, 0x3C},
		{0xc3, 0x4F},
		{0xc4, 0x5F},
		{0xc5, 0x6F},
		{0xc6, 0x8A},
		{0xc7, 0x9F},
		{0xc8, 0xB4},
		{0xc9, 0xC6},
		{0xcA, 0xD3},
		{0xcB, 0xDD},
		{0xcC, 0xE5},
		{0xcD, 0xF1},
		{0xcE, 0xFA},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m4)
		{0xfe , 0x00},
		{0xBF, 0x0E},
		{0xc0, 0x1C},
		{0xc1, 0x34},
		{0xc2, 0x48},
		{0xc3, 0x5A},
		{0xc4, 0x6B},
		{0xc5, 0x7B},
		{0xc6, 0x95},
		{0xc7, 0xAB},
		{0xc8, 0xBF},
		{0xc9, 0xCE},
		{0xcA, 0xD9},
		{0xcB, 0xE4},
		{0xcC, 0xEC},
		{0xcD, 0xF7},
		{0xcE, 0xFD},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m5)
		{0xfe , 0x00},
		{0xBF, 0x10},
		{0xc0, 0x20},
		{0xc1, 0x38},
		{0xc2, 0x4E},
		{0xc3, 0x63},
		{0xc4, 0x76},
		{0xc5, 0x87},
		{0xc6, 0xA2},
		{0xc7, 0xB8},
		{0xc8, 0xCA},
		{0xc9, 0xD8},
		{0xcA, 0xE3},
		{0xcB, 0xEB},
		{0xcC, 0xF0},
		{0xcD, 0xF8},
		{0xcE, 0xFD},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_m6)
		// largest gamma curve
		{0xfe , 0x00},
		{0xBF, 0x14},
		{0xc0, 0x28},
		{0xc1, 0x44},
		{0xc2, 0x5D},
		{0xc3, 0x72},
		{0xc4, 0x86},
		{0xc5, 0x95},
		{0xc6, 0xB1},
		{0xc7, 0xC6},
		{0xc8, 0xD5},
		{0xc9, 0xE1},
		{0xcA, 0xEA},
		{0xcB, 0xF1},
		{0xcC, 0xF5},
		{0xcD, 0xFB},
		{0xcE, 0xFE},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_night)
		//Gamma for night mode
		{0xfe , 0x00},
		{0xBF, 0x0B},
		{0xc0, 0x16},
		{0xc1, 0x29},
		{0xc2, 0x3C},
		{0xc3, 0x4F},
		{0xc4, 0x5F},
		{0xc5, 0x6F},
		{0xc6, 0x8A},
		{0xc7, 0x9F},
		{0xc8, 0xB4},
		{0xc9, 0xC6},
		{0xcA, 0xD3},
		{0xcB, 0xDD},
		{0xcC, 0xE5},
		{0xcD, 0xF1},
		{0xcE, 0xFA},
		{0xcF, 0xFF},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_cap)
		{0xfe , 0x00},
		{0xbf , 0x10},
		{0xc0 , 0x1c},
		{0xc1 , 0x33},
		{0xc2 , 0x48},
		{0xc3 , 0x5a},
		{0xc4 , 0x6b},
		{0xc5 , 0x7b},
		{0xc6 , 0x95},
		{0xc7 , 0xab},
		{0xc8 , 0xbf},
		{0xc9 , 0xcd},
		{0xca , 0xd9},
		{0xcb , 0xe3},
		{0xcc , 0xeb},
		{0xcd , 0xf7},
		{0xce , 0xfd},
		{0xcf , 0xff},
	#elif (GC0328_RGB_Gamma == GC0328_RGB_Gamma_test)
		{0xfe , 0x00},
		{0xbf , 0x00},
		{0xc0 , 0x01},
		{0xc1 , 0x02},
		{0xc2 , 0x03},
		{0xc3 , 0x04},
		{0xc4 , 0x05},
		{0xc5 , 0x06},
		{0xc6 , 0x07},
		{0xc7 , 0x08},
		{0xc8 , 0x09},
		{0xc9 , 0x0a},
		{0xca , 0x0b},
		{0xcb , 0x0c},
		{0xcc , 0x0d},
		{0xcd , 0x0e},
		{0xce , 0x0f},
		{0xcf , 0xff},
	#endif

	///Y gamma
	#if (GC0328_Y_Gamma == GC0328_Y_Gamma_05)
	//0.5
	{0xfe , 0x00},  
	{0x63 , 0x00},  
	{0x64 , 0x49},  
	{0x65 , 0x68},  
	{0x66 , 0x80},  
	{0x67 , 0x93},  
	{0x68 , 0xa5},  
	{0x69 , 0xb5},  
	{0x6a , 0xc3},  
	{0x6b , 0xd1},  
	{0x6c , 0xdd},  
	{0x6d , 0xe9},  
	{0x6e , 0xf5},  
	{0x6f , 0xFF},
	#elif (GC0328_Y_Gamma == GC0328_Y_Gamma_06)
	//0.6
	{0xfe , 0x00},  
	{0x63,0x0 },
	{0x64,0x39},
	{0x65,0x57},
	{0x66,0x6F},
	{0x67,0x84},
	{0x68,0x97},
	{0x69,0xA8},
	{0x6a,0xB9},
	{0x6b,0xC8},
	{0x6c,0xD7},
	{0x6d,0xE5},
	{0x6e,0xF2},
	{0x6f,0xff},

	#elif (GC0328_Y_Gamma == GC0328_Y_Gamma_07)
	//0.7
	{0xfe , 0x00},  
	{0x63,0x0  },
	{0x64,0x2C },
	{0x65,0x49 },
	{0x66,0x61 },
	{0x67,0x76 },
	{0x68,0x8A },
	{0x69,0x9D },
	{0x6a,0xAF },
	{0x6b,0xC0 },
	{0x6c,0xD1 },
	{0x6d,0xE1 },
	{0x6e,0xF0 },
	{0x6f,0xff },
	#elif (GC0328_Y_Gamma == GC0328_Y_Gamma_08)
	//0.8
	{0xfe , 0x00},
	{0x63,0x0 },
	{0x64,0x23},
	{0x65,0x3D},
	{0x66,0x54},
	{0x67,0x6A},
	{0x68,0x7F},
	{0x69,0x93},
	{0x6a,0xA6},
	{0x6b,0xB9},
	{0x6c,0xCB},
	{0x6d,0xDD},
	{0x6e,0xEE},
	{0x6f,0xff},
	#elif (GC0328_Y_Gamma == GC0328_Y_Gamma_09)
	//0.9
	{0xfe , 0x00},
	{0x63,0x0 },
	{0x64,0x1B},
	{0x65,0x33},
	{0x66,0x49},
	{0x67,0x5F},
	{0x68,0x74},
	{0x69,0x89},
	{0x6a,0x9D},
	{0x6b,0xB1},
	{0x6c,0xC5},
	{0x6d,0xD9},
	{0x6e,0xEC},
	{0x6f,0xff},
	#elif (GC0328_Y_Gamma == GC0328_Y_Gamma_10)
	//0.9
	{0xfe , 0x00},
	{0x63,0x0 },
	{0x64,0x15},
	{0x65,0x2A},
	{0x66,0x40},
	{0x67,0x55},
	{0x68,0x6A},
	{0x69,0x80},
	{0x6a,0x95},
	{0x6b,0xAA},
	{0x6c,0xC0},
	{0x6d,0xD5},
	{0x6e,0xEA},
	{0x6f,0xff},
	#elif (GC0328_Y_Gamma == GC0328_Y_Gamma_default)
	{0xfe , 0x00},
	{0x63,0   },
	{0x64,0x10},
	{0x65,0x1c},
	{0x66,0x30},
	{0x67,0x43},
	{0x68,0x54},
	{0x69,0x65},
	{0x6a,0x75},
	{0x6b,0x93},
	{0x6c,0xb0},
	{0x6d,0xcb},
	{0x6e,0xe6},
	{0x6f,0xff},


	#endif


	//////ASDE
	{0xfe , 0x01},  
	{0x18 , 0x02},  
	{0xfe , 0x00},  
	{0x98 , 0x00},  
	{0x9b , 0x20},  
	{0x9c , 0x80},  
	{0xa4 , 0x10},  
	{0xa8 , 0xB0},  
	{0xaa , 0x40},  
	{0xa2 , 0x23},  
	{0xad , 0x01},  

		//////////////////////////////////////////////
		////////// AEC    ////////////////////////
		//////////////////////////////////////////////
	{0xfe , 0x01},
	{0x9c , 0x02},
	{0x08 , 0xa0},
	{0x09 , 0xe8},
				
	{0x10 , 0x00},
	{0x11 , 0x11},
	{0x12 , 0x10},
	{0x13 , 0x80},
	{0x15 , 0xfc},
	{0x18 , 0x03},
	{0x21 , 0xc0},
	{0x22 , 0x60},
	{0x23 , 0x30},
	{0x25 , 0x00},
	{0x24 , 0x14},
				
				
	//////////////////////////////////////
	////////////LSC//////////////////////
	//////////////////////////////////////
	//gc0328 Alight lsc reg setting list
				
	{0xfe , 0x01}, 
	{0xc0 , 0x10}, 
	{0xc1 , 0x0c}, 
	{0xc2 , 0x0a}, 
	{0xc6 , 0x0e}, 
	{0xc7 , 0x0b}, 
	{0xc8 , 0x0a}, 
	{0xba , 0x26}, 
	{0xbb , 0x1c}, 
	{0xbc , 0x1d}, 
	{0xb4 , 0x23}, 
	{0xb5 , 0x1c}, 
	{0xb6 , 0x1a}, 
	{0xc3 , 0x00}, 
	{0xc4 , 0x00},
	{0xc5 , 0x00},
	{0xc9 , 0x00},
	{0xca , 0x00},
	{0xcb , 0x00},
	{0xbd , 0x00},
	{0xbe , 0x00},
	{0xbf , 0x00},
	{0xb7 , 0x07},
	{0xb8 , 0x05},
	{0xb9 , 0x05},
	{0xa8 , 0x07},
	{0xa9 , 0x06},
	{0xaa , 0x00},
	{0xab , 0x04},
	{0xac , 0x00},
	{0xad , 0x02},
	{0xae , 0x0d},
	{0xaf , 0x05},
	{0xb0 , 0x00},
	{0xb1 , 0x07},
	{0xb2 , 0x03},
	{0xb3 , 0x00},
	{0xa4 , 0x00},
	{0xa5 , 0x00},
	{0xa6 , 0x00},
	{0xa7 , 0x00},
	{0xa1 , 0x3c},
	{0xa2 , 0x50},
	{0xfe , 0x00},

	///cct
	{0xB1 , 0x04},  
	{0xB2 , 0xfd},  
	{0xB3 , 0xfc},  
	{0xB4 , 0xf0},  
	{0xB5 , 0x05},  
	{0xB6 , 0xf0},  



	{0xfe , 0x00},
	{0x27 , 0xf7},
	{0x28 , 0x7F},
	{0x29 , 0x20},
	{0x33 , 0x20},
	{0x34 , 0x20},
	{0x35 , 0x20},
	{0x36 , 0x20},
	{0x32 , 0x08},

	{0x47 , 0x00},
	{0x48 , 0x00},
					
	{0xfe , 0x01},
	{0x79 , 0x00},
	{0x7d , 0x00},
	{0x50 , 0x88},
	{0x5b , 0x0c},
	{0x76 , 0x8f},
	{0x80 , 0x70},
	{0x81 , 0x70},
	{0x82 , 0xb0},
	{0x70 , 0xff},
	{0x71 , 0x00},
	{0x72 , 0x28},
	{0x73 , 0x0b},  
	{0x74 , 0x0b},  
					
	{0xfe , 0x00},  
	{0x70 , 0x45},
	{0x4f , 0x01},
	{0xf1 , 0x07},
	{0xf2 , 0x01}, 

	{0x00, 0x00},
};

static const uint8_t qqvga_config[][2] = { //k210 
    {0xfe , 0x00},
    // window
        //windowing mode
	{0x09 , 0x00},
    {0x0a , 0x00},
	{0x0b , 0x00},
	{0x0c , 0x00},
    {0x0d , 0x01},
	{0x0e , 0xe8},
	{0x0f , 0x02},
	{0x10 , 0x88},
        //crop mode 
    {0x50 , 0x01},
    {0x51, 0x00},
    {0x52, 0x00},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x56, 0x78},
    {0x57, 0x00},
    {0x58, 0xA0},
    //subsample 1/4
    {0x59, 0x44},
    {0x5a, 0x03},
    {0x5b, 0x00},
    {0x5c, 0x00},
    {0x5d, 0x00},
    {0x5e, 0x00},
    {0x5f, 0x00},
    {0x60, 0x00},
    {0x61, 0x00},
    {0x62, 0x00},

    {0x00, 0x00}
};

static const uint8_t qvga_config[][2] = { //k210 
    {0xfe , 0x00},
    // window
        //windowing mode
	{0x09 , 0x00},
    {0x0a , 0x00},
	{0x0b , 0x00},
	{0x0c , 0x00},
    {0x0d , 0x01},
	{0x0e , 0xe8},
	{0x0f , 0x02},
	{0x10 , 0x88},
        //crop mode 
    {0x50 , 0x01},
    // {0x51, 0x00},
    // {0x52, 0x78},
    // {0x53, 0x00},
    // {0x54, 0xa0},
    // {0x55, 0x00},
    // {0x56, 0xf0},
    // {0x57, 0x01},
    // {0x58, 0x40},
    //subsample 1/2
    {0x59, 0x22},
    {0x5a, 0x00},
    {0x5b, 0x00},
    {0x5c, 0x00},
    {0x5d, 0x00},
    {0x5e, 0x00},
    {0x5f, 0x00},
    {0x60, 0x00},
    {0x61, 0x00},
    {0x62, 0x00},

    {0x00, 0x00}
};

static const uint8_t vga_config[][2] = { //k210 
    {0xfe , 0x00},
    // window
        //windowing mode
	// {0x09 , 0x00},
    // {0x0a , 0x78},
	// {0x0b , 0x00},
	// {0x0c , 0xa0},
    // {0x0d , 0x00},
	// {0x0e , 0xf8},
	// {0x0f , 0x01},
	// {0x10 , 0x48},
        //crop mode 
    {0x50 , 0x00},
    // {0x51, 0x00},
    // {0x52, 0x78},
    // {0x53, 0x00},
    // {0x54, 0xa0},
    // {0x55, 0x00},
    // {0x56, 0xf0},
    // {0x57, 0x01},
    // {0x58, 0x40},
    //subsample 1/2
    // {0x59, 0x00},
    // {0x5a, 0x00},
    // {0x5b, 0x00},
    // {0x5c, 0x00},
    // {0x5d, 0x00},
    // {0x5e, 0x00},
    // {0x5f, 0x00},
    // {0x60, 0x00},
    // {0x61, 0x00},
    // {0x62, 0x00},
    {0x00, 0x00}
};

/*
#define NUM_CONTRAST_LEVELS (5)
static uint8_t contrast_regs[NUM_CONTRAST_LEVELS][2]={
	{0x80, 0x00},
	{0x80, 0x20},
	{0x80, 0x40},
	{0x80, 0x60},
	{0x80, 0x80}
};


#define NUM_SATURATION_LEVELS (5)
static uint8_t saturation_regs[NUM_SATURATION_LEVELS][3]={
	{0x00, 0x00, 0x00},
	{0x10, 0x10, 0x10},
	{0x20, 0x20, 0x20},
	{0x30, 0x30, 0x30},
	{0x40, 0x40, 0x40},
};*/

const int resolution[][2] = {
    {0,    0   },
    // C/SIF Resolutions
    {88,   72  },    /* QQCIF     */
    {176,  144 },    /* QCIF      */
    {352,  288 },    /* CIF       */
    {88,   60  },    /* QQSIF     */
    {176,  120 },    /* QSIF      */
    {352,  240 },    /* SIF       */
    // VGA Resolutions
    {40,   30  },    /* QQQQVGA   */
    {80,   60  },    /* QQQVGA    */
    {160,  120 },    /* QQVGA     */
    {320,  240 },    /* QVGA      */
    {640,  480 },    /* VGA       */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x128    */
    {240,  240 },    /* 240x240    */
    // Other
    {224, 224}, //{128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1280, 1024},    /* SXGA      */
    {1600, 1200},    /* UXGA      */
};


int gc0328_set_framesize(framesize_t framesize)
{
    int ret=0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    int i=0;
    const uint8_t (*regs)[2];

	if(framesize == FRAMESIZE_QQVGA)
	{
		regs = qqvga_config;
	}
    else if ((w <= 320) && (h <= 240)) {
        regs = qvga_config;
    } else {
        regs = vga_config;
    }

    while (regs[i][0]) {
        cambus_writeb(GC0328_ADDR, regs[i][0], regs[i][1]);
        i++;
    }
    /* delay n ms */
    msleep(30);
	dvp_set_image_size(w, h);
    return ret;
}


int gc0328_read_id(void)
{
	uint8_t id;

	dvp_sccb_send_data(GC0328_ADDR, 0xfe, 0x00);
    id = dvp_sccb_receive_data(GC0328_ADDR, 0xF0);
	if (id != 0x9d) {
		printf("error gc0328 detect, ret id is 0x%x\r\n", id);
		return 0;
	}

    return id;
}

extern volatile dvp_t* const dvp;
#define DCMI_RESET_LOW()      dvp->cmos_cfg &= ~DVP_CMOS_RESET
#define DCMI_RESET_HIGH()     dvp->cmos_cfg |= DVP_CMOS_RESET
#define DCMI_PWDN_LOW()       dvp->cmos_cfg &= ~DVP_CMOS_POWER_DOWN
#define DCMI_PWDN_HIGH()      dvp->cmos_cfg |= DVP_CMOS_POWER_DOWN

int gc0328_init()
{
	uint8_t id;
  	uint16_t index = 0;

    /*DCMI_PWDN_LOW();//enable gc0328 要恢复 normal 工作模式，需将 PWDN pin 接入低电平即可，同时写入初始化寄存器即可
    DCMI_RESET_LOW();//reset gc3028
    msleep(10);
    DCMI_RESET_HIGH();
    msleep(10);*/

	id = cambus_scan_gc0328();
	if (!id) {
		printf("error gc0328 detect, ret id is 0x%x\r\n", id);
		return -1;
	}

	dvp_sccb_send_data(GC0328_ADDR, 0xfe, 0x01);
	for (index = 0; sensor_default_regs[index][0]; index++)
	{
    	if(sensor_default_regs[index][0] == 0xff){
        	msleep(sensor_default_regs[index][1]);
        	continue;
    	}
    	// printf("0x12,0x%02x,0x%02x,\r\n", sensor_default_regs[index][0], sensor_default_regs[index][1]);//debug
    	dvp_sccb_send_data(GC0328_ADDR, sensor_default_regs[index][0], sensor_default_regs[index][1]);
    	// mp_hal_delay_ms(1);
	}
    
	return 0;
}

int gc0328_reset(void)
{
    uint16_t index = 0;
    
    cambus_writeb(GC0328_ADDR, 0xfe, 0x01);
    for (index = 0; sensor_default_regs[index][0]; index++)
    {
        if(sensor_default_regs[index][0] == 0xff){
            msleep(sensor_default_regs[index][1]);
            continue;
        }
        // mp_printf(&mp_plat_print, "0x12,0x%02x,0x%02x,\r\n", sensor_default_regs[index][0], sensor_default_regs[index][1]);//debug
        cambus_writeb(GC0328_ADDR, sensor_default_regs[index][0], sensor_default_regs[index][1]);
        // mp_hal_delay_ms(1);
    }
    return 0;
}

