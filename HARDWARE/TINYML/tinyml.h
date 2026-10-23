#ifndef _TINYML_H
#define _TINYML_H
#include "sys.h"
#include <string.h>
#include <stdlib.h>
#include "sram.h"
#include "touch.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//TINYML代码	   
//创建日期:2026/10/20
//版本：V1.0
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

// ============ 触摸位图缓冲区（1bit/像素） ============
#define TP_BUF_W        480
#define TP_BUF_H        800
#define TP_ROW_BYTES    (TP_BUF_W / 8)          // 480/8 = 60

// 外部 SRAM 基地址（Bank1_SRAM3_ADDR = 0x68000000）
#define EXT_SRAM_BASE   Bank1_SRAM3_ADDR

// 位图占内部 RAM
extern u8 touch_in_buffer[TP_BUF_H][TP_ROW_BYTES];   // [480][100]

// 置位：把 (x,y) 设为 1
#define TP_BUF_SET(x, y)   do { \
    if ((x) < TP_BUF_W && (y) < TP_BUF_H) \
        touch_in_buffer[(y)][(x) >> 3] |= (1 << ((x) & 7)); \
} while (0)

// 清位
#define TP_BUF_CLR_BIT(x, y) do { \
    if ((x) < TP_BUF_W && (y) < TP_BUF_H) \
        touch_in_buffer[(y)][(x) >> 3] &= ~(1 << ((x) & 7)); \
} while (0)

// 读位
#define TP_BUF_GET(x, y)   (((x) < TP_BUF_W && (y) < TP_BUF_H) ? \
    ((touch_in_buffer[(y)][(x) >> 3] >> ((x) & 7)) & 1) : 0)

// 清空整个位图
#define TP_BUF_CLEAR()     memset(touch_in_buffer, 0, sizeof(touch_in_buffer))

void TP_Buf_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);

// 缩放
void TP_Buf_To_28x28(int8_t *q_input);

// tinyml_infer 函数
extern int tinyml_predict(const int8_t *input, int8_t *output);

#endif
