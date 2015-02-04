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


static __IO uint32_t DelayCounter; /* for busy wait */
static __IO uint32_t phase_counter; /* for phase change */
static __IO uint32_t blink_counter; /* for display blinking */
static __IO uint32_t key_debouncer; /* for key presses management */


#define PHASES 6
#define BOUNCER 200
#define BLINK_MS 300


typedef struct strEngine
{
    uint32_t state;
    uint32_t rotation;
    uint32_t requested_rotation;
    uint32_t duration;
    unsigned char phase; /* 0-5, 60° phase */
    unsigned char direction; /* 1 - right, 0 - left */
    unsigned char requested_direction;
    unsigned char started;
    unsigned char fault_overcurrent;
    
} Engine;

Engine engine;

void engine_next_phase_cb()
{
    if(engine.direction == 1) //right
    {
        if(engine.phase < PHASES-1)
        {
            engine.phase++;
        }
        else
        {
            engine.phase = 0;
        }
    }
    else
    {
        if(engine.phase == 0)
        {
            engine.phase = PHASES-1;
        }
        else
        {
            engine.phase--;
        }
    }
}

void engine_set_phase_delay(uint32_t delay)
{
    phase_counter = delay;
}

void DelayMs_Decrement(void)
{
    if(DelayCounter != 0x00)
    {
        DelayCounter--;
    }

    if(phase_counter != 0)
    {
        phase_counter--;
    }
    else
    {
        engine_next_phase_cb();
        phase_counter = engine.duration;
    }

    if(key_debouncer != 0)
    {
        key_debouncer--;
    }

    if(blink_counter == 0)
    {
        blink_counter = BLINK_MS;
    }
    else
    {
        blink_counter--;
    }
    
}

/// Configuration for the transistors (last two bits do not matter)
unsigned char phase_configuration[PHASES] = {0b00100001, 0b00000011, 0b00000110, 0b00001100, 0b00011000, 0b00110000};
GPIOPin T[PHASES];

void engine_init_pins()
{
    T[0] = gpiopin(GPIOC, 6);
    T[1] = gpiopin(GPIOC, 7);
    T[2] = gpiopin(GPIOC, 8);
    T[3] = gpiopin(GPIOC, 9);
    T[4] = gpiopin(GPIOA, 8);
    T[5] = gpiopin(GPIOA, 9);
    /* Configure them as outputs: */
    T[0].port->MODER |= 1 << (T[0].pin * 2);
    T[1].port->MODER |= 1 << (T[1].pin * 2);
    T[2].port->MODER |= 1 << (T[2].pin * 2);
    T[3].port->MODER |= 1 << (T[3].pin * 2);
    T[4].port->MODER |= 1 << (T[4].pin * 2);
    T[5].port->MODER |= 1 << (T[5].pin * 2);

    
}

void engine_set_pins_to_phase(unsigned char phase)
{
    unsigned char cfg;
    if(engine.rotation == 0)
    {
        cfg = 0;
    }
    else
    {
        cfg = phase_configuration[phase];
    }
    
    unsigned char i;
    for(i = 0; i < PHASES; i++)
    {
        if((cfg & (1 << i)) == 0)
        {
            gpiopin_clear(T[i]);
        }
        else
        {
            gpiopin_set(T[i]);
        }
    }
}

void DelayMs(__IO uint32_t ms)
{
    DelayCounter = ms;
    while(DelayCounter != 0x00);
}



void delay (int a);

unsigned char key_handler(PT6961_Init* pt, unsigned char key)
{
    key = pt6961_read(pt);
    if(key_debouncer > 0)
    {
        return KEY_NONE;
    }
    else
    {
        key_debouncer = BOUNCER;
        return key;
    }
}

unsigned char gpio_reader(GPIO_TypeDef* port, uint32_t pin)
{
    unsigned char data = gpio_get(port, pin);
    if(key_debouncer > 0)
    {
        return 0;
    }
    else
    {
        key_debouncer = BOUNCER;
        return data;
    }
}

#define ROT_MAX 1000
#define ROT_MIN 0

void handle_menu(PT6961_Init* pt, unsigned char key)
{
    char disp_mode_max = 9;
    char prog_mode_max = 2;
    static uint32_t selected_rotation = 0;
    static unsigned char selected_direction = 0;
    static unsigned char cur_id = 0; /* Current parameter ID. */
    static unsigned char mode = 0; /* Current mode. 0 - display, 1 - program */
    if(mode == 0)
    {
        switch(key)
        {
        case KEY_ESC:
            mode = 1;
            cur_id = 0;
            return;

        case KEY_UP:
            if(cur_id < disp_mode_max)
                cur_id++;
            else
                cur_id = 0;
            break;

        case KEY_DOWN:
            if(cur_id == 0)
                cur_id = disp_mode_max;
            else
                cur_id--;
            break;

        case KEY_START:
            if(engine.requested_rotation == 0)
            {
                engine.requested_rotation = selected_rotation;
            }
            if(engine.requested_rotation > 0)
                engine.started = 1;
            break;

        case KEY_STOP:
            engine.started = 0;
            engine.fault_overcurrent = 0;
            break;
            
        case KEY_OK:
            engine.requested_direction = !engine.requested_direction;
            break;
            
        default: /* KEY_NONE */
            break;
        }
        char st = 'd'; // for display
        if(engine.fault_overcurrent == 1)
        {
            st = 'f'; // for fault signalization
        }
        switch(cur_id)
        {
        case 0: /* current rotation */
            snprintf(pt->value, PT_LEN+1, "%c%d%d", st, cur_id, engine.rotation);
            pt6961_update(pt);
            break;
        case 1: /* U phase voltage */
            snprintf(pt->value, PT_LEN+1, "%c%d%d", st, cur_id, 230);
            pt6961_update(pt);
            break;
        case 2: /* V phase voltage */
            snprintf(pt->value, PT_LEN+1, "%c%d%d", st, cur_id, 229);
            pt6961_update(pt);
            break;
        case 3: /* W phase voltage */
            snprintf(pt->value, PT_LEN+1, "%c%d%d", st, cur_id, 235);
            pt6961_update(pt);
            break;
        case 4: /* U phase current */
            if(engine.started)
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "10.9");
            else
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "  0");
            pt6961_update(pt);
            break;
        case 5: /* V phase current */
            if(engine.started)
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "11.1");
            else
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "  0");
            pt6961_update(pt);
            break;
        case 6: /* W phase current */
            if(engine.started)
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "11.3");
            else
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "  0");
            pt6961_update(pt);
            break;
        case 7: /* Last uncleared fault */
            if(engine.fault_overcurrent)
            {
                
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "1");
            }
            else
            {
                snprintf(pt->value, PT_LEN+1, "%c%d%s", st, cur_id, "0");
            }
            pt6961_update(pt);
            break;
        case 8: /* Direction */
            snprintf(pt->value, PT_LEN+1, "%c%d%c", st, cur_id, engine.direction?'r':'l');
            pt6961_update(pt);
            break;

        case 9: /* Requested rotation */
            snprintf(pt->value, PT_LEN+1, "%c%d%d", st, cur_id, engine.requested_rotation);
            pt6961_update(pt);
            break;
            
        default:
            break;
        }
    }
    else /* mode == 1 (program) */
    {
        static char program_mode = 0;

        
        switch(cur_id)
        {
        case 0: /* set rotation */
            if(blink_counter < BLINK_MS/2 || program_mode == 0)
            {
                snprintf(pt->value, PT_LEN+1, "p%d%d", cur_id, selected_rotation);
            }
            else
            {
                snprintf(pt->value, PT_LEN+1, "  %d", selected_rotation);
            }
            pt6961_update(pt);
            if(program_mode)
            {
                switch(key)
                {
                case KEY_UP:
                    if(selected_rotation < ROT_MAX)
                        selected_rotation += 10;
                    break;
                case KEY_DOWN:
                    if(selected_rotation > ROT_MIN)
                    {
                        selected_rotation -= 10;
                    }
                    break;
                case KEY_OK:
                    engine.requested_rotation = selected_rotation;
                    program_mode = 0;
                default:
                    break;
                
                }
            }
            else
            {
            
                switch(key)
                {
                case KEY_UP:
                    if(cur_id < prog_mode_max-1)
                        cur_id++;
                    else
                        cur_id = 0;
                    break;

                case KEY_DOWN:
                    if(cur_id > 0)
                        cur_id--;
                    else
                        cur_id = prog_mode_max - 1;
                    break;
                        
                case KEY_OK:
                    program_mode = 1;
                    break;

                case KEY_ESC:
                    mode = 0;
                    cur_id = 0;
                    break;

                default:
                    break;
                        
                }
            }
            break;

        case 1: /* set direction */
            if(blink_counter < BLINK_MS/2 || program_mode == 0)
            {
                snprintf(pt->value, PT_LEN+1, "p%d%c", cur_id, selected_direction?'r':'l');
            }
            else
            {
                snprintf(pt->value, PT_LEN+1, "  %c", selected_direction?'r':'l');
            }
            pt6961_update(pt);
            if(program_mode)
            {
                switch(key)
                {
                case KEY_UP:
                case KEY_DOWN:
                    selected_direction = !selected_direction;
                    break;
                case KEY_OK:
                    engine.requested_direction = selected_direction;
                    program_mode = 0;
                default:
                    break;
                
                }
            }
            else
            {
            
                switch(key)
                {
                case KEY_UP:
                    if(cur_id < prog_mode_max-1)
                        cur_id++;
                    else
                        cur_id = 0;
                    break;

                case KEY_DOWN:
                    if(cur_id > 0)
                        cur_id--;
                    else
                        cur_id = prog_mode_max - 1;
                    break;
                        
                case KEY_OK:
                    program_mode = 1;
                    break;

                case KEY_ESC:
                    mode = 0;
                    cur_id = 0;
                    break;

                default:
                    break;
                        
                }
            }

            break;
        }

    }
    
}


int main(void)
{

  /* GPIOC Periph clock enable */
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

  
    GPIOA->MODER &= ~(1 << (10*2)); /* set PA10 as input. */
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;

   

    /* Initialize SysTick for 1ms window */
    SysTick_Config(SystemCoreClock / 1000);
    
    PT6961_Init pt;
    pt.CLK = gpiopin(GPIOB, 5);
    pt.DIN = gpiopin(GPIOB, 7);
    pt.DOUT = gpiopin(GPIOB, 6);
    pt.STB = gpiopin(GPIOB, 4);
    snprintf(pt.value, PT_LEN+1, "BLDC00");
    pt.handler = 0; //key_handler;
    
    engine_init_pins();
    pt6961_init(&pt);
    pt6961_update(&pt);
    DelayMs(10);

    /* Main program loop */
	while (1)
	{
        /* snprintf(pt.value, PT_LEN+1, "1F%d", engine.requested_rotation); */
        /* pt6961_update(&pt); */
        /* if(!gpio_get(GPIOA, 10)) */
        /*     engine.fault_overcurrent = 1; */
        if(gpio_get(GPIOA, 10))
        {
            engine.fault_overcurrent = 1;
            engine.state = 3;
        }
        
        uint32_t data = key_handler(&pt, 0);
        uint32_t inert_counter = 0;
        static uint32_t rotation_before_reverse;
        static unsigned char first_detected_reverse = 0;
        handle_menu(&pt, data);
        /* Control the engine state: */
        switch(engine.state)
        {    
        case 0: /* init engine */
            engine.phase = 0;
            engine.direction = 0;
            engine.rotation = 0;
            engine.requested_direction = 0;
            engine.requested_rotation = 0;
            engine.started = 0;
            engine.fault_overcurrent = 0;
            engine.state = 1;
            break;
        case 1: /* engine ready */
            if(engine.requested_direction !=  engine.direction)
            {
                if(!first_detected_reverse)
                {
                    rotation_before_reverse = engine.requested_rotation;
                    first_detected_reverse = 1;
                }
                engine.state = 4;
                break;
            }
            if(engine.requested_rotation > 0 && engine.started == 1 && engine.fault_overcurrent == 0)
            {
                engine.duration = 60000 / (engine.rotation * PHASES);
                engine_set_phase_delay(engine.duration);
                engine.state = 2;
            }
            else
            {
                engine.rotation = 0;
            }
            break;

        case 2: /* engine rotating */
            if(engine.started == 0 || engine.fault_overcurrent == 1)
            {
                /* engine.requested_rotation = 0; */
                engine.rotation = 0;
            }
            if(inert_counter == 0)
            {
                if(engine.rotation < engine.requested_rotation)
                {
                    engine.rotation++;
                    engine.state = 1;
                }
                if(engine.rotation > engine.requested_rotation)
                {
                    engine.rotation--;
                    engine.state = 1;
                }
                if(engine.rotation != engine.requested_rotation && engine.rotation % 10 == 0)
                    engine.state = 1;
                inert_counter = 0;
            }
            else
            {
                inert_counter--;
            }
            if(engine.requested_direction !=  engine.direction)
            {
                if(!first_detected_reverse)
                {
                    rotation_before_reverse = engine.requested_rotation;
                    first_detected_reverse = 1;
                }
                engine.state = 4;
                break;
            }
            break;

        case 3: /* engine stopped */
            engine.started = 0;
            engine.requested_rotation = 0;
            engine.rotation = 0;
            engine.state = 1;
            break;
        case 4: /* engine needs to be reversed */
            if(engine.rotation > 0)
            {
                engine.requested_rotation = 0;
                engine.state = 2;
            }
            else
            {
                engine.direction = engine.requested_direction;
                engine.requested_rotation = rotation_before_reverse;
                engine.phase = 0;
                engine.started = 1;
                first_detected_reverse = 0;
                engine.state = 2;
            }
            break;

        default:
            break;
            
        }

        /* Change output pins configuration */
        engine_set_pins_to_phase(engine.phase);
	}
	
	return 0;
}

