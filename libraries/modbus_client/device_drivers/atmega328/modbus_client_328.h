/**
 * @file modbus_client_k20.h
 * @author Kali Erickson <kerickson@irisdynamics.com>, rebecca mcwilliam <rmcwilliam@irisdynamics.com>
 * 
 * @brief  Eagle (k20) device driver for Modbus client serial communication
 * 
 * This class extends the virtual ModbusClient base class, and is used in the implementation of the Actuator object
 * 
 * @version 2.2.0
    
    @copyright Copyright 2022 Iris Dynamics Ltd 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    For questions or feedback on this file, please email <support@irisdynamics.com>. 
 */

#ifndef MODBUS_CLIENT_328_H_
#define MODBUS_CLIENT_328_H_

#include "../../modbus_client.h"

#define CYCLES_PER_MICRO F_CPU / 1000000L

#define UART_READY    (1<<RXCIE0) | (1<<RXEN0)			//!<Enable RX interrupts, ready to receive incomming messages
#define UART_SENDING  UART_READY | (1<<UDRIE0) | (1<<TXEN0)		//!< Enable tx interrupts, start sending outgoing messages
/**
 * @class atmega328_ModbusClient
 * @brief Extension of the ModbusClient virtual class that implements functions for the Seagull atmega328's timers, and interrupts. UART channel specific functions
*/
class atmega328_ModbusClient : public ModbusClient {

    public:

    //constructor
		atmega328_ModbusClient(int channel, uint32_t cycles_per_second):
		ModbusClient(channel, cycles_per_second)
    {
        init(UART_BAUD_RATE);
    }

	int id() {
		return channel_number;
	}

	/**
 * @brief Intializes and configure the device timers and interrupts
 *
 * @param baud The UART baud rate with which to initialize communications 
*/
	void init(int baud) override {
		cli();
		UCSR0A = (1 << U2X0);      //!<double speed asynch
		UCSR0B = UART_READY;//UART_SENDING;
		DDRD &= ~0x02;  // disable pin
		UCSR0C = (1 << UCSZ01) + (1 << UCSZ00) + (1 << UPM01);  //!< asynchronous, even parity, 1 stop bit, 8 bit data
		

		//baud 19200
		//UBRR0H = (uint8_t)(129>>8);
		//UBRR0L = (uint8_t)129;  //!< 19200 Baud
		adjust_baud_rate(19200);
		sei();
		//SIM_SCGC4 |= SIM_SCGC4_UARTX;   //enable/start system clock

		////port mux
		//*PORTZ_PCRX = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3);    //pullup resistor enabled, passive filter enabled - in alt3 port mux config PORTD_PCR2 (PTD2) is UART2_RX
		//*PORTZ_PCRY = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);                 //High drive strength enabled, Slow slew rate enabled, in alt3 port mux config PORTD_PCR3 (PTD3)  is UART2_TX   

		////set to 9-bit with even parity- to make odd, add "| UART_C1_PT" - to make no parity, set equal to 0
		//UARTX->C1 = UART_C1_M | UART_C1_PE;

		////disable FIFO / set transmit and receive FIFO/Buffer depth to 1 dataword
		//UARTX->PFIFO = 0;

		////set initial state (status reg) - enables receiver, receiver interrupts, and transmitter (NOT transmitter interrupts)
		//UARTX->C2 = C2_ENABLE;
		//UARTX->C3 = C3_EI_ENABLE; //enables receiver overrun, noise, and framing error interrupts (if parity is added, PEIE will also need to be enabled)

		////interupt priority and enable for UART channel 2 status and error and PIT channel 2
		//NVIC_SET_PRIORITY(IRQ_UARTX_STATUS, IRQ_PRIORITY);
		//NVIC_ENABLE_IRQ(IRQ_UARTX_STATUS);
		//NVIC_CLEAR_PENDING(IRQ_UARTX_STATUS);
		
		//adjust_baud_rate(baud);
		reset_state();
	}
	////////////////////////    isr's   ///////////////////////////////
/**
 * @brief RX/TX interrupt service routine.
 * Checks interrupt flags and calls appropriate ModbusClient function
*/
	void uart_isr() {
		////	RX register FULL flag (RX interrupts always enabled)
		if (byte_ready_to_receive()) {
			receive();
		}
		else {
			send();
		}
		
		//}
		////if tx data register empty(TDRE) interrupt enabled and TDRE flag set, OR transmission complete(TC) interrupt enabled and TC flag is set
		//else if (((UARTX->C2 & UART_C2_TIE) && (UARTX->S1 & UART_S1_TDRE))){// || ((UARTX->C2 & UART_C2_TCIE) && (UARTX->S1 & UART_S1_TC))) {
		//	send();
		//}
	}


	/**
	* @brief return the number of system cycles, micros accurate to 4 microseconds
	*/
	uint32_t get_system_cycles() {
		//return micros() * clockCyclesPerMicrosecond();	//todo remove double multiplication
		return micros() * clockCyclesPerMicrosecond();
	}

    //////////// Virtual function implementations ////////////
   protected:
	/**
	 * @brief Enable the transmission of characters by enabling TIE. This in turn enables TDRE and TCIE interrupts
	*/
	void tx_enable() override {
	//	UARTX->C2 = C2_TX_ENABLE;    //enable transmission interrupts
		UCSR0B = UART_SENDING;
	}

	/**
	* @brief Disable the transmission of characters by disabling TIE. This in turn disables TDRE and TCIE interrupts
	*/
	void tx_disable() override {
		//UARTX->C2 = C2_TX_DISABLE;    //disable transmission interrupts
		UCSR0B = UART_READY;
		DDRD &= ~0x02;
	}

	/**
	 * @brief Load the FIFO data register with the next byte
	 * @param byte		The byte to be transmitted.
	 */
	void send_byte(uint8_t data) override {
		UDR0 = data;
	}

	/**
	 * @brief Return the next byte received by the FIFO data register.
	 */
	uint8_t receive_byte() override {
		digitalWrite(LED_BUILTIN, HIGH);
		return UDR0;
	}

	/**
	* @brief There is a byte waiting to be received
	*/
	bool byte_ready_to_receive() override{
		//return; //true;//UART0->S1 & UART_S1_RDRF;
		return false;
	}

	/**
	 * @brief Adjust the baud rate
	 * @param baud_rate the new baud rate in bps
	*/
	void adjust_baud_rate(uint32_t baud_rate_bps) override {
		uint32_t divisor = (F_CPU / (8 * baud_rate_bps)) - 1;
		//switch (channel_number) {
		//case 0: divisor = BAUD2DIV(baud_rate_bps);
		//	break;
		//case 1:	divisor = BAUD2DIV2(baud_rate_bps);
		//	break;
		//case 2: divisor = BAUD2DIV3(baud_rate_bps);
		//	break;
		//default:
//			divisor = BAUD2DIV(baud_rate_bps);
////		}
		UBRR0 = divisor;
	}



};

extern atmega328_ModbusClient modbus_client;

#endif
