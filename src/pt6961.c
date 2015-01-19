/**
 * @file   pt6961.c
 * @author Wiktor Gołgowski <wgolgowski@gmail.com>
 * @date   Mon Jan 19 00:37:48 2015
 * 
 * @brief  PT6961 library. Basic functionality.
 * 
 */

#include "stm32f0xx.h"
#include "pt6961.h"
#include "delay.h"

#define set_gpio(gpio,bit) gpio->BSRR = 1<<bit
#define clear_gpio(gpio,bit) gpio->BRR = 1<<bit

#define DIN GPIOC,9
#define DOUT GPIOC,8
#define CLK GPIOC,7
#define STB GPIOC,6

static unsigned char pt6961_buffer[13] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void pt6961_send(unsigned char data)
{
    unsigned char i;
    for(i=0; i<8; i++)
    {
        if(data & 0x01)
        {
            set_gpio(GPIOC,9); // DIN
        }
        else
        {
            clear_gpio(GPIOC,9); // DIN
        }
        data >>=0x01;
        /* blink clock */
        clear_gpio(GPIOC,7); // CLK
        set_gpio(GPIOC,7); //clk
    }
}

void pt6961_init(void)
{
    clear_gpio(GPIOC,9); // DIN
    set_gpio(GPIOC,7); //clk
    set_gpio(GPIOC,6); //stb

    DelayMs(30);

    clear_gpio(GPIOC,6); //stb
    pt6961_send(0b01000000);
    set_gpio(GPIOC,6);//stb

    clear_gpio(GPIOC,6); //stb
    pt6961_send(0b11000000);

    unsigned char i;
    for(i=0; i<13; i++)
    {
        pt6961_send(0x0);
    }
    set_gpio(GPIOC,6);

    clear_gpio(GPIOC,6);//stb
    pt6961_send(0b00000011);
    set_gpio(GPIOC,6);//stb

    clear_gpio(GPIOC,6);//stb
    pt6961_send(0b10001000);
    set_gpio(GPIOC,6);//stb
}

void pt6961_update(void)
{
    clear_gpio(GPIOC,6);//stb
    pt6961_send(0b11000000);
    unsigned char i;
    for(i=0; i<13; i++)
    {
        pt6961_send(pt6961_buffer[i]);
    }
    set_gpio(GPIOC,6);//stb
}
