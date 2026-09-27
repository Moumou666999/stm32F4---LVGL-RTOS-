#ifndef __SHT30_H
#define __SHT30_h

#include "i2c.h"
#define SHT30_ADDRESS 0x44 << 1 // SHT30 I2CµØÖ·

typedef struct
{
    float temp;
    float humi;
} Sht30Data;

Sht30Data Sht30_Getdata(void);

uint8_t Sht30_Reset(void);

#endif
