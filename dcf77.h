/*
 * dcf77.h
 *
 *  Created on: 30.11.2017
 *      Author: baumlndn
 */

#ifndef DCF77_H_
#define DCF77_H_

#include "ds3231.h"

void DCF77_Init(void);
uint8_t DCF77_Sync(ds3231_datetime_t * dcf_ret);

#endif /* DCF77_H_ */
