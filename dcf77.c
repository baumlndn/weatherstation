/*
 * dcf77.c
 *
 *  Created on: 30.11.2017
 *      Author: baumlndn
 */

#include "dcf77.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "config.h"
#include "usart.h"
#include "ds3231.h"

uint8_t cnt_second;
volatile ds3231_datetime_t dcf77_data[3];
volatile uint8_t sec0found = 0;
volatile uint8_t data_valid = 0;
volatile uint8_t dcf_retries = 0;

void DCF77_Init(void)
{
	TCCR1B = ((1 << ICNC1) | (1 << ICES1) | (1 << CS12) | (1 << CS10));
}

uint8_t DCF77_Sync(ds3231_datetime_t * dcf_ret)
{
	cli();
	TCNT1 = 0;
	TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
	TIFR1 = (1<<ICF1) | (1<<TOV1);
	sec0found = 0xFF;
	data_valid = 0;
	sei();

	while (sec0found != 4)
	{

	}

	TIMSK1 = 0x00;

	USART_Transmit(data_valid);

	if (data_valid != 0x00)
	{
		dcf_ret->second = 0;
		dcf_ret->minute = dcf77_data[data_valid].minute;
		dcf_ret->hour = dcf77_data[data_valid].hour;
		dcf_ret->day = dcf77_data[data_valid].day;
		dcf_ret->month = dcf77_data[data_valid].month;
		dcf_ret->year = dcf77_data[data_valid].year;
		data_valid = 1;
	}
	return data_valid;
}

ISR(TIMER1_CAPT_vect)
{
	uint16_t tmp_u16 = ICR1;

	cli();

	if (tmp_u16 > 0x2700)
	{
		cnt_second = 0;
		sec0found = sec0found + 1;
	}
	else
	{
		cnt_second++;
	}

	_delay_ms(150);

//	USART_Transmit(sec0found);
//	USART_Transmit(((cnt_second / 10)<<4) | (cnt_second % 10));

	if (PINB & (1 <<PINB0))
	{
		switch (cnt_second)
		{
		case 21:
			dcf77_data[sec0found].minute = 1;
			break;
		case 22:
			dcf77_data[sec0found].minute |= 0x02;
			break;
		case 23:
			dcf77_data[sec0found].minute |= 0x04;
			break;
		case 24:
			dcf77_data[sec0found].minute |= 0x08;
			break;
		case 25:
			dcf77_data[sec0found].minute |= 0x10;
			break;
		case 26:
			dcf77_data[sec0found].minute |= 0x20;
			break;
		case 27:
			dcf77_data[sec0found].minute |= 0x40;
			break;
		case 29:
			dcf77_data[sec0found].hour = 0x01;
			break;
		case 30:
			dcf77_data[sec0found].hour |= 0x02;
			break;
		case 31:
			dcf77_data[sec0found].hour |= 0x04;
			break;
		case 32:
			dcf77_data[sec0found].hour |= 0x08;
			break;
		case 33:
			dcf77_data[sec0found].hour |= 0x10;
			break;
		case 34:
			dcf77_data[sec0found].hour |= 0x20;
			break;
		case 36:
			dcf77_data[sec0found].day = 0x01;
			break;
		case 37:
			dcf77_data[sec0found].day |= 0x02;
			break;
		case 38:
			dcf77_data[sec0found].day |= 0x04;
			break;
		case 39:
			dcf77_data[sec0found].day |= 0x08;
			break;
		case 40:
			dcf77_data[sec0found].day |= 0x10;
			break;
		case 41:
			dcf77_data[sec0found].day |= 0x20;
			break;
		case 45:
			dcf77_data[sec0found].month = 0x01;
			break;
		case 46:
			dcf77_data[sec0found].month |= 0x02;
			break;
		case 47:
			dcf77_data[sec0found].month |= 0x04;
			break;
		case 48:
			dcf77_data[sec0found].month |= 0x08;
			break;
		case 49:
			dcf77_data[sec0found].month |= 0x10;
			break;
		case 50:
			dcf77_data[sec0found].year = 0x01;
			break;
		case 51:
			dcf77_data[sec0found].year |= 0x02;
			break;
		case 52:
			dcf77_data[sec0found].year |= 0x04;
			break;
		case 53:
			dcf77_data[sec0found].year |= 0x08;
			break;
		case 54:
			dcf77_data[sec0found].year |= 0x10;
			break;
		case 55:
			dcf77_data[sec0found].year |= 0x20;
			break;
		case 56:
			dcf77_data[sec0found].year |= 0x40;
			break;
		case 57:
			dcf77_data[sec0found].year |= 0x80;
			break;
		default:
			break;
		}
//		USART_Transmit(0x01);
	}
	else
	{
		switch (cnt_second)
		{
		case 21:
			dcf77_data[sec0found].minute = 0x00;
			break;
		case 29:
			dcf77_data[sec0found].hour = 0x00;
			break;
		case 36:
			dcf77_data[sec0found].day = 0x00;
			break;
		case 45:
			dcf77_data[sec0found].month = 0x00;
			break;
		case 50:
			dcf77_data[sec0found].year = 0x00;
			break;
		default:
			break;
		}

//		USART_Transmit(0x00);
	}

	/* Check if minute is 59 and skip measurement */
	if ((cnt_second == 0) && (sec0found == 2))
	{
		if (dcf77_data[1].minute == 0x59)
		{
			sec0found = 1;
			dcf_retries++;

			if (dcf_retries > 2)
			{
				data_valid = 0;
				sec0found = 4;
			}
		}
	}

	/* Check if data is consistent between both datasets 0 and 1*/
	if ((cnt_second == 0) && (sec0found == 2))
	{
		USART_Transmit(dcf77_data[0].day);
		USART_Transmit(dcf77_data[0].month);
		USART_Transmit(dcf77_data[0].year);
		USART_Transmit(dcf77_data[0].hour);
		USART_Transmit(dcf77_data[0].minute);

		USART_Transmit(dcf77_data[1].day);
		USART_Transmit(dcf77_data[1].month);
		USART_Transmit(dcf77_data[1].year);
		USART_Transmit(dcf77_data[1].hour);
		USART_Transmit(dcf77_data[1].minute);

		if (
				(dcf77_data[0].day == dcf77_data[1].day) &&
				(dcf77_data[0].month == dcf77_data[1].month) &&
				(dcf77_data[0].year == dcf77_data[1].year) &&
				(dcf77_data[0].hour == dcf77_data[1].hour) &&
				(dcf77_data[0].minute == (dcf77_data[1].minute - 0x01))
			)
		{
			data_valid = 1;
			sec0found = 4;
		}
	}

	/* Check if data is consistent between both datasets 0 and 1 and 2*/
	if ((cnt_second == 0) && (sec0found == 3))
	{
		USART_Transmit(dcf77_data[0].day);
		USART_Transmit(dcf77_data[0].month);
		USART_Transmit(dcf77_data[0].year);
		USART_Transmit(dcf77_data[0].hour);
		USART_Transmit(dcf77_data[0].minute);

		USART_Transmit(dcf77_data[1].day);
		USART_Transmit(dcf77_data[1].month);
		USART_Transmit(dcf77_data[1].year);
		USART_Transmit(dcf77_data[1].hour);
		USART_Transmit(dcf77_data[1].minute);

		USART_Transmit(dcf77_data[2].day);
		USART_Transmit(dcf77_data[2].month);
		USART_Transmit(dcf77_data[2].year);
		USART_Transmit(dcf77_data[2].hour);
		USART_Transmit(dcf77_data[2].minute);

		if (
				(dcf77_data[0].day == dcf77_data[2].day) &&
				(dcf77_data[0].month == dcf77_data[2].month) &&
				(dcf77_data[0].year == dcf77_data[2].year) &&
				(dcf77_data[0].hour == dcf77_data[2].hour) &&
				(dcf77_data[0].minute == (dcf77_data[2].minute - 0x02))
			)
		{
			data_valid = 2;
			sec0found = 4;
		}

		if (
				(dcf77_data[1].day == dcf77_data[2].day) &&
				(dcf77_data[1].month == dcf77_data[2].month) &&
				(dcf77_data[1].year == dcf77_data[2].year) &&
				(dcf77_data[1].hour == dcf77_data[2].hour) &&
				(dcf77_data[1].minute == (dcf77_data[2].minute - 0x01))
			)
		{
			data_valid = 2;
			sec0found = 4;
		}
	}

	TCNT1 = 0;
	sei();
}

/* Overflow in case DCF is not active or present */
ISR(TIMER1_OVF_vect)
{
	data_valid = 0;
	sec0found = 4;
}
