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

/**
 * Mapping bits to proper segments:
 *
 *   a
 *   -
 * f| |b
 * g -
 * e| |c
 *   -
 *   d
 */
#define DISP_A 0b00000001
#define DISP_B 0b00000010
#define DISP_C 0b00000100
#define DISP_D 0b00010000
#define DISP_E 0b00001000
#define DISP_F 0b00100000
#define DISP_G 0b01000000

/** 
 * Helper function. As DelayMs has resolution on 1ms, here is
 * something faster.
 * 
 * @param a iterations to make
 */
void delay (int a)
{
	volatile int i,j;
	
	for (i=0 ; i < a ; i++)
	{ 
		j++;
	}
	
	return;
}

/** 
 * Helper function. Converts received ascii character to PT6961
 * accepted format for segment display. If the character is not
 * recognized, empty space is returned.
 * 
 * @param c character to convert
 * 
 * @return converted character in segment display format, zero if not found
 */
unsigned char char2segment(unsigned char c)
{
    unsigned char data = 0;
    switch(c)
    {
    case '0':
        data = DISP_A | DISP_B | DISP_C | DISP_D | DISP_E | DISP_F;
        break;
    case '1':
        data = DISP_B | DISP_C;
        break;
    case '2':
        data = DISP_A | DISP_B | DISP_G | DISP_E | DISP_D;
        break;
    case '3':
        data = DISP_A | DISP_B | DISP_C | DISP_D | DISP_G;
        break;
    case '4':
        data = DISP_F|DISP_G|DISP_B|DISP_C;
        break;
    case '5':
        data = DISP_A|DISP_F|DISP_G|DISP_C|DISP_D;
        break;
    case '6':
        data = DISP_A|DISP_F|DISP_E|DISP_D|DISP_C|DISP_G;
        break;
    case '7':
        data = DISP_A|DISP_B|DISP_C;
        break;
    case '8':
        data = DISP_A|DISP_B|DISP_C|DISP_D|DISP_E|DISP_F|DISP_G;
        break;
    case '9':
        data = DISP_A|DISP_B|DISP_C|DISP_D|DISP_F|DISP_G;
        break;
    case '-':
        data = DISP_G;
        break;
    case 'A':
    case 'a':
        data = DISP_A | DISP_B | DISP_C | DISP_F | DISP_E | DISP_G;
        break;
    case 'B':
    case 'b':
        data = DISP_F | DISP_E | DISP_G | DISP_C | DISP_D;
        break;
    case 'C':
    case 'c':
        data = DISP_A | DISP_F | DISP_E | DISP_D;
        break;
    case 'D':
    case 'd':
        data = DISP_B | DISP_C | DISP_D | DISP_E | DISP_G;
        break;
    case 'e':
    case 'E':
        data = DISP_A | DISP_G | DISP_D | DISP_F | DISP_E;
        break;
    case 'f':
    case 'F':
        data = DISP_A | DISP_G | DISP_F | DISP_E;
        break;
    case ' ':
    default:
        data = 0x00; // empty space
    }
    return data;
}

void pt6961_send(PT6961_Init* pt, unsigned char data)
{
    unsigned char i;
    for(i=0; i<8; i++)
    {
        if(data & 0x01)
        {
            /* /\* set_gpio(pt->DIN_port, pt->DIN_pin); *\/ */
            /* set_gpio(GPIOC,10); // DIN */
            gpiopin_set(pt->DIN);
        }
        else
        {
            gpiopin_clear(pt->DIN);
            /* clear_gpio(pt->DIN_port, pt->DIN_pin); */
            /* clear_gpio(GPIOC,10); // DIN */
        }
        data >>=0x01;
        /* blink clock */
        gpiopin_clear(pt->CLK);
        gpiopin_set(pt->CLK);
        
        /* clear_gpio(pt->CLK_port, pt->CLK_pin); */
        /* clear_gpio(GPIOC,7); // CLK */
        /* set_gpio(pt->CLK_port, pt->CLK_pin); */
        /* set_gpio(GPIOC,7); //clk */
    }
}

void pt6961_init(PT6961_Init* pt)
{
    /* Setting output mode for STB, CLK and DIN */

    pt->CLK.port->MODER |= 1 << (pt->CLK.pin * 2);
    pt->STB.port->MODER |= 1 << (pt->STB.pin * 2);
    pt->DIN.port->MODER |= 1 << (pt->DIN.pin * 2);

    /* Setting input mode for DOUT */
    pt->DOUT.port->MODER &= ~(1 << (pt->DOUT.pin*2));
    /* GPIO_InitTypeDef GPIO_InitStructure; */
    
    /* GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; */
    /* GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; */
    /* GPIO_InitStructure.GPIO_Pin = pt->DOUT_pin; */
    /* GPIO_Init(pt->DOUT_port, &GPIO_InitStructure); */

    gpiopin_clear(pt->DIN);
    gpiopin_set(pt->CLK);
    gpiopin_set(pt->STB);
    /* clear_gpio(pt->DIN_port, pt->DIN_pin); // DIN */
    /* set_gpio(pt->CLK_port, pt->CLK_pin); //clk */
    /* set_gpio(pt->STB_port, pt->STB_pin); //stb */

    DelayMs(30);

    gpiopin_clear(pt->STB);
    /* clear_gpio(pt->STB_port, pt->STB_pin); */
    pt6961_send(pt, 0b01000000);
    gpiopin_set(pt->STB);
    /* set_gpio(pt->STB_port, pt->STB_pin); */

    gpiopin_clear(pt->STB);
    /* clear_gpio(pt->STB_port, pt->STB_pin); */
    pt6961_send(pt, 0b11000000);

    unsigned char i;
    for(i=0; i<13; i++)
    {
        pt6961_send(pt, 0x0);
    }
    gpiopin_set(pt->STB);
    /* set_gpio(pt->STB_port, pt->STB_pin); */

    gpiopin_clear(pt->STB);
    /* clear_gpio(pt->STB_port, pt->STB_pin); */
    pt6961_send(pt, 0b00000010);
    gpiopin_set(pt->STB);
    /* set_gpio(pt->STB_port, pt->STB_pin); */

    gpiopin_clear(pt->STB);
    /* clear_gpio(pt->STB_port, pt->STB_pin); */
    pt6961_send(pt, 0b10001100);
    gpiopin_set(pt->STB);
    /* set_gpio(pt->STB_port, pt->STB_pin); */
}

void pt6961_print(PT6961_Init* pt, const char* str)
{
    gpiopin_clear(pt->STB);
    /* clear_gpio(GPIOC,6);//stb */
    
    pt6961_send(pt, 0b01000000); // select write mode.
    pt6961_send(pt, 0b11000000); // set address to the beginning.
    
    unsigned char i;
    for(i=0; i<PT_LEN; i++)
    {
        if(str[i] == '\0')
            break;
        pt6961_send(pt, char2segment(str[i]));
        pt6961_send(pt, 0xFF); // there are more segments available, so we omit the second byte.
    }
    for(; i < PT_LEN; i++)
    {
        pt6961_send(pt, 0x00);
        pt6961_send(pt, 0x00);
    }
    pt6961_send(pt, 0x00);

    gpiopin_set(pt->STB);
    /* set_gpio(GPIOC,6);//stb */
}

void pt6961_update(PT6961_Init* pt)
{
    gpiopin_clear(pt->STB);
    /* clear_gpio(GPIOC,6);//stb */
    pt6961_send(pt, 0b01000000); // select write mode.
    pt6961_send(pt, 0b11000000); // set address to the beginning.
    
    unsigned char i;
    for(i=0; i<PT_LEN; i++)
    {
        if(pt->value[i] == '\0')
            break;
        pt6961_send(pt, char2segment(pt->value[i]));
        pt6961_send(pt, 0xFF); // there are more segments available, so we omit the second byte.
    }
    for(; i < PT_LEN; i++)
    {
        pt6961_send(pt, 0x00);
        pt6961_send(pt, 0x00);
    }
    pt6961_send(pt, 0x00);

    gpiopin_set(pt->STB);
    
    /* set_gpio(GPIOC,6);//stb */
}

uint32_t pt6961_read(PT6961_Init* pt)
{
    gpiopin_clear(pt->STB);
    /* clear_gpio(GPIOC,6); //stb */
    gpiopin_set(pt->CLK);
    /* set_gpio(GPIOC,7); //clk */
    unsigned char data = 0;
    pt6961_send(pt, 0b01000110);
    delay(1000);
    
    /* Read key matrix state. */
    data = 0;
    unsigned char j;
    for(j = 0; j < 8; j++)
    {
        gpiopin_clear(pt->CLK);
        /* clear_gpio(GPIOC,7); //clk */
            
        data = (data << 1) | ((!(GPIOC->IDR & (1 << 12))) & 0x01);

        gpiopin_set(pt->CLK);
        /* set_gpio(GPIOC,7); */
    }
    /* data = (data << 2) >> 2; */
    data &= ~(0b00000011);
    gpiopin_set(pt->STB);
    /* set_gpio(GPIOC, 6); */
    if(data == KEY_NONE)
    {
        return 0;
    }
    else
    {
        if(pt->handler != 0)
        {
            pt->handler(pt, data);
        }
        return data;
    }
}
