/**
 * @file   gpiopin.c
 * @author Wiktor Gołgowski <wgolgowski@gmail.com>
 * @date   Fri Jan 30 01:01:24 2015
 * 
 * @brief  Basic GPIO helper implementation.
 * 
 */

#include "gpiopin.h"

GPIOPin gpiopin(GPIO_TypeDef* port, uint32_t pin)
{
    GPIOPin tmp;
    tmp.port = port;
    tmp.pin = pin;
    return tmp;
}

