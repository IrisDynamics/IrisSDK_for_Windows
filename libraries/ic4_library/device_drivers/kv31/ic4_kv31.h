/**
 * ic4_kv31.h
 *
 * Created on: Apr 15, 2021
 * @author Kyle Hagen <khagen@irisdynamics.com>
 * @version 2.2.0
    
    Copyright 2022 Iris Dynamics Ltd 
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

#ifndef IC4_KV31_H_
#define IC4_KV31_H_

#include "fsl_uart.h"
#include "fsl_gpio.h"
#include "../../config.h"
#include "../../iriscontrols4.h"
#include "common.h"
#include <string>

/**
 * @brief Manages the UART communications used for an IC4 connection
 *
 * The IC4_kv31 object contains a state machine with 3 distinct states:
 *
 * listening - The device is not connected to IrisControls, but serial commands are being parsed. 						UART status: RX enabled
 * tx		 - The device is currently transmitting data out of the UART. May or may not be connected to IrisControls.  UART status: TX enabled
 * rx		 - The device is connected to IrisControls and is waiting for a response.									UART status: RX enabled
 */
class IC4_kv31 : public IrisControls4 {

protected:

	UART_Type * uart_ptr;

	volatile bool frame_finished = 0; // for indicating that a half duplex frame has finished building, checked in ISR

public:

	typedef enum {
		rx			= 0,
		tx 			= 1,
	} states;

	u32 up_count = 0;
	u32 down_count = 0;
	volatile states state = rx;

	IC4_kv31 (
		UART_Type * _uart_ptr
	) :
		uart_ptr(_uart_ptr)
	{}

	virtual ~IC4_kv31() {};

	void setup() {
		UART_SetBaudRate(uart_ptr,IC4_BAUDRATE,CLOCK_GetCoreSysClkFreq());
//		uart_config_t uartConfig;
//		uartConfig.baudRate_Bps = 115200U;
//		uartConfig.parityMode = kUART_ParityDisabled;
//		uartConfig.txFifoWatermark = 0;
//		uartConfig.rxFifoWatermark = 7;
//		UART_Init(uart_ptr, &uartConfig, 120000000U);
	}

	/**
	 * @brief Enables UART receiving and disables UART transmission.
	 */
	void enable_rx () {
		state = rx;
		GPIO_PinWrite(GPIOB, 18, 0);
		UART_DisableInterrupts(uart_ptr, kUART_TransmissionCompleteInterruptEnable | kUART_TxDataRegEmptyInterruptEnable);
		UART_EnableRx(uart_ptr, true);
		UART_EnableTx(uart_ptr, false);
		UART_EnableInterrupts(uart_ptr, kUART_RxDataRegFullInterruptEnable | kUART_IdleLineInterruptEnable);
	}

	/**
	 * @brief Enables UART transmission and disables UART receiving.
	 */
	void enable_tx () {
		state = tx;
		GPIO_PinWrite(GPIOB, 18, 1);
		UART_DisableInterrupts(uart_ptr, kUART_RxDataRegFullInterruptEnable | kUART_IdleLineInterruptEnable);
		UART_EnableTx(uart_ptr, true);
		UART_EnableRx(uart_ptr, false);
		UART_EnableInterrupts(uart_ptr, kUART_TxDataRegEmptyInterruptEnable);
	}

	/* For exposing private function to C ISR */
	u32 get_tx_buffer_size() {
		u32 size = transmit_buffer.size();
		return size;
	}

	/* For exposing private function to C ISR */
	char popchar() {
		char c = transmit_buffer.popchar();
		return c;
	}

	void send() override {
		enable_tx();
	}

	void handle_eot() override {
		send();
	}

	/**
	 * Receive is intentionally  empty for the kv31 platform to avoid polling the receiver in check().
	 * Receiving is handled entirely in interrupts on the kv31.
	 */
	void receive() override {}

	/**
	 * @brief return the system time in microseconds
	 */
	u64 system_time() override {
		return (u64)micros();
	}

	/**
	 * @brief Returns a string formatted int.
	 * @param[in] int d - The int to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print integers to the console in Iris Controls.
	 */
	const char* val_to_str(int d) override {
		std::string str_obj(std::to_string(d));
		return str_obj.c_str();
	}
	/**
	 * @brief Returns a string formatted int.
	 * @param[in] unsigned int d - The int to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print integers to the console in Iris Controls.
	 */
	const char* val_to_str(unsigned int u) override {
		std::string str_obj(std::to_string(u));
		return str_obj.c_str();
	}

	/**
	 * @brief Returns a string formatted int.
	 * @param[in] u64 d - The uint64_t to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print integers to the console in Iris Controls.
	 */
	const char* val_to_str(u64 u) override {
		std::string str_obj(std::to_string(u));
		return str_obj.c_str();
	}

	/**
	 * @brief Returns a string formatted float.
	 * @param[in] float f - The float to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print floats to the console in Iris Controls
	 */
	const char* val_to_str(float f) override {
		std::string str_obj(std::to_string(f));
		return str_obj.c_str();
	}

	/**
	 * @brief Returns 0.0 because the kv31 cannot use sscanf to parse doubles.
	 * @return double 0.0.
	 * @note
	 * The IC4 library uses sscanf in its console command argument parsing functions. The KV31 platform does not contain
	 * a complete standard library and using sscanf with floating point arguments produces undefined behaviour affecting
	 * connection stability with IC4. This dummy implementation was added to provide user feedback.
	 */
	double parse_double() override {
		PRINTL("Error: Cannot parse doubles on the KV31 platform. Please use integers and divide as appropriate.");
		return 0.0;
	}

	virtual int random_number() {
		return 0;
	}

	void rdrf_isr() {
		switch(state) {
			case rx:
				while(uart_ptr->RCFIFO > 0) {
					char c = UART_ReadByte(uart_ptr);
					receive_char(c);
					down_count++;
				}
				break;

			case tx:
				orca_mem.write(TX_RDRF_ERROR, orca_mem.read(TX_RDRF_ERROR) + 1);
				break;
		}
	}

	void tdre_isr() {
		switch(state) {
			case tx: {
				u16 tx_space = 8 - uart_ptr->TCFIFO;	// calculate how much room is left in the tx fifo
				// write software buffer to tx fifo
				for (u16 i = 0; i < tx_space; i++) {
					if(get_tx_buffer_size()) {				// if there is data in the tx buffer, send it
						UART_WriteByte(uart_ptr, popchar());
						up_count++;
					}
					else if(frame_finished || is_disconnected()) { // If there is no data in the tx buffer and the GUI frame is finished building or the device is disconnected, enable TC and disable TDRE
						UART_DisableInterrupts(uart_ptr, kUART_TxDataRegEmptyInterruptEnable);
						UART_EnableInterrupts(uart_ptr, kUART_TransmissionCompleteInterruptEnable);
						break;
					}
					else {
						UART_DisableInterrupts(uart_ptr, kUART_TxDataRegEmptyInterruptEnable); // if there is no data in the buffers but the frame is not finished and the device is still connected, just disable TDRE
					}
				}
				break;
			}

			case rx:
				enable_rx();
				orca_mem.write(RX_TDRE_ERROR, orca_mem.read(RX_TDRE_ERROR) + 1);
				break;

		}
	}

	void tc_isr() {
		switch(state) {
			case tx:
				frame_finished = 0;	// reset frame finished flag
				enable_rx();	// enable receiver after transitioning out of tx mode
				break;

			case rx:
				orca_mem.write(RX_TC_ERROR, orca_mem.read(RX_TC_ERROR) + 1);
				break;
		}
	}

	/**
	* @fn int parse_device_driver(char* cmd)
	* @brief Attempts to parse commands that were not successfully parsed by higher level parsers.
	* @param[in] char* cmd - The command to be parsed
	* @param[out] int - 1 if parsed successfully, 0 otherwise
	*/
	int parse_device_driver(char* cmd) {


		std::string command_list = "\rKV31:\r";

		START_PARSING

		COMMAND_IS "system_time" THEN_DO

			print_l("System Time: ");
			print_d(system_time());
			print_l(" us\r");

		COMMAND_IS "help" THEN_DO

			print_l(command_list.c_str());
			return 1;

		FINISH_PARSING
	}

};





#endif /* IC4_KV31_H_ */
