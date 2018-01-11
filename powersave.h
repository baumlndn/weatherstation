/*
 * powersave.h
 *
 *  Created on: 15.02.2015
 *      Author: baumlndn
 */

#ifndef POWERSAVE_H_
#define POWERSAVE_H_

#include <avr/io.h>

void PowerSave_Init( void );
void PowerSave_StartTimer_s( uint16_t sec);
void PowerSave_Sleep( void );


#endif /* POWERSAVE_H_ */
