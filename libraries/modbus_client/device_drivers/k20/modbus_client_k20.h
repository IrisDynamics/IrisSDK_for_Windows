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

#ifndef MODBUS_CLIENT_K20_H_
#define MODBUS_CLIENT_K20_H_

#include "../../modbus_client.h"
#include "modbus_k20_config.h"

#define CYCLES_PER_MICRO F_CPU / 1000000L
/**
 * @class k20_ModbusClient
 * @brief Extension of the ModbusClient virtual class that implements functions for the Eagle K20's timers, and interrupts. UART channel specific functions
*/
class k20_ModbusClient : public ModbusClient {

    KINETISK_UART_t* UARTX;             //!< pointer to a struct representing the section of memory related to the UART control and status registers, as defined in kinetis.h
    uint8_t IRQ_UARTX_STATUS;
    uint8_t IRQ_UARTX_ERROR;
    volatile uint32_t* PORTZ_PCRX;
    volatile uint32_t* PORTZ_PCRY;
    uint32_t SIM_SCGC4_UARTX;

    volatile uint32_t *PIT_LDVALX;
    volatile uint32_t *PIT_TCTRLX;
    volatile uint32_t *PIT_TFLGX;


    public:

    //constructor
    k20_ModbusClient(int channel, uint32_t cycles_per_second):
		ModbusClient(channel, cycles_per_second)
    {
        switch(channel){
            case 0:
                UARTX = &KINETISK_UART0;
                IRQ_UARTX_STATUS = IRQ_UART0_STATUS;
                IRQ_UARTX_ERROR = IRQ_UART0_ERROR;
                PORTZ_PCRX = &PORTB_PCR16;
                PORTZ_PCRY = &PORTB_PCR17;
                SIM_SCGC4_UARTX = SIM_SCGC4_UART0;
                break;

            case 1:
                UARTX = &KINETISK_UART1;
                IRQ_UARTX_STATUS = IRQ_UART1_STATUS;
                IRQ_UARTX_ERROR = IRQ_UART1_ERROR;
                PORTZ_PCRX = &PORTC_PCR3;
                PORTZ_PCRY = &PORTC_PCR4;
                SIM_SCGC4_UARTX = SIM_SCGC4_UART1;
                break;

            case 2:
                UARTX = &KINETISK_UART2;
                IRQ_UARTX_STATUS = IRQ_UART2_STATUS;
                IRQ_UARTX_ERROR = IRQ_UART2_ERROR;
                PORTZ_PCRX = &PORTD_PCR2;
                PORTZ_PCRY = &PORTD_PCR3;
                SIM_SCGC4_UARTX = SIM_SCGC4_UART2;
                break;
        }

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
		
		SIM_SCGC4 |= SIM_SCGC4_UARTX;   //enable/start system clock

		//port mux
		*PORTZ_PCRX = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3);    //pullup resistor enabled, passive filter enabled - in alt3 port mux config PORTD_PCR2 (PTD2) is UART2_RX
		*PORTZ_PCRY = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);                 //High drive strength enabled, Slow slew rate enabled, in alt3 port mux config PORTD_PCR3 (PTD3)  is UART2_TX   

		//set to 9-bit with even parity- to make odd, add "| UART_C1_PT" - to make no parity, set equal to 0
		UARTX->C1 = UART_C1_M | UART_C1_PE;

		//disable FIFO / set transmit and receive FIFO/Buffer depth to 1 dataword
		UARTX->PFIFO = 0;

		//set initial state (status reg) - enables receiver, receiver interrupts, and transmitter (NOT transmitter interrupts)
		UARTX->C2 = C2_ENABLE;
		UARTX->C3 = C3_EI_ENABLE; //enables receiver overrun, noise, and framing error interrupts (if parity is added, PEIE will also need to be enabled)

		//interupt priority and enable for UART channel 2 status and error and PIT channel 2
		NVIC_SET_PRIORITY(IRQ_UARTX_STATUS, IRQ_PRIORITY);
		NVIC_ENABLE_IRQ(IRQ_UARTX_STATUS);
		NVIC_CLEAR_PENDING(IRQ_UARTX_STATUS);

		adjust_baud_rate(baud);
		reset_state();
	}
	////////////////////////    isr's   ///////////////////////////////
/**
 * @brief RX/TX interrupt service routine.
 * Checks interrupt flags and calls appropriate ModbusClient function
*/
	void uart_isr() {
		//	RX register FULL flag (RX interrupts always enabled)
		if (byte_ready_to_receive()) {
			receive();
		}
		//if tx data register empty(TDRE) interrupt enabled and TDRE flag set, OR transmission complete(TC) interrupt enabled and TC flag is set
		else if (((UARTX->C2 & UART_C2_TIE) && (UARTX->S1 & UART_S1_TDRE))){// || ((UARTX->C2 & UART_C2_TCIE) && (UARTX->S1 & UART_S1_TC))) {
			send();
		}
	}


	/**
	* @brief return the number of system cycles, micros accurate to 4 microseconds
	*/
	uint32_t get_system_cycles() {
		return micros() * clockCyclesPerMicrosecond();	//todo remove double multiplication
	}

    //////////// Virtual function implementations ////////////
   protected:
	/**
	 * @brief Enable the transmission of characters by enabling TIE. This in turn enables TDRE and TCIE interrupts
	*/
	void tx_enable() override {
		UARTX->C2 = C2_TX_ENABLE;    //enable transmission interrupts
	}

	/**
	* @brief Disable the transmission of characters by disabling TIE. This in turn disables TDRE and TCIE interrupts
	*/
	void tx_disable() override {
		UARTX->C2 = C2_TX_DISABLE;    //disable transmission interrupts
	}

	/**
	 * @brief Load the FIFO data register with the next byte
	 * @param byte		The byte to be transmitted.
	 */
	void send_byte(uint8_t data) override {
		UARTX->D = data;
	}

	/**
	 * @brief Return the next byte received by the FIFO data register.
	 */
	uint8_t receive_byte() override {
		return UARTX->D;
	}

	/**
	* @brief There is a byte waiting to be received
	*/
	bool byte_ready_to_receive() override{
		return UARTX->S1 & UART_S1_RDRF;
	}

	/**
	 * @brief Adjust the baud rate
	 * @param baud_rate the new baud rate in bps
	*/
	void adjust_baud_rate(uint32_t baud_rate_bps) override {
		uint32_t divisor;
		switch (channel_number) {
		case 0: divisor = BAUD2DIV(baud_rate_bps);
			break;
		case 1:	divisor = BAUD2DIV2(baud_rate_bps);
			break;
		case 2: divisor = BAUD2DIV3(baud_rate_bps);
			break;
		default:divisor = BAUD2DIV(baud_rate_bps);
		}
		UARTX->BDH = (divisor >> 13) & 0x1F;
		UARTX->BDL = (divisor >> 5) & 0xFF;
		UARTX->C4 = divisor & 0x1F;
	}

};

extern k20_ModbusClient modbus_client;

#endif
