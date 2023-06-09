/**
 * @file IC4_Zynq7000.h
 * @author  Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 *	This class extends the IrisControls4 class and implements Zynq 7000 specific functions
    
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

#pragma once
#include "../../../ic4_library/iriscontrols4.h"
#include "xuartps_hw.h"
#include "xuartps.h"
#include "../../../interrupts.h"
#include "../../../config.h"

#include "xuartns550_l.h"        // uart hardware definitions

class IC4_Zynq7000 : public IrisControls4
{

	UINTPTR base_address;
	const int interrupt_id;

public:

	void setup() {

		set_device_id(DEVICE_ID);
		//set_full_duplex();
		set_half_duplex();

		XUartNs550_SetBaud(base_address, XPAR_PL_UART3_CLOCK_FREQ_HZ, IRISCONTROLS_UART_BAUDRATE);
		/////////////////////////////////////////////////////// Set up UART Format
		// http://www.ti.com/lit/ds/symlink/pc16550d.pdf page 20
		// 8 data bits
		// 1 stop
		// no parity
		XUartNs550_SetLineControlReg (base_address, XUN_LCR_8_DATA_BITS );
		//////////////////////////////////////////////////////////////////////////////


    	/////////////////////////////////////////////////////// Set up UART interrupt
    	u32 current_ier = Xil_In32(base_address + XUN_IER_OFFSET);
    	Xil_Out32(base_address + XUN_IER_OFFSET, current_ier | XUN_IER_RX_DATA);
    	InterruptSystem2::enable( interrupt_id  );
//
//    	// Enable FIFO, reset FIFOs
    	Xil_Out32 (base_address + XUN_FCR_OFFSET, XUN_FIFO_ENABLE);

    	// fifo 14
    	Xil_Out32 (base_address + XUN_FCR_OFFSET, XUN_FIFO_RX_TRIG_LSB | XUN_FIFO_RX_TRIG_MSB | XUN_FIFO_TX_RESET | XUN_FIFO_RX_RESET | XUN_FIFO_ENABLE);

    	// fifo 8
//    	Xil_Out32 (base_address + XUN_FCR_OFFSET, XUN_FIFO_RX_TRIG_MSB | XUN_FIFO_TX_RESET | XUN_FIFO_RX_RESET | XUN_FIFO_ENABLE);

    	// fifo 4
//    	Xil_Out32 (base_address + XUN_FCR_OFFSET, XUN_FIFO_RX_TRIG_LSB | XUN_FIFO_TX_RESET | XUN_FIFO_RX_RESET | XUN_FIFO_ENABLE);

	}


	typedef enum {
		tx,
		rx
	} states;
	volatile states gui_frame_state = rx;

	IC4_Zynq7000(UINTPTR _base_address, int _interrupt_id) :
		base_address(_base_address),
		interrupt_id(_interrupt_id)
	{}


	void uart_isr() {


		volatile u32 isr_status = Xil_In32(base_address + XUN_IIR_OFFSET) & XUN_INT_ID_MASK;

		// https://www.xilinx.com/support/documentation/ip_documentation/axi_uart16550/v2_0/pg143-axi-uart16550.pdf
		// table 2-8
		#define MODEM_STATUS			0b0000	// reset by reading modem status register
		#define TX_HOLDING_REG_EMPTY	0b0010	// reset by reading IIR register (if source of interrupt), or writing into transmitter holding register
		#define CHARACTER_TIMEOUT		0b1100	// reset by reading receiver
		#define RX_DATA_AVAILABLE		0b0100	// reset by reading data from FIFO
		#define RX_LINE_STATUS			0b0110	// reset by reading LSR

		switch ( isr_status ) {

			case CHARACTER_TIMEOUT:
			case RX_DATA_AVAILABLE:

				while (Xil_In32(base_address + XUN_LSR_OFFSET) & XUN_LSR_DATA_READY) {
					int c = Xil_In32(base_address + XUN_RBR_OFFSET);
//					int c = XUartNs550_RecvByte(base_address);
					receive_char(c);
				}

				break;

			case TX_HOLDING_REG_EMPTY:

				// Send bytes until the fifo is full or the buffer is empty
				for(int i=0; i < XUN_FIFO_SIZE && transmit_buffer.size() > 0; i++){
					char char_to_send = transmit_buffer.popchar();
					XUartNs550_WriteReg(base_address, XUN_THR_OFFSET, char_to_send);
				}

				// If the buffer is empty, disable the ISR
				if ( transmit_buffer.size() == 0 ) {
					u32 current_ier = Xil_In32(base_address + XUN_IER_OFFSET);
					Xil_Out32(base_address + XUN_IER_OFFSET, current_ier & ~XUN_IER_TX_EMPTY);

					if (eot_queued ) {
						eot_queued = 0;
						gui_frame_state = rx;
					}
				}
				break;

			case RX_LINE_STATUS:
			case MODEM_STATUS:
			default:
				break;
		}

	}

	/**
	* @brief Writes bytes onto the hardware TX buffer from the software TX buffer
	*
	* If there is an EOT enqueued in the software TX buffer the gui_frame_state is advanced to RX.
	* Should be called every GUI run loop while the gui_frame_state is TX.
	*/
	void send(){		

		// start er up
    	u32 current_ier = Xil_In32(base_address + XUN_IER_OFFSET);
		Xil_Out32(base_address + XUN_IER_OFFSET, current_ier | XUN_IER_TX_EMPTY);



//		while (transmit_buffer.size() > 0) {
//			if (!XUartNs550_IsTransmitEmpty(base_address))
//				return; //this ensures that if an EOT is queued but isn't actually sent that the state machine won't advance.
//
//			char char_to_send = transmit_buffer.popchar();
//
//			XUartNs550_SendByte(base_address, char_to_send);
//		}
//
//		if (eot_queued) {
//			eot_queued = 0;
//			gui_frame_state = rx;
//		}

	}

	/**
	* @brief moves data from hardware to receive buffer
	*/
	void receive() {


//		while (XUartNs550_IsReceiveData(base_address)) {
//			int c = XUartNs550_RecvByte(base_address);
//			receive_char(c);
//		}

	}


	/**
	 * @brief returns the device time in microseconds
	 */
	u64 system_time(){
		u64 now;
		XTime_GetTime(&now);
		return now * MICROSECONDS_PER_COUNT;
//		return now * MILLISECONDS_PER_COUNT;
	}

	/**
	* @brief Method called when an EOT is successfully parsed
	* The EOT marks the end transmission by the application to the device.
	* Sets the gui_frame_state to tx.
	*/
	void handle_eot() {
		gui_frame_state = tx;
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
	 * Used to print integers to the console in Iris Controls
	 */
	const char* val_to_str(unsigned int d) override {
		std::string str_obj(std::to_string(d));
		return str_obj.c_str();
	}

	/**
	 * @brief Returns a string formatted uint64_t.
	 * @param[in] u64 d - The uint64_t to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print integers to the console in Iris Controls
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
	* @fn int parse_device_driver(char* cmd)
	* @brief Attempts to parse commands that were not successfully parsed by higher level parsers.
	* @param[in] char* cmd - The command to be parsed
	* @param[out] int - 1 if parsed successfully, 0 otherwise
	*/
	int parse_device_driver(char* cmd) {

		std::string command_list = "\rZynq 7000: \r\r system_time\r";

		START_PARSING

		COMMAND_IS "system_time" THEN_DO

			print_l("System Time: ");
			print_d(system_time());
			print_l(" us\r");

		COMMAND_IS "help" THEN_DO

			print_l(command_list.c_str());			

		FINISH_PARSING
	}

};
