#ifndef PT6961_H
#define PT6961_H
/**
 * @file   pt6961.h
 * @author Wiktor Gołgowski <wgolgowski@gmail.com>
 * @date   Fri Jan 30 00:57:56 2015
 * 
 * @brief  Header file for PT6961 display library implementation.
 * 
 */


#include "gpiopin.h"

/// Six characters available for display.
#define PT_LEN 6 

/** 
 * This structure keeps information about GPIO connections and
 * currently displayed value. Also, it could be initialized with
 * function pointer to keyboard handler.
 * 
 */

struct strPT6961_Init;

/// Type of function pointer, which must be defined to receive button calls.
typedef unsigned char (*pt_keyhandler)(struct strPT6961_Init*, unsigned char key);

typedef struct strPT6961_Init
{
    GPIOPin DIN;
    GPIOPin DOUT;
    GPIOPin CLK;
    GPIOPin STB;
    unsigned char value[PT_LEN+1];
    pt_keyhandler handler;
} PT6961_Init;

/**
 * Six keys with codes defined. The last one acts as terminator, if
 * no key is read.
 */
enum
{
    KEY_ESC = 124,
    KEY_OK = 244,
    KEY_UP = 188,
    KEY_DOWN = 236,
    KEY_START = 220,
    KEY_STOP = 248,
    KEY_NONE = 252
};

/** 
 * This function initializes the display, but keeps it turned off.
 * 
 * @param pt pointer to PT6961 configuration structure
 */
void pt6961_init(PT6961_Init* pt);

/** 
 * This function sends a single byte to the display.
 * 
 * @param pt pointer to PT6961 configuration structure
 * @param data byte to send 
 */
void pt6961_send(PT6961_Init* pt, unsigned char data);

/** 
 * This function refreshes the display, using the string kept in the
 * configuration structure.
 * 
 * @param pt pointer to PT6961 configuration structure
 */
void pt6961_update(PT6961_Init* pt);

/** 
 * This function returns key pressed on interface board. If there is a
 * callback given in configuration, it is called.
 * 
 * @param pt pointer to PT6961 configuration structure
 * 
 * @return key pressed, zero in no key
 */
uint32_t pt6961_read(PT6961_Init* pt);

/** 
 * This function prints given string on the display. It is displayed
 * only till next call of the pt6961_update, not persistent.
 * 
 * @param pt pointer to PT6961 configuration structure
 * @param data data to display
 */
void pt6961_print(PT6961_Init* pt, const char* data);

/** 
 * This function sets given string as display contents. Triggers
 * pt6961_update.
 * 
 * @param pt pointer to PT6961 configuration structure
 * @param data data to display
 */
void pt6961_set(PT6961_Init* pt, const char* data);

/** 
 * This function turns the display on.
 * 
 * @param pt pointer to PT6961 configuration structure
 */
void pt6961_on(PT6961_Init* pt);

/** 
 * This function turns the display off.
 * 
 * @param pt pointer to PT6961 configuration structure
 */
void pt6961_off(PT6961_Init* pt);

#endif /* PT6961_H */
