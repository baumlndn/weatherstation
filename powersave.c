/*
 * powersave.c
 *
 *  Created on: 15.02.2015
 *      Author: baumlndn
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "powersave.h"
#include "config.h"
#include "usart.h"

void PowerSave_Init(void)
{
_delay_ms(3000);

    //Disable timer2 interrupts
    TIMSK2  = 0;
    //Enable asynchronous mode
    ASSR  = (1<<AS2) | (1<<EXCLK);
    //set initial counter value
    TCNT2 = 0;
    //set prescaler 8
    TCCR2B = (0<<CS22)|(1<<CS21)|(0<<CS20);
    //wait for registers update
    while (ASSR & 0x1F);
    //clear interrupt flags
    TIFR2  = (1<<TOV2);
    //enable TOV2 interrupt
    TIMSK2  = (1<<TOIE2);

//    USART_Transmit(TIMSK2);
//    USART_Transmit(ASSR);
//    USART_Transmit(TCNT2);
//    USART_Transmit(TCCR2B);
//    USART_Transmit(TIFR2);
}

void PowerSave_StartTimer_s( uint16_t sec)
{
	/* Timer 2 is configured to 8 sec intervals */
	uint8_t ticks = (uint8_t) ((sec) >> 3);

	sei();

	/* Configure sleep mode register to Power-Save mode */
    SMCR |= ( (1<<SM1) | (1<<SM0) );

    /* Reset Timer2 count */
    TCNT2 = (uint8_t) (256-ticks);

    while (ASSR & (1<<TCN2UB));
}

void PowerSave_Sleep( void )
{
	sei();
	_delay_ms(500);
	/* Activate sleep */
    SMCR |= (1<<SE);
    asm volatile("sleep"::);

    /* Controller goes to sleep here and will wake back up at this position */
    asm volatile("nop"::);

    /* Deactivate sleep mode */
    SMCR&=~(1<<SE);

	/* Configure sleep mode register to idle mode */
	SMCR &= ~( (1<<SM1) | (1<<SM0) );
}

ISR(TIMER2_OVF_vect)
{
	asm volatile("nop"::);
}
