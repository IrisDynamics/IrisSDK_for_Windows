/*!
 * @file IC4_k20.h
 * @author  Kyle Hagen <khagen@irisdynamics.com>
 * @version 2.2.0
 * created April 2021
 * @brief IrisControls4 device driver for the k20 linked against Arduino 1.8.13 and Teensyduio 1.53
 *
 * This object extends the IrisControls virtual base object and implements;
 * 1. USB messaging via the Teensy Serial object
 * 2. Int- and Float-to-string conversion via the Arduino String object
 * 3. System time using the Arduino micros() function
 *
 * This object does not add a serial parser, but an application may extend this object and override the parser to add custom serial parsing.
    
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
#ifndef _IC4_K20_H_
#define _IC4_K20_H_


#include "../../iriscontrols4.h"


class IC4_k20 : public IrisControls4 {
	
public:


	typedef enum {
		tx,
		rx
	} states;
	volatile states gui_frame_state = rx;

	IC4_k20() {
		set_full_duplex();
		Serial.println();
	}

	/**
	* @brief Writes bytes onto the hardware TX buffer from the software TX buffer
	* 
	* If there is an EOT enqueued in the software TX buffer the gui_frame_state is advanced to RX.
	* Should be called every GUI run loop while the gui_frame_state is TX. 
	*
	* Note: Using the write() method will group bytes together into USB packets so calling this method is not
	*		guaranteed to actually transmit immediately. There is a Serial.send_now() method that forces even
	*		a partially filled hardware tx buffer to send as soon as possible but using it here was causing 
	*		the Eagle to crash when the connection to the application was severed ungracefully
	*		e.g. Xing out of Iris Controls.
	*		Source: https://www.pjrc.com/teensy/td_serial.html
	*/
	void send() {
		while	(transmit_buffer.size())	Serial.write(transmit_buffer.popchar()); 
		
		if (eot_queued) {
			eot_queued = 0;
			gui_frame_state = rx;
		}
	}

	/**
	* @brief Transfers all data on the hardware RX buffer to the software RX buffer
	* Called by IrisControls4::check()
	*/
	void receive() {	
		while (Serial.available()) receive_char(Serial.read());
	}

	/**
	 * @brief return the system time in microseconds
	 */
	u64 system_time() {		
		return micros();
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
		return (String(d).c_str());
	}

	/**
	 * @brief Returns a string formatted int.
	 * @param[in] unsigned int d - The int to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print integers to the console in Iris Controls
	 */
	const char* val_to_str(unsigned int u) override {
		return (String(u).c_str());
	}

	/**
	 * @brief Returns a string formatted uint64_t.
	 * @param[in] u64 d - The uint64_t to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print integers to the console in Iris Controls
	 */
	const char* val_to_str(u64 u) override {
		long unsigned int lu = (long unsigned int)u;
		return (String(lu).c_str());
	}

	/**
	 * @brief Returns a string formatted float.
	 * @param[in] float f - The float to print.
	 * @param[out] const char * - The formatted input value.
	 * Used to print floats to the console in Iris Controls
	 */
	const char* val_to_str(float f) override {
		return (String(f).c_str());
	}

	int random_number() {
		return 0;
	}

	/**
	* @fn int parse_device_driver(char* cmd)
	* @brief Attempts to parse commands that were not successfully parsed by higher level parsers.
	* @param[in] char* cmd - The command to be parsed
	* @param[out] int - 1 if parsed successfully, 0 otherwise
	*/
	int parse_device_driver(char* cmd) {
		
		std::string command_list = "\rK20: \r\r system_time\r";

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


#endif
