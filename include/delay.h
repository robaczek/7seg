#ifndef DELAY_H
#define DELAY_H

#include "stm32f0xx.h"

void DelayMs_Decrement(void);
void DelayMs(__IO uint32_t ms);

#endif /* DELAY_H */
