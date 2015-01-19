#include "delay.h"

static __IO uint32_t DelayCounter;

void DelayMs_Decrement(void)
{
    if(DelayCounter != 0x00)
    {
        DelayCounter--;
    }
}

void DelayMs(__IO uint32_t ms)
{
    DelayCounter = ms;
    while(DelayCounter != 0x00);
}
