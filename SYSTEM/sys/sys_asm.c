#include "stm32f4xx.h"

// WFI 休眠
__attribute__((naked)) void WFI_SET(void)
{
    __asm volatile (
        "WFI \n"
        "BX LR \n"
    );
}

// 关闭可屏蔽中断 CPSID I
__attribute__((naked)) void INTX_DISABLE(void)
{
    __asm volatile (
        "CPSID I \n"
        "BX LR \n"
    );
}

// 开启可屏蔽中断 CPSIE I
__attribute__((naked)) void INTX_ENABLE(void)
{
    __asm volatile (
        "CPSIE I \n"
        "BX LR \n"
    );
}

// 设置主栈指针 MSP，参数 addr 在 r0
__attribute__((naked)) void MSR_MSP(uint32_t addr)
{
    __asm volatile (
        "MSR MSP, r0 \n"
        "BX LR \n"
    );
}