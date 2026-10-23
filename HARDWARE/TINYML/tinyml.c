#include "tinyml.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK STM32F407开发板
//正点原子@ALIENTEK
//创建日期:2026/10/20
//版本：V1.0
//All rights reserved
//********************************************************************************

u8 touch_in_buffer[TP_BUF_H][TP_ROW_BYTES];   // [480][100]

// 在两点之间插值，把经过的位图格都置 1
void TP_Buf_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    int dx = (int)x2 - (int)x1;
    int dy = (int)y2 - (int)y1;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    int i;

    if (steps == 0) {
        TP_BUF_SET(x1, y1);
        return;
    }
    for (i = 0; i <= steps; i++) {
        int x = (int)x1 + dx * i / steps;
        int y = (int)y1 + dy * i / steps;
        if (x >= 0 && x < TP_BUF_W && y >= 0 && y < TP_BUF_H)
            TP_BUF_SET((u16)x, (u16)y);
    }
}

// ============ 缩放 + 量化 ============
#define NN_IN_W   28
#define NN_IN_H   28
#define NN_BOX    20   // MNIST 数字大约占 20x20

void TP_Buf_To_28x28(int8_t *q_input)
{
    u16 x, y;
    u16 min_x = TP_BUF_W, max_x = 0;
    u16 min_y = TP_BUF_H, max_y = 0;
    u8  found = 0;

    // 1. 找 bounding box
    for (y = 0; y < TP_BUF_H; y++) {
        for (x = 0; x < TP_BUF_W; x++) {
            if (TP_BUF_GET(x, y)) {
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
                found = 1;
            }
        }
    }

    // 2. 初始化为 0（MNIST 背景是 0，对应 int8 -128）
    for (int i = 0; i < NN_IN_W * NN_IN_H; i++)
        q_input[i] = -128;

    if (!found) return;   // 没画东西，全 0

    // 3. 计算 bounding box 宽高
    u16 bw = max_x - min_x + 1;
    u16 bh = max_y - min_y + 1;

    // 4. 等比缩放：长边缩到 NN_BOX，短边按比例
    u16 scale_w, scale_h;
    if (bw >= bh) {
        scale_w = NN_BOX;
        scale_h = (u32)bh * NN_BOX / bw;
        if (scale_h == 0) scale_h = 1;
    } else {
        scale_h = NN_BOX;
        scale_w = (u32)bw * NN_BOX / bh;
        if (scale_w == 0) scale_w = 1;
    }

    // 5. 居中偏移
    u16 off_x = (NN_IN_W - scale_w) / 2;
    u16 off_y = (NN_IN_H - scale_h) / 2;

    // 6. 区域平均缩放到目标尺寸
    for (u16 dy = 0; dy < scale_h; dy++) {
        u16 sy0 = min_y + (u32)dy       * bh / scale_h;
        u16 sy1 = min_y + (u32)(dy + 1) * bh / scale_h;
        if (sy1 <= sy0) sy1 = sy0 + 1;

        for (u16 dx = 0; dx < scale_w; dx++) {
            u16 sx0 = min_x + (u32)dx       * bw / scale_w;
            u16 sx1 = min_x + (u32)(dx + 1) * bw / scale_w;
            if (sx1 <= sx0) sx1 = sx0 + 1;

            u32 sum = 0, cnt = 0;
            for (u16 sy = sy0; sy < sy1; sy++)
                for (u16 sx = sx0; sx < sx1; sx++) {
                    sum += TP_BUF_GET(sx, sy);
                    cnt++;
                }
            u32 ratio = (sum * 255) / cnt;   // 覆盖率，0~255

            u8 gray;
            if (ratio == 0) {
                gray = 0;                     // 完全没触摸，纯黑
            } else {
                // 只要有触摸就至少 100，然后按覆盖率提升
                gray = 100 + ratio * 2;
                if (gray > 255) gray = 255;
            }
            q_input[(off_y + dy) * NN_IN_W + (off_x + dx)] = (int8_t)(gray - 128);
        }
    }
}
