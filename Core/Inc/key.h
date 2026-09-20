#ifndef __KEY_H
#define __KEY_H

#include "gpio.h"

typedef enum
{
    KEY_NONE = 0,
    KEY_1,
    KEY_2
} KeyEvent_t;

KeyEvent_t Key_GetEvent(void);

#endif
