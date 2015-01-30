/**
 * @file   main.c
 * @author Wiktor Gołgowski <wgolgowski@gmail.com>
 * @date   Mon Jan 19 00:36:05 2015
 * 
 * @brief  PT6961 test file.
 * 
 */


#include "stm32f0xx.h"
#include <mini-printf.h>
#include "delay.h"
#include "pt6961.h"
#include "gpiopin.h"

void delay (int a);

unsigned char key_handler(PT6961_Init* pt, unsigned char key)
{
    pt6961_print(pt, "-CAFE-");
    DelayMs(400);
    char tmp[7];
    snprintf(tmp, 7, "%d", key);
    pt6961_print(pt, tmp);
    DelayMs(300);
    return 0;
}


int main(void)
{

  /* GPIOC Periph clock enable */
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  
	GPIOC->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0) ;
   

    /* Initialize SysTick for 1ms window */
    SysTick_Config(SystemCoreClock / 1000);
    
    PT6961_Init pt;
    pt.CLK = gpiopin(GPIOC, 7);
    pt.DIN = gpiopin(GPIOC, 10);
    pt.DOUT = gpiopin(GPIOC, 12);
    pt.STB = gpiopin(GPIOC, 6);
    snprintf(pt.value, PT_LEN+1, "ABC123");
    pt.handler = key_handler;
    
    /*
    pt.CLK_port = pt.DIN_port = pt.DOUT_port = pt.STB_port = GPIOC;
    pt.CLK_pin = 7;
    pt.DIN_pin = 10;
    pt.DOUT_pin = 12;
    pt.STB_pin = 6;
    */
    
    pt6961_init(&pt);
    pt6961_update(&pt);
    DelayMs(100);    
	while (1)
	{
        uint32_t data = pt6961_read(&pt);
		/* Set PC8 and PC9 */
		GPIOC->BSRR = 1 << 8;
		/* Reset PC8 and PC9 */
        pt6961_update(&pt);
        GPIOC->BSRR = (data & 0x01) << 9;
        /* snprintf(pt.value, PT_LEN+1, "%d", data); */
        /* DelayMs(100); */
        GPIOC->BRR = 1 << 9;
        GPIOC->BRR = 1 << 8;
	}
	
	return 0;
}

