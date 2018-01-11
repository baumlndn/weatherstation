/*
 * main.c
 *
 *  Created on: 11.02.2015
 *      Author: baumlndn
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "config.h"
#include "usart.h"
#include "nrf24l01p.h"
#include "bme280.h"
#include "ds3231.h"
#include "powersave.h"
#include "dcf77.h"


int main()
{
	uint8_t nrf_data_buffer[NRF24L01P_PW];
	ds3231_datetime_t dcf_time;

	/* ToDo: reset to 0 after testing */
	uint8_t rtc_dcf_init = 0;

	/* Sensor data pointer */
	int32_t * temperature_data_ptr = (int32_t *) &nrf_data_buffer[6];
	uint32_t * pressure_data_ptr = (uint32_t *) &nrf_data_buffer[10];
	uint32_t * humidity_data_ptr = (uint32_t *) &nrf_data_buffer[14];

	/* Initialize USART */
	USART_Init(51u);

	/* Initialize NRF24L01+ */
	NRF24L01P_Init();

	/* Initialize BME280 */
	BME280_Init();

	/* Initialize DS3231 RTC */
	DS3231_Init();

	/* Initialize DCF77 */
	DCF77_Init();

	/* Initialize PowerSave */
	PowerSave_Init();

	char tmp_welcome[] = "Hallo";
	for (uint8_t i=0;i<strlen(tmp_welcome);i++)
	{
		USART_Transmit(tmp_welcome[i]);
	}

	USART_Transmit(0x0D);
	USART_Transmit(0x0A);

	/* Start sleep timer */
	PowerSave_StartTimer_s(600);

	while (1)
	{
		DS3231_printDateTime();
//		BME280_printData();

		DS3231_getDate(&nrf_data_buffer[0]);
		BME280_measure(temperature_data_ptr, pressure_data_ptr, humidity_data_ptr);

		NRF24L01P_WakeUp();
		NRF24L01P_Proc();
		NRF24L01P_Transmit((char * )&nrf_data_buffer[0]);

		NRF24L01P_Proc();
		NRF24L01P_Sleep();

		/* sync time after power-up or after 02.00 AM */
		if (rtc_dcf_init == 0 || ( (nrf_data_buffer[3] == 0x02) && (nrf_data_buffer[4]) < 0x10 ))
		{
			rtc_dcf_init = 1;
			uint8_t dcf_valid = 0;
			dcf_valid = DCF77_Sync(&dcf_time);

			if (dcf_valid == 1)
			{
				DS3231_setDate(&dcf_time);
			}
		}

		/* Put ATMeag to sleep */
		PowerSave_Sleep();

		/* ATMega will wake up here */
		PowerSave_StartTimer_s(600);

	}
	
	return 0;
}
