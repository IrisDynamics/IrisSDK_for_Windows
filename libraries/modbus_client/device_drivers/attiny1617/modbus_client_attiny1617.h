/**
 * @file modbus_client_attiny1617.h
 * @author Kali Erickson <kerickson@irisdynamics.com>, rebecca mcwilliam <rmcwilliam@irisdynamics.com>, sean jeffery <sjeffery@irisdynamics.com>
 * 
 * @brief  ATtiny1617 device driver for Modbus client serial communication
 * 
 * This class extends the virtual ModbusClient base class.
 * 
    
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

#ifndef MODBUS_CLIENT_ATTINY1617_H_
#define MODBUS_CLIENT_ATTINY1617_H_

#include "../../modbus_client.h"
#include "avr_usart.h"
#include "avr_timer.h"

/**
 * @class attiny1617_ModbusClient
 * @brief Extension of the ModbusClient virtual class that implements functions for the attiny1617's timers, and interrupts. UART channel specific functions
*/
class attiny1617_ModbusClient : public ModbusClient {

	uint32_t cycles_per_us;
	Usart& usart; // ATtiny1617 Usart driver object
	Timer& timer; // ATtiny1617 Timer driver object
	
    public:

	attiny1617_ModbusClient(
		int channel, uint32_t _cycles_per_us, Usart& _usart, Timer& _timer
	):
		ModbusClient(channel, _cycles_per_us),
		cycles_per_us(_cycles_per_us),
		usart(_usart),
		timer(_timer)
    {
        init(UART_BAUD_RATE);
    }

	int id() {
		return channel_number;
	}

/**
 * @brief Intializes and configures device peripherals.
 *
 * @param baud The UART baud rate with which to initialize communications 
*/
	void init(int baud) override {
		usart.init(baud);
		reset_state();
	}
	
	////////////////////////    isr's   ///////////////////////////////
/**
 * @brief RX/TX interrupt service routine.
 * Checks interrupt flags and calls appropriate ModbusClient function
*/
	void uart_isr() override {
		if (byte_ready_to_receive()) {
			receive();
		}
		else {
			send();
		}
	}

	/**
	* @brief return the number of system cycles
	*/
	uint32_t get_system_cycles() {
		return timer.micros() * cycles_per_us;
	}

    //////////// Virtual function implementations ////////////
    protected:
	/**
	 * @brief Enable the transmission of characters.
	*/
	void tx_enable() override {
		usart.enable_transmitter();
	}

	/**
	* @brief Disable the transmission of characters.
	*/
	void tx_disable() override {
		usart.disable_transmitter();
	}

	/**
	 * @brief Load the FIFO data register with the next byte
	 * @param byte		The byte to be transmitted.
	 */
	void send_byte(uint8_t data) override {
		usart.send_byte(data);
	}

	/**
	 * @brief Return the next byte received by the FIFO data register.
	 */
	uint8_t receive_byte() override {
		return usart.receive_byte();
	}

	/**
	* @brief There is a byte waiting to be received
	*/
	bool byte_ready_to_receive() override {
		return usart.is_byte_received();
	}

	/**
	 * @brief Adjust the baud rate
	 * @param baud_rate the new baud rate in bps
	*/
	void adjust_baud_rate(uint32_t baud_rate_bps) override {
		usart.set_baud(baud_rate_bps);
	}

};

extern attiny1617_ModbusClient modbus_client;

#endif
