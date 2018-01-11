/*
 * nrf24l01p.c
 *
 *  Created on: 05.11.2017
 *      Author: baumlndn
 */


#include "nrf24l01p.h"
#include "usart.h"

uint8_t nrf24l01p_state = NRF_STATE_IDLE;
char * rx_buf_ptr;

void NRF24L01P_Init(void)
{
	char tmp_addr[5] = NRF24L01P_ADDR;
	SPI_MasterInit(NRF24L01P_PL);
	NRF24L01P_DDR_CE |= (1<<NRF24L01P_DD_CE);

#ifdef NRF24L01P_MODE_RX
	NRF24L01P_WriteReg(CONFIG,(1<<EN_CRC)|(1<<PWR_UP)|(1<<PRIM_RX));
#else
	NRF24L01P_WriteReg(CONFIG,(1<<EN_CRC)|(1<<CRCO)|(1<<PWR_UP));
#endif
	NRF24L01P_WriteReg(RF_SETUP,(0<<RF_DR_LOW)|(3<<RF_PWR));
	NRF24L01P_WriteReg(RF_CH,0x4C);
	NRF24L01P_WriteReg(RX_PW_P0,NRF24L01P_PW);
	NRF24L01P_WriteRegAddr(TX_ADDR,&tmp_addr[0]);
	NRF24L01P_WriteRegAddr(RX_ADDR_P0,&tmp_addr[0]);

#ifdef NRF24L01P_INT
	EICRA &= ~(0b11 << ISC00);
	EIMSK |= (1<<INT0);
#endif

#ifdef NRF24L01P_MODE_RX
	/* activate NRF RX */
	NRF24L01P_PORT_CE |= (1<<NRF24L01P_DD_CE);
#endif
}

void NRF24L01P_Sleep(void)
{
	NRF24L01P_WriteReg(CONFIG,(1<<EN_CRC)|(1<<CRCO)|(0<<PWR_UP));
}

void NRF24L01P_WakeUp(void)
{
	NRF24L01P_WriteReg(CONFIG,(1<<EN_CRC)|(1<<CRCO)|(1<<PWR_UP));
	_delay_ms(5);
}

void NRF24L01P_WriteReg(nrf24l01p_reg_e reg, char data)
{
	char tmp_data[2];
	tmp_data[0] = (0x20 | reg);
	tmp_data[1] = data;
	(void) SPI_MasterTransmitMulti(2,&tmp_data[0]);
}

void NRF24L01P_WriteRegAddr(nrf24l01p_reg_e reg, char* data)
{
	char tmp_data[6];
	tmp_data[0] = (0x20 | reg);

	for (uint8_t addr_cnt=0;addr_cnt<5;addr_cnt++)
	{
		tmp_data[addr_cnt+1] = data[addr_cnt];
	}
	(void) SPI_MasterTransmitMulti(6,&tmp_data[0]);
}

char NRF24L01P_ReadReg(nrf24l01p_reg_e reg, char data)
{
	char tmp_data[2];
	char *ret_data;
	tmp_data[0] = reg;
	tmp_data[1] = data;
	ret_data = SPI_MasterTransmitMulti(2,&tmp_data[0]);
	return ret_data[1];
}

void NRF24L01P_WritePayload(char * arg_pl)
{
	char tmp_data[NRF24L01P_PW+1];
	tmp_data[0] = 0b10100000;

	for (uint8_t pl_cnt=0;pl_cnt<NRF24L01P_PW;pl_cnt++)
	{
		tmp_data[pl_cnt+1] = arg_pl[pl_cnt];
//		USART_Transmit(arg_pl[pl_cnt]);
	}

	(void) SPI_MasterTransmitMulti(NRF24L01P_PW+1,&tmp_data[0]);
}

void NRF24L01P_FlushTxBuffer(void)
{
	(void) SPI_MasterTransmit(0xE1);
}

char* NRF24L01P_ReadPayload(void)
{
	char tmp_data[NRF24L01P_PW+1];
//	char *ret_data;
	tmp_data[0] = 0b01100001;

	for (uint8_t pl_cnt=0;pl_cnt<NRF24L01P_PW;pl_cnt++)
	{
		tmp_data[pl_cnt+1] = 0;
	}

	return SPI_MasterTransmitMulti(NRF24L01P_PW+1,&tmp_data[0]);
}

void NRF24L01P_Transmit(char * arg_pl)
{
	if (nrf24l01p_state == NRF_STATE_PENDING)
	{
		NRF24L01P_Proc();
	}

	nrf24l01p_state = NRF_STATE_PENDING;

	NRF24L01P_WritePayload(arg_pl);

	NRF24L01P_PORT_CE |= (1<<NRF24L01P_DD_CE);
	_delay_us(100);
	NRF24L01P_PORT_CE &= ~(1<<NRF24L01P_DD_CE);
}

char* NRF24L01P_getData(void)
{
	for (uint8_t i=0;i<NRF24L01P_PW;i++)
	{
		NRF24L01P_RX_BUF[i] = rx_buf_ptr[i+1];
	}
	nrf24l01p_state = NRF_STATE_IDLE;
	return &NRF24L01P_RX_BUF[0];
}

uint8_t NRF24L01P_getState(void)
{
	return nrf24l01p_state;
}

#ifdef NRF24L01P_INT
ISR(INT0_vect)
{
	NRF24L01P_Proc();
}
#endif

void NRF24L01P_ReSend_internal(void);
void NRF24L01P_ReSend_internal(void)
{
	nrf24l01p_state = NRF_STATE_PENDING;

	NRF24L01P_PORT_CE |= (1<<NRF24L01P_DD_CE);
	_delay_us(100);
	NRF24L01P_PORT_CE &= ~(1<<NRF24L01P_DD_CE);
	_delay_ms(1);
	NRF24L01P_Proc();
}

void NRF24L01P_Proc(void)
{
	char tmp_spi;

	if (nrf24l01p_state == NRF_STATE_PENDING)
	{
		tmp_spi = 0;
		while ( (tmp_spi & 0xF0) == 0x00 )
		{
			tmp_spi = NRF24L01P_ReadReg(STATUS,0x00);
			USART_Transmit(tmp_spi);
			_delay_ms(10);
		}

		switch (tmp_spi>>4)
		{
		case 0x01:
			nrf24l01p_state = NRF_STATE_FAILED;
			break;
		case 0x02:
			nrf24l01p_state = NRF_STATE_DONE;
			break;
		case 0x04:
			nrf24l01p_state = NRF_STATE_NEW_RX;
			rx_buf_ptr = NRF24L01P_ReadPayload();
			break;
		default:
			break;
		}
	}

	if (nrf24l01p_state == NRF_STATE_DONE)
	{
		tmp_spi = NRF24L01P_ReadReg(FIFO_STATUS,0x00);
//		USART_Transmit(tmp_spi);

		/* If TX FIFO is not empty - trigger another send command */
		if ( (tmp_spi & 0x10) == 0x00 )
		{
			NRF24L01P_ReSend_internal();
		}
		else
		{
			nrf24l01p_state = NRF_STATE_IDLE;
		}
	}

	NRF24L01P_WriteReg(STATUS,0x70);
	_delay_ms(2);
//	tmp_spi = NRF24L01P_ReadReg(STATUS,0x00);
//	USART_Transmit(tmp_spi);
//	tmp_spi = NRF24L01P_ReadReg(FIFO_STATUS,0x00);
//	USART_Transmit(tmp_spi);
}

