#ifndef GPIOPIN_H
#define GPIOPIN_H
/**
 * @file   gpiopin.h
 * @author Wiktor Gołgowski <wgolgowski@gmail.com>
 * @date   Fri Jan 30 01:00:44 2015
 * 
 * @brief  Basic low-level GPIO helpers.
 * 
 */


#include "stm32f0xx.h"

#define gpio_set(port, pin) port->BSRR = 1 << (pin)
#define gpio_clear(port, pin) port->BRR = 1 << (pin)
#define gpio_get(port, pin) ((!((port)->IDR & (1 << (pin)))))
#define gpiopin_set(mypin) (mypin).port->BSRR = 1<<((mypin).pin)
#define gpiopin_clear(mypin) (mypin).port->BRR = 1<<((mypin).pin)

/// This struct defines GPIO pin, which uses GPIO port address pointer and number of the pin.
typedef struct strGPIOPin
{
    GPIO_TypeDef *port;
    uint32_t pin;
} GPIOPin;

GPIOPin gpiopin(GPIO_TypeDef* port, uint32_t pin);

#endif /* GPIOPIN_H */
