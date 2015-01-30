/**
 * @file   interrupts.c
 * @author Wiktor Gołgowski <wgolgowski@gmail.com>
 * @date   Fri Jan 30 23:34:08 2015
 * 
 * @brief  This file contains required interrupt handlers.
 * 
 */

#include "delay.h"

void SysTick_Handler(void)
{
    DelayMs_Decrement();
}

