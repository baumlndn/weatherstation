/*
 * ds3231.h
 *
 *  Created on: 25.11.2017
 *      Author: baumlndn
 */

#ifndef DS3231_H_
#define DS3231_H_

#include <avr/io.h>

#define DS3231_I2C_ADDR		0x68

typedef struct
{
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} ds3231_datetime_t;

void DS3231_Init( void );
void DS3231_readDate (void);
void DS3231_printDateTime (void);
void DS3231_getDate(uint8_t *data_ptr);
void DS3231_setDate (ds3231_datetime_t * dcf_time);

#endif /* DS3231_H_ */
