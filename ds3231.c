/*
 * ds1307.c
 *
 *  Created on: 25.11.2017
 *      Author: baumlndn
 */


#include <avr/io.h>
#include <util/delay.h>
#include "config.h"
#include "ds3231.h"
#include "i2c.h"
#include "usart.h"

ds3231_datetime_t date_rtc;

void DS3231_Init ( void )
{
	uint8_t rtc_data[8];
	uint8_t i2c_status;

	I2C_Init(I2C_TWBR);
	_delay_ms(500);

	i2c_status = I2C_Receive(DS3231_I2C_ADDR,0x0F,&rtc_data[0],1);

//	USART_Transmit(i2c_status);
	if (i2c_status == 0)
	{
//		USART_Transmit(rtc_data[0]);
	}

	rtc_data[0] = 0x00;
	i2c_status = I2C_Transmit(DS3231_I2C_ADDR,0x0E,&rtc_data[0],1);
	USART_Transmit(i2c_status);
	if (i2c_status == 0)
	{
//		USART_Transmit(rtc_data[0]);
	}
}

void DS3231_readDate (void)
{
	uint8_t rtc_data[8];
	uint8_t i2c_status;

	i2c_status = I2C_Receive(DS3231_I2C_ADDR,0x00,&rtc_data[0],8);

	if (i2c_status == 0)
	{
		date_rtc.second 	= rtc_data[0];
		date_rtc.minute 	= rtc_data[1];
		date_rtc.hour	= rtc_data[2];
		date_rtc.day 	= rtc_data[4];
		date_rtc.month 	= rtc_data[5];
		date_rtc.year 	= rtc_data[6];
	}
}

void DS3231_setDate (ds3231_datetime_t * dcf_time)
{
	uint8_t rtc_data[7];

	rtc_data[0] = dcf_time->second;
	rtc_data[1] = dcf_time->minute;
	rtc_data[2] = dcf_time->hour;
	rtc_data[3] = 1;
	rtc_data[4] = dcf_time->day;
	rtc_data[5] = dcf_time->month;
	rtc_data[6] = dcf_time->year;

	(void) I2C_Transmit(DS3231_I2C_ADDR,0x00,&rtc_data[0],7);
//	USART_Transmit(i2c_status);
}

void DS3231_getDate(uint8_t *data_ptr)
{
	DS3231_readDate();

	data_ptr[0] = date_rtc.day;
	data_ptr[1] = date_rtc.month;
	data_ptr[2] = date_rtc.year;
	data_ptr[3] = date_rtc.hour;
	data_ptr[4] = date_rtc.minute;
	data_ptr[5] = date_rtc.second;
}

void DS3231_printDateTime (void)
{
	DS3231_readDate();

	USART_Transmit((date_rtc.day >> 4 ) + 0x30);
	USART_Transmit((date_rtc.day & 0x0F ) + 0x30);
	USART_Transmit('.');
	USART_Transmit((date_rtc.month >> 4 ) + 0x30);
	USART_Transmit((date_rtc.month & 0x0F ) + 0x30);
	USART_Transmit('.');
	USART_Transmit((date_rtc.year >> 4 ) + 0x30);
	USART_Transmit((date_rtc.year & 0x0F ) + 0x30);
	USART_Transmit(' ');
	USART_Transmit((date_rtc.hour >> 4 ) + 0x30);
	USART_Transmit((date_rtc.hour & 0x0F ) + 0x30);
	USART_Transmit(':');
	USART_Transmit((date_rtc.minute >> 4 ) + 0x30);
	USART_Transmit((date_rtc.minute & 0x0F ) + 0x30);
	USART_Transmit(':');
	USART_Transmit((date_rtc.second >> 4 ) + 0x30);
	USART_Transmit((date_rtc.second & 0x0F ) + 0x30);

	USART_Transmit(0x0D);
	USART_Transmit(0x0A);
}
