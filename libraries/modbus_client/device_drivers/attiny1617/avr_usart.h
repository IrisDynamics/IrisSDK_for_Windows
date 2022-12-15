/*
* @file avr_usart.h
* @brief USART driver for the ATtiny1617. Sets up USART for use with an Iris Modbus Client.
* 
* To use with Modbus Client:
*     Construct a Usart object and pass into the attiny1617_ModbusClient constructor.
*
* Created: 2022-10-31 4:19:33 PM
*  Author: sjeffery
*/


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>

class Usart {

	USART_t * usart_base;	// Pointer to USART peripheral
	uint32_t clk_hz;		// USART peripheral clock speed
	PORT_t * tx_port;		// Port containing TXD pin
	uint8_t  tx_pin;		// TXD Pin number
	PORT_t * rx_port;		// Port containing RXD pin
	uint8_t  rx_pin;		// RXD Pin number

	void	enable_interrupt	(uint8_t mask)	{ usart_base->CTRLA |= mask;				}
	void	disable_interrupt	(uint8_t mask)	{ usart_base->CTRLA &= ~(mask);				}
	void	clear_interrupt		(uint8_t flag)	{ usart_base->STATUS |= flag;				}
		
public:
	
	/**
	 * @brief Usart constructor.
	 * @param _usart_base	Pointer to the USART peripheral
	 * @param _clk_hz		Frequency of the USART peripheral clock
	 * @param _tx_port		Pointer to the PORT peripheral containing the tx pin.
	 * @param _tx_pin		Pin number of the tx pin.
	 * @param _rx_port		Pointer to the PORT peripheral containing the rx pin.
	 * @param _rx_pin		Pin number of the rx pin.
	 */
	Usart(
		USART_t * _usart_base,
		uint32_t _clk_hz,
		PORT_t * _tx_port,
		uint8_t  _tx_pin,
		PORT_t * _rx_port,
		uint8_t  _rx_pin
	):
		usart_base(_usart_base),
		clk_hz(_clk_hz),
		tx_port(_tx_port),
		tx_pin(_tx_pin),
		rx_port(_rx_port),
		rx_pin(_rx_pin)
	{}
		
	uint8_t receive_byte		()				{ return usart_base->RXDATAL;				  }
	void	send_byte			(uint8_t byte)	{ usart_base->TXDATAL = byte;				  }
	void	enable_transmitter	()				{ usart_base->CTRLB |= USART_RXEN_bm;		  }
	void	enable_receiver		()				{ usart_base->CTRLB |= USART_TXEN_bm;		  } 
	void	disable_transmitter	()				{ usart_base->CTRLB &= ~(USART_RXEN_bm);	  }
	void	disable_receiver	()				{ usart_base->CTRLB &= ~(USART_TXEN_bm);	  }
	bool 	is_byte_received	()				{ return usart_base->STATUS & USART_RXCIF_bm; }
			
	void init(uint16_t baud) {	
		set_baud(baud);
		usart_base->CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_EVEN_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc; // Configure USART for asynchronous, even parity, 1 stop bit, and 8-bit data 
		tx_port->DIRSET |= tx_pin; // Configure USART TXD pin as output.
		rx_port->DIRCLR |= rx_pin; // Configure USART RXD pin as input.
		enable_interrupt(USART_RXCIE_bm | USART_TXCIE_bm); // Enable RXC, TXC interrupts.
		enable_receiver();
		enable_transmitter();
	}

	void set_baud(uint32_t baud) {
		usart_base->BAUD = (uint16_t)(64 * clk_hz / 16 / baud);
	}
	
	void send() {
		usart_base->CTRLA |= USART_DREIE_bm; // Enables DRE interrupt. 
	}
	
};


#endif /* USART_H_ */