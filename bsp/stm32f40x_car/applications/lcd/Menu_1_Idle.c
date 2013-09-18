/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>

#include "menu_include.h"
#include "sed1520.h"
#include "printer.h"

static uint32_t lasttick;
unsigned char	dispstat = 0;

unsigned char	gsm_g[] = {
	0x1c,                                                                       /*[   ***  ]*/
	0x22,                                                                       /*[  *   * ]*/
	0x40,                                                                       /*[ *      ]*/
	0x40,                                                                       /*[ *      ]*/
	0x4e,                                                                       /*[ *  *** ]*/
	0x42,                                                                       /*[ *    * ]*/
	0x22,                                                                       /*[  *   * ]*/
	0x1e,                                                                       /*[   **** ]*/
};

unsigned char	gsm_0[] = {
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x80,                                                                       /*[*       ]*/
	0x80,                                                                       /*[*       ]*/
};

unsigned char	gsm_1[] = {
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x20,                                                                       /*[  *     ]*/
	0x20,                                                                       /*[  *     ]*/
	0xa0,                                                                       /*[* *     ]*/
	0xa0,                                                                       /*[* *     ]*/
};

unsigned char	gsm_2[] = {
	0x00,                                                                       /*[        ]*/
	0x00,                                                                       /*[        ]*/
	0x08,                                                                       /*[    *   ]*/
	0x08,                                                                       /*[    *   ]*/
	0x28,                                                                       /*[  * *   ]*/
	0x28,                                                                       /*[  * *   ]*/
	0xa8,                                                                       /*[* * *   ]*/
	0xa8,                                                                       /*[* * *   ]*/
};

unsigned char	gsm_3[] = {
	0x02,                                                                       /*[      * ]*/
	0x02,                                                                       /*[      * ]*/
	0x0a,                                                                       /*[    * * ]*/
	0x0a,                                                                       /*[    * * ]*/
	0x2a,                                                                       /*[  * * * ]*/
	0x2a,                                                                       /*[  * * * ]*/
	0xaa,                                                                       /*[* * * * ]*/
	0xaa,                                                                       /*[* * * * ]*/
};

unsigned char	link_on[] = {
	0x08,                                                                       /*[    *   ]*/
	0x04,                                                                       /*[     *  ]*/
	0xfe,                                                                       /*[******* ]*/
	0x00,                                                                       /*[        ]*/
	0xfe,                                                                       /*[******* ]*/
	0x40,                                                                       /*[ *      ]*/
	0x20,                                                                       /*[  *     ]*/
	0x00,                                                                       /*[        ]*/
};

unsigned char	link_off[] = {
	0x10,                                                                       /*[   *    ]*/
	0x08,                                                                       /*[    *   ]*/
	0xc6,                                                                       /*[**   ** ]*/
	0x00,                                                                       /*[        ]*/
	0xe6,                                                                       /*[***  ** ]*/
	0x10,                                                                       /*[   *    ]*/
	0x08,                                                                       /*[    *   ]*/
	0x00,                                                                       /*[        ]*/
};

unsigned char	num0[] = { 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00 };    /*"0",0*/ /*"0",0*/
unsigned char	num1[] = { 0x00, 0x20, 0x60, 0x20, 0x20, 0x20, 0x70, 0x00 };    /*"1",1*/ /*"1",1*/
unsigned char	num2[] = { 0x00, 0x78, 0x88, 0x08, 0x30, 0x40, 0xF8, 0x00 };    /*"2",2*/ /*"2",2*/
unsigned char	num3[] = { 0x00, 0x78, 0x88, 0x30, 0x08, 0x88, 0x70, 0x00 };    /*"3",3*/ /*"3",3*/
unsigned char	num4[] = { 0x00, 0x10, 0x30, 0x50, 0x90, 0x78, 0x10, 0x00 };    /*"4",4*/ /*"4",4*/
unsigned char	num5[] = { 0x00, 0xF8, 0x80, 0xF0, 0x08, 0x88, 0x70, 0x00 };    /*"5",5*/ /*"5",5*/
unsigned char	num6[] = { 0x00, 0x70, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00 };    /*"6",6*/ /*"6",6*/
unsigned char	num7[] = { 0x00, 0xF8, 0x90, 0x10, 0x20, 0x20, 0x20, 0x00 };    /*"7",7*/ /*"7",7*/
unsigned char	num8[] = { 0x00, 0x70, 0x88, 0x50, 0x88, 0x88, 0x70, 0x00 };    /*"8",8*/ /*"8",8*/
unsigned char	num9[] = { 0x00, 0x70, 0x88, 0x88, 0x78, 0x08, 0x70, 0x00 };    /*"9",9*/ /*"9",9*/

//DECL_BMP(6,8,num0);

IMG_DEF img_num0608[10] =
{
	{ 6, 8, num0 },
	{ 6, 8, num1 },
	{ 6, 8, num2 },
	{ 6, 8, num3 },
	{ 6, 8, num4 },
	{ 6, 8, num5 },
	{ 6, 8, num6 },
	{ 6, 8, num7 },
	{ 6, 8, num8 },
	{ 6, 8, num9 },
};

static unsigned char	Battery[] = { 0x00, 0xFC, 0xFF, 0xFF, 0xFC, 0x00 };     //8*6
static unsigned char	NOBattery[] = { 0x04, 0x0C, 0x98, 0xB0, 0xE0, 0xF8 };   //6*6
static unsigned char	TriangleS[] = { 0x30, 0x78, 0xFC, 0xFC, 0x78, 0x30 };   //6*6
static unsigned char	TriangleK[] = { 0x30, 0x48, 0x84, 0x84, 0x48, 0x30 };   //6*6

static unsigned char	empty[] = { 0x84, 0x84, 0x84, 0x84, 0x84, 0xFC };       /*空车*/
static unsigned char	full_0[] = { 0x84, 0x84, 0x84, 0xFC, 0xFC, 0xFC };      /*半满*/
static unsigned char	full_1[] = { 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC };      /*重车*/

static unsigned char	arrow_0[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00,
	0x01, 0xF0, 0x00, 0x00, 0x7F, 0x83, 0xF8, 0x3F, 0xC0, 0x41, 0x07, 0xFC, 0x10, 0x40, 0x42, 0x0F,
	0xFE, 0x08, 0x40, 0x44, 0x00, 0x00, 0x04, 0x40, 0x49, 0x55, 0x55, 0x52, 0x40, 0x50, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x05, 0x00, 0x25, 0x00, 0x00, 0x14, 0x80, 0x44, 0x00, 0x00, 0x04, 0x40,
	0x85, 0x00, 0x00, 0x14, 0x20, 0x44, 0x00, 0x00, 0x04, 0x40, 0x25, 0x00, 0x00, 0x14, 0x80, 0x14,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0x40, 0x48, 0x0F, 0xFE, 0x02, 0x40, 0x44, 0x04, 0x04,
	0x04, 0x40, 0x42, 0x02, 0x08, 0x08, 0x40, 0x41, 0x01, 0x10, 0x10, 0x40, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

unsigned char			arrow_45[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x41, 0x04, 0x04, 0x1F, 0xC0, 0x42, 0x0F,
	0xFE, 0x0F, 0xC0, 0x44, 0x00, 0x00, 0x07, 0xC0, 0x49, 0x55, 0x55, 0x53, 0xC0, 0x50, 0x00, 0x00,
	0x01, 0xC0, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x05, 0x00, 0x25, 0x00, 0x00, 0x14, 0x80, 0x44, 0x00, 0x00, 0x04, 0x40,
	0x85, 0x00, 0x00, 0x14, 0x20, 0x44, 0x00, 0x00, 0x04, 0x40, 0x25, 0x00, 0x00, 0x14, 0x80, 0x14,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0x40, 0x48, 0x0F, 0xFE, 0x02, 0x40, 0x44, 0x04, 0x04,
	0x04, 0x40, 0x42, 0x02, 0x08, 0x08, 0x40, 0x41, 0x01, 0x10, 0x10, 0x40, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char			arrow_90[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x41, 0x04, 0x04, 0x10, 0x40, 0x42, 0x0F,
	0xFE, 0x08, 0x40, 0x44, 0x00, 0x00, 0x04, 0x40, 0x49, 0x55, 0x55, 0x52, 0x40, 0x50, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x07, 0x00, 0x25, 0x00, 0x00, 0x17, 0x80, 0x44, 0x00, 0x00, 0x07, 0xC0,
	0x85, 0x00, 0x00, 0x17, 0xE0, 0x44, 0x00, 0x00, 0x07, 0xC0, 0x25, 0x00, 0x00, 0x17, 0x80, 0x14,
	0x00, 0x00, 0x07, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0x40, 0x48, 0x0F, 0xFE, 0x02, 0x40, 0x44, 0x04, 0x04,
	0x04, 0x40, 0x42, 0x02, 0x08, 0x08, 0x40, 0x41, 0x01, 0x10, 0x10, 0x40, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

unsigned char			arrow_135[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x41, 0x04, 0x04, 0x10, 0x40, 0x42, 0x0F,
	0xFE, 0x08, 0x40, 0x44, 0x00, 0x00, 0x04, 0x40, 0x49, 0x55, 0x55, 0x52, 0x40, 0x50, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x05, 0x00, 0x25, 0x00, 0x00, 0x14, 0x80, 0x44, 0x00, 0x00, 0x04, 0x40,
	0x85, 0x00, 0x00, 0x14, 0x20, 0x44, 0x00, 0x00, 0x04, 0x40, 0x25, 0x00, 0x00, 0x14, 0x80, 0x14,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0xC0, 0x48, 0x0F, 0xFE, 0x03, 0xC0, 0x44, 0x04, 0x04,
	0x07, 0xC0, 0x42, 0x02, 0x08, 0x0F, 0xC0, 0x41, 0x01, 0x10, 0x1F, 0xC0, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char			arrow_180[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x41, 0x04, 0x04, 0x10, 0x40, 0x42, 0x0F,
	0xFE, 0x08, 0x40, 0x44, 0x00, 0x00, 0x04, 0x40, 0x49, 0x55, 0x55, 0x52, 0x40, 0x50, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x05, 0x00, 0x25, 0x00, 0x00, 0x14, 0x80, 0x44, 0x00, 0x00, 0x04, 0x40,
	0x85, 0x00, 0x00, 0x14, 0x20, 0x44, 0x00, 0x00, 0x04, 0x40, 0x25, 0x00, 0x00, 0x14, 0x80, 0x14,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0x40, 0x48, 0x0F, 0xFE, 0x02, 0x40, 0x44, 0x07, 0xFC,
	0x04, 0x40, 0x42, 0x03, 0xF8, 0x08, 0x40, 0x41, 0x01, 0xF0, 0x10, 0x40, 0x7F, 0x80, 0xE0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

unsigned char			arrow_225[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x41, 0x04, 0x04, 0x10, 0x40, 0x42, 0x0F,
	0xFE, 0x08, 0x40, 0x44, 0x00, 0x00, 0x04, 0x40, 0x49, 0x55, 0x55, 0x52, 0x40, 0x50, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x05, 0x00, 0x25, 0x00, 0x00, 0x14, 0x80, 0x44, 0x00, 0x00, 0x04, 0x40,
	0x85, 0x00, 0x00, 0x14, 0x20, 0x44, 0x00, 0x00, 0x04, 0x40, 0x25, 0x00, 0x00, 0x14, 0x80, 0x14,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x70, 0x00, 0x00, 0x01, 0x40, 0x78, 0x0F, 0xFE, 0x02, 0x40, 0x7C, 0x04, 0x04,
	0x04, 0x40, 0x7E, 0x02, 0x08, 0x08, 0x40, 0x7F, 0x01, 0x10, 0x10, 0x40, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

unsigned char			arrow_270[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x41, 0x04, 0x04, 0x10, 0x40, 0x42, 0x0F,
	0xFE, 0x08, 0x40, 0x44, 0x00, 0x00, 0x04, 0x40, 0x49, 0x55, 0x55, 0x52, 0x40, 0x50, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x1C, 0x00, 0x00, 0x05, 0x00, 0x3D, 0x00, 0x00, 0x14, 0x80, 0x7C, 0x00, 0x00, 0x04, 0x40,
	0xFD, 0x00, 0x00, 0x14, 0x20, 0x7C, 0x00, 0x00, 0x04, 0x40, 0x3D, 0x00, 0x00, 0x14, 0x80, 0x1C,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0x40, 0x48, 0x0F, 0xFE, 0x02, 0x40, 0x44, 0x04, 0x04,
	0x04, 0x40, 0x42, 0x02, 0x08, 0x08, 0x40, 0x41, 0x01, 0x10, 0x10, 0x40, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char			arrow_315[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00, 0x7F, 0x82, 0x08, 0x3F, 0xC0, 0x7F, 0x04, 0x04, 0x10, 0x40, 0x7E, 0x0F,
	0xFE, 0x08, 0x40, 0x7C, 0x00, 0x00, 0x04, 0x40, 0x79, 0x55, 0x55, 0x52, 0x40, 0x70, 0x00, 0x00,
	0x01, 0x40, 0x61, 0x00, 0x00, 0x10, 0xC0, 0x44, 0x00, 0x00, 0x04, 0x40, 0x0D, 0x00, 0x00, 0x16,
	0x00, 0x14, 0x00, 0x00, 0x05, 0x00, 0x25, 0x00, 0x00, 0x14, 0x80, 0x44, 0x00, 0x00, 0x04, 0x40,
	0x85, 0x00, 0x00, 0x14, 0x20, 0x44, 0x00, 0x00, 0x04, 0x40, 0x25, 0x00, 0x00, 0x14, 0x80, 0x14,
	0x00, 0x00, 0x05, 0x00, 0x0D, 0x00, 0x00, 0x16, 0x00, 0x44, 0x00, 0x00, 0x04, 0x40, 0x61, 0x55,
	0x55, 0x50, 0xC0, 0x50, 0x00, 0x00, 0x01, 0x40, 0x48, 0x0F, 0xFE, 0x02, 0x40, 0x44, 0x04, 0x04,
	0x04, 0x40, 0x42, 0x02, 0x08, 0x08, 0x40, 0x41, 0x01, 0x10, 0x10, 0x40, 0x7F, 0x80, 0xA0, 0x3F,
	0xC0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

IMG_DEF					img_arrow[8] =
{
	{ 35, 32, arrow_0	},
	{ 35, 32, arrow_45	},
	{ 35, 32, arrow_90	},
	{ 35, 32, arrow_135 },
	{ 35, 32, arrow_180 },
	{ 35, 32, arrow_225 },
	{ 35, 32, arrow_270 },
	{ 35, 32, arrow_315 },
};

/*0度周围的角度 337-359 0-23作为缺省情况处理*/
static uint8_t check_arrow_index( uint16_t cog )
{
	uint8_t		i;
	uint16_t	arrow_scope[8] = { 23, 68, 113, 158, 203, 248, 293, 337 };
	for( i = 1; i < 8; i++ )
	{
		if( ( cog > arrow_scope[i - 1] ) && ( cog <= arrow_scope[i] ) )
		{
			return i;
		}
	}
	return 0;
}

//电池 是否校验特征系数的标志
DECL_BMP( 8, 6, Battery );  DECL_BMP( 6, 6, NOBattery );
DECL_BMP( 6, 6, TriangleS ); DECL_BMP( 6, 6, TriangleK );
//信号强度标志
DECL_BMP( 7, 8, gsm_g );    DECL_BMP( 7, 8, gsm_0 );
DECL_BMP( 7, 8, gsm_1 );    DECL_BMP( 7, 8, gsm_2 );
DECL_BMP( 7, 8, gsm_3 );
//连接或者在线标志
DECL_BMP( 7, 8, link_on );  DECL_BMP( 7, 8, link_off );
//空车 半满 重车
DECL_BMP( 6, 6, empty );    DECL_BMP( 6, 6, full_0 );  DECL_BMP( 6, 6, full_1 );


/*
   这里显示gps时间，若gps未定位，时间停走
   可选择
   rtc时间，需要频繁读取(1秒1次)，如何判断校时
   使用自己的mytime，如何更新(使用系统的1s定时器是否准确)
 */
#if 1
void Disp_Idle( void )
{
	char	*mode[] = { "  ", "BD", "GP", "GN" };
	char	buf_date[22];
	char	buf_time[22];
	char	buf_speed[20];

	lcd_fill( 0 );
	if( jt808_alarm & ( BIT_ALARM_GPS_ERR | BIT_ALARM_GPS_OPEN | BIT_ALARM_GPS_SHORT ) )    /*北斗异常，不显示*/
	{
		lcd_text12( 0, 0, mode[0], 2, LCD_MODE_SET );
		sprintf( buf_date, "--/--/--" );
		sprintf( buf_time, "--:--:--" );
	}else                                                                                   /*定位正常显示*/
	{
		if( jt808_status & BIT_STATUS_FIXED )
		{
			lcd_text12( 0, 0, mode[gps_status.mode], 2, LCD_MODE_SET );
		}else
		{
			lcd_text12( 0, 0, mode[gps_status.mode], 2, LCD_MODE_INVERT );
		}
		lcd_bitmap( 12, 1, &img_num0608[gps_status.NoSV / 10], LCD_MODE_SET );
		lcd_bitmap( 18, 1, &img_num0608[gps_status.NoSV % 10], LCD_MODE_SET );

		sprintf( buf_date, "%02d-%02d-%02d",
		         YEAR( mytime_now ),
		         MONTH( mytime_now ),
		         DAY( mytime_now ) );

		sprintf( buf_time, "%02d:%02d:%02d",
		         HOUR( mytime_now ),
		         MINUTE( mytime_now ),
		         SEC( mytime_now ) );
		lcd_bitmap( 86, 0, &img_arrow[check_arrow_index( gps_cog )], LCD_MODE_SET );
		sprintf( buf_speed, "%03d", gps_speed );
		lcd_text12( 95, 9, (char*)buf_speed, strlen( buf_speed ), LCD_MODE_SET );
	}

	lcd_text12( 0, 10, (char*)buf_date, strlen( buf_date ), LCD_MODE_SET );
	lcd_text12( 0, 21, (char*)buf_time, strlen( buf_time ), LCD_MODE_SET );

	lcd_bitmap( 30, 2, &BMP_gsm_g, LCD_MODE_SET );
	lcd_bitmap( 38, 2, &BMP_gsm_3, LCD_MODE_SET );

	lcd_text12( 48, 0, "GPRS", 4, LCD_MODE_SET );

	if( gsm_socket[0].state == CONNECTED ) /*gprs连接状态*/
	{
		lcd_bitmap( 72, 2, &BMP_link_on, LCD_MODE_SET );
	}else
	{
		lcd_bitmap( 72, 2, &BMP_link_off, LCD_MODE_SET );
	}

#if 0
	if( JT808Conf_struct.LOAD_STATE == 1 ) //车辆载重标志
	{
		lcd_bitmap( 95, 2, &BMP_empty, LCD_MODE_SET );
	}else if( JT808Conf_struct.LOAD_STATE == 2 )
	{
		lcd_bitmap( 95, 2, &BMP_full_0, LCD_MODE_SET );
	}else if( JT808Conf_struct.LOAD_STATE == 3 )
	{
		lcd_bitmap( 95, 2, &BMP_full_1, LCD_MODE_SET );
	}

	if( ModuleStatus & 0x04 ) //电源标志
	{
		lcd_bitmap( 105, 2, &BMP_Battery, LCD_MODE_SET );
	}else
	{
		lcd_bitmap( 105, 2, &BMP_NOBattery, LCD_MODE_SET );
	}

	if( DF_K_adjustState ) //是否校验特征系数
	{
		lcd_bitmap( 115, 2, &BMP_TriangleS, LCD_MODE_SET );
	}else
	{
		lcd_bitmap( 115, 2, &BMP_TriangleK, LCD_MODE_SET );
	}
#endif

	lcd_update_all( );
}

#else


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Disp_Idle( void )
{
	char	*mode[] = { "  ", "BD", "GP", "GN" };
	char	buf_date[22];
	char	buf_time[22];
	char	buf_speed[20];

	lcd_fill( 0 );
	if( jt808_alarm & ( BIT_ALARM_GPS_ERR | BIT_ALARM_GPS_OPEN | BIT_ALARM_GPS_SHORT ) )    /*北斗异常，不显示*/
	{
		lcd_text12( 0, 0, mode[0], 2, LCD_MODE_SET );
		sprintf( buf_date, "--/--/--" );
		sprintf( buf_time, "--:--:--" );
	}else if( jt808_status & BIT_STATUS_FIXED )                                             /*定位正常显示*/
	{
		lcd_text12( 0, 0, mode[gps_status.mode], 2, LCD_MODE_SET );
		lcd_bitmap( 12, 1, &img_num0608[gps_status.NoSV / 10], LCD_MODE_SET );
		lcd_bitmap( 18, 1, &img_num0608[gps_status.NoSV % 10], LCD_MODE_SET );

		sprintf( buf_date, "%02d-%02d-%02d",
		         YEAR( mytime_now ),
		         MONTH( mytime_now ),
		         DAY( mytime_now ) );

		sprintf( buf_time, "%02d:%02d:%02d",
		         HOUR( mytime_now ),
		         MINUTE( mytime_now ),
		         SEC( mytime_now ) );
		lcd_bitmap( 86, 0, &img_arrow[check_arrow_index( gps_cog )], LCD_MODE_SET );
		sprintf( buf_speed, "%03d", gps_speed );
		lcd_text12( 95, 9, (char*)buf_speed, strlen( buf_speed ), LCD_MODE_SET );
	}else /*未定位，反色显示*/
	{
		lcd_text12( 0, 0, mode[gps_status.mode], 2, LCD_MODE_INVERT );
		lcd_bitmap( 12, 1, &img_num0608[gps_status.NoSV / 10], LCD_MODE_SET );
		lcd_bitmap( 18, 1, &img_num0608[gps_status.NoSV % 10], LCD_MODE_SET );
		sprintf( buf_date, "--/--/--" );
		sprintf( buf_time, "%02d:%02d:%02d", gps_sec_count / 3600, ( gps_sec_count % 3600 ) / 60, ( gps_sec_count % 3600 ) % 60 );
	}

	lcd_text12( 0, 10, (char*)buf_date, strlen( buf_date ), LCD_MODE_SET );
	lcd_text12( 0, 21, (char*)buf_time, strlen( buf_time ), LCD_MODE_SET );

	lcd_bitmap( 30, 2, &BMP_gsm_g, LCD_MODE_SET );
	lcd_bitmap( 38, 2, &BMP_gsm_3, LCD_MODE_SET );

	lcd_text12( 48, 0, "GPRS", 4, LCD_MODE_SET );

	if( gsm_socket[0].state == CONNECTED ) /*gprs连接状态*/
	{
		lcd_bitmap( 72, 2, &BMP_link_on, LCD_MODE_SET );
	}else
	{
		lcd_bitmap( 72, 2, &BMP_link_off, LCD_MODE_SET );
	}

#if 0
	if( JT808Conf_struct.LOAD_STATE == 1 ) //车辆载重标志
	{
		lcd_bitmap( 95, 2, &BMP_empty, LCD_MODE_SET );
	}else if( JT808Conf_struct.LOAD_STATE == 2 )
	{
		lcd_bitmap( 95, 2, &BMP_full_0, LCD_MODE_SET );
	}else if( JT808Conf_struct.LOAD_STATE == 3 )
	{
		lcd_bitmap( 95, 2, &BMP_full_1, LCD_MODE_SET );
	}

	if( ModuleStatus & 0x04 ) //电源标志
	{
		lcd_bitmap( 105, 2, &BMP_Battery, LCD_MODE_SET );
	}else
	{
		lcd_bitmap( 105, 2, &BMP_NOBattery, LCD_MODE_SET );
	}

	if( DF_K_adjustState ) //是否校验特征系数
	{
		lcd_bitmap( 115, 2, &BMP_TriangleS, LCD_MODE_SET );
	}else
	{
		lcd_bitmap( 115, 2, &BMP_TriangleK, LCD_MODE_SET );
	}
#endif

	lcd_update_all( );
}

#endif

/**/
static void msg( void *p )
{
}

/**/
static void show( void )
{
	Disp_Idle( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void keypress( unsigned int key )
{
	char	buf[128];
	uint8_t i, pos, hour, minute;
	switch( key )
	{
		case KEY_MENU:
			pMenuItem = &Menu_2_InforCheck;
			pMenuItem->show( );
			break;
		case KEY_OK:

			break;
		case KEY_UP:

			break;
		case KEY_DOWN:
			//打印开电
			//GPIO_SetBits( GPIOB, GPIO_Pin_6 );

			sprintf( buf, "车牌号码:%s\n", jt808_param.id_0x0083 );
			printer( buf );
			sprintf( buf, "车牌分类:%s\n", jt808_param.id_0xF00A );
			printer( buf );
			sprintf( buf, "车辆VIN:%s\n", jt808_param.id_0xF005 );
			printer( buf );
			sprintf( buf, "驾驶员姓名:%s\n", jt808_param.id_0xF008 );
			printer( buf );
			sprintf( buf, "驾驶证代码:%s\n", jt808_param.id_0xF009 );
			printer( buf );
			memset( buf, 64, 0 );
			sprintf( buf, "打印时间:20%02d-%02d-%02d %02d:%02d:%02d\n",
			         YEAR( mytime_now ),
			         MONTH( mytime_now ),
			         DAY( mytime_now ),
			         HOUR( mytime_now ),
			         MINUTE( mytime_now ),
			         SEC( mytime_now )
			         );

			printer( buf );
			printer( "停车前15分钟车速:\n" );
			pos = hmi_15min_speed_curr;
			for( i = 0; i < 15; i++ )
			{
				if( hmi_15min_speed[pos].time != 0 ) /*有数据*/
				{
					hour	= HOUR( hmi_15min_speed[pos].time );
					minute	= MINUTE( hmi_15min_speed[pos].time );
					sprintf( buf, " [%02d] %02d:%02d %d kmh\n", i + 1, hour, minute, hmi_15min_speed[pos].speed );
					printer( buf );
				}else
				{
					sprintf( buf, " [%02d] --:-- --\n", i + 1 );
					printer( buf );
				}
				if( pos == 0 )
				{
					pos = 15;
				}
				pos--;
			}

			printer( "最近一次疲劳驾驶记录:\n无疲劳驾驶记录\n\n\n\n\n\n\n" );
			//GPIO_ResetBits( GPIOB, GPIO_Pin_6 );

			break;
		case KEY_MENU_REPEAT: /*长按进入设置*/
			pMenuItem = &Menu_0_0_password;
			pMenuItem->show( );
			break;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void timetick( unsigned int systick )
{
#if NEED_TODO

	if( reset_firstset == 6 )
	{
		reset_firstset++;
		//----------------------------------------------------------------------------------
		JT808Conf_struct.password_flag = 0; // clear  first flag
		Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
		//----------------------------------------------------------------------------------
	}else if( reset_firstset >= 7 )         //50ms一次,,60s
	{
		reset_firstset++;
		lcd_fill( 0 );
		lcd_text12( 0, 3, "需重新设置车牌号和ID", 20, LCD_MODE_SET );
		lcd_text12( 24, 18, "重新加电查看", 12, LCD_MODE_SET );
		lcd_update_all( );
	}else
	{
		//主电源掉电
		if( Warn_Status[1] & 0x01 )
		{
			BuzzerFlag = 11;
			lcd_fill( 0 );
			lcd_text12( 30, 10, "主电源掉电", 10, LCD_MODE_SET );
			lcd_update_all( );
		}
		//循环显示待机界面
		tickcount++;
		if( tickcount >= 16 )
		{
			tickcount = 0;
			Disp_Idle( );
		}
	}

	Cent_To_Disp( );
#endif

	if( systick - lasttick >= 50 )
	{
		Disp_Idle( );
		lasttick = systick;
	}
}

MENUITEM Menu_1_Idle =
{
	"待机界面",
	8,		   0,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

/************************************** The End Of File **************************************/
