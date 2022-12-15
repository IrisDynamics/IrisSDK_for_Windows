/**
 * @file modbus_client_application.h
 * @author Kali Erickson <kerickson@irisdynamics.com>
 * 
 * @brief  Base implementation of a Modbus client application layer
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

#ifndef MODBUS_CLIENT_APPLICATION_H_
#define MODBUS_CLIENT_APPLICATION_H_

#include <stdint.h>
#include "function_code_parameters.h"
#include "modbus_client.h"

/**
 * @class ModbusClientApplication
 * @brief Base implementation of a Modbus client application layer.
 *
 * To add a function code in a derived class:
 *
 * 1. Add the function code's name and code number (in hex) to a public function code enum in the derived class.
 *
 * 2. Add the expected length of the function code's response to the function_code_parameters.h file if it is known. 
 *    If the length is variable, don't define it. Instead, calculate it in the implementation of the get_app_reception_length function.
 *     
 * 	  Other useful definitions pertaining to the function code may be added here as well, such as valid data ranges.
 *
 * 3. Add a case returning the expected length to the implementation of the get_app_response_length function.
 *    If the length is unknown until response reception, return -1.
 *
 * 4. Add a private function ( "function_code_name_fn()" ) to the derived class that loads transmission with the properly formatted message and then calls send_transaction.
 *    Return a 1 if the function is successful, return a 0 if an exception occurs.
 */
class ModbusClientApplication {

	/**
	 * @brief Enum of all supported diagnostic sub-function codes.
	 */
	enum sub_function_codes_e {
		return_query_data = 00,
		restart_communications = 01,
		return_diagnostic_register = 02,
		force_listen_only_mode = 04,
		clear_counters_and_diagnostic_register = 10,
		return_bus_message_count = 11,
		return_bus_communication_error_count = 12,
		return_bus_exception_error_count = 13,
		return_server_message_count = 14,
		return_server_no_response_count = 15,
		return_server_NAK_count = 16,
		return_server_busy_count = 17,
		return_bus_character_overrun_count = 18,
		clear_overrun_counter_and_flag = 20
	};

protected:

    Transaction my_temp_transaction;
	ModbusClient& UART;

public: 
	ModbusClientApplication(ModbusClient& _UART) :
		UART(_UART)
	{}
	/**
	 * @brief Enum of all supported function codes.
	 */
	enum function_codes_e {
		read_coils = 0x01,
		read_discrete_inputs = 0x02,
		read_holding_registers = 0x03,
		read_input_registers = 0x04,
		write_single_coil = 0x05,
		write_single_register = 0x06,
		read_exception_status = 0x07,
		diagnostics = 0x08,
		get_comm_event_counter = 0x0B,
		get_comm_event_log = 0x0C,
		write_multiple_coils = 0x0F,
		write_multiple_registers = 0x10,
		report_server_id = 0x11,
		mask_write_register = 0x16,
		read_write_multiple_registers = 0x17
	};


	/**
	 * @brief Format a read_coils request, function code 01, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param starting_address The address of the first coil to start reading from
     * @param num_coils The quantity of coil values to read
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
	int read_coils_fn(uint8_t device_address, uint16_t starting_address, uint16_t num_coils) {
		//exception checking
		if(num_coils < 1 || num_coils > MAX_NUM_READ_COILS) return 0;

		int ret_size;
		if(num_coils % 8 == 0){
			ret_size = 5 + num_coils / 8;
		} else {
			ret_size = 6 + num_coils / 8;
		}

		uint8_t data_bytes[4] = {uint8_t(starting_address >> 8), uint8_t(starting_address), uint8_t(num_coils >> 8), uint8_t(num_coils)};
        my_temp_transaction.load_transmission_data(
        		device_address, read_coils, data_bytes, 4,
				ret_size);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

	/**
	 * @brief Format a read_discrete_inputs request, function code 02, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param starting_address The address of the first discrete input to start reading from
     * @param num_inputs The quanity of discrete input values to read
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
	int read_discrete_inputs_fn(uint8_t device_address, uint16_t starting_address, uint16_t num_inputs) {
		//exception checking
		if(num_inputs < 1 || num_inputs > MAX_NUM_DISCRETE_INPUTS) return 0;

		int ret_size;
		if(num_inputs % 8 == 0){
			ret_size = 5 + num_inputs / 8;
		} else {
			ret_size = 6 + num_inputs / 8;
		}
		uint8_t data_bytes[4] = {uint8_t(starting_address >> 8), uint8_t(starting_address), uint8_t(num_inputs >> 8), uint8_t(num_inputs)};
        my_temp_transaction.load_transmission_data(
        		device_address, read_discrete_inputs, data_bytes, 4,
				ret_size);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

	/**
	 * @brief Format a read_holding_registers request, function code 03, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param starting_address The address of the first holding register to start reading from
     * @param num_registers The quanity of holding registers to read
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
	int read_holding_registers_fn(uint8_t device_address, uint16_t starting_address, uint16_t num_registers) {
		//parameter checking
		if(num_registers < 1 || num_registers > MAX_NUM_READ_REG) return 0;

		uint8_t data_bytes[4] = {uint8_t(starting_address >> 8), uint8_t(starting_address), uint8_t(num_registers >> 8), uint8_t(num_registers)};
        my_temp_transaction.load_transmission_data(
        		device_address, read_holding_registers, data_bytes, 4,
				5 + (num_registers*2));
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

	/**
	 * @brief Format a read_input_registers request, function code 04, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param starting_address The address of the first input register to start reading from
     * @param num_registers The quantity of input registers to read
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfully, 0 if an exception occurs
	 */
	int read_input_registers_fn(uint8_t device_address, uint16_t starting_address, uint16_t num_registers) {
		//parameter checking
		if(num_registers < 1 || num_registers > MAX_NUM_READ_REG) return 0;

		uint8_t data_bytes[4] = {uint8_t(starting_address >> 8), uint8_t(starting_address), uint8_t(num_registers >> 8), uint8_t(num_registers)};
        my_temp_transaction.load_transmission_data(
        		device_address, read_input_registers, data_bytes, 4,
				5 + (num_registers*2));
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

	/**
	 * @brief Format a write_single_coil request, function code 05, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param address The address of the coil to be written to
     * @param data The discrete value to be written to the coil
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
	int write_single_coil_fn(uint8_t device_address, uint16_t address, uint16_t data){
		//check for exceptions
		if(data != WRITE_COIL_OFF && data != WRITE_COIL_ON) return 0;

		//format and load response
		uint8_t data_bytes[4] = {uint8_t(address >> 8), uint8_t(address), uint8_t(data >> 8), uint8_t(data)};
		my_temp_transaction.load_transmission_data(
				device_address, write_single_coil, data_bytes, 4,
				WRITE_OR_GET_COUNTER_RESPONSE_LEN);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
    }

    /**
	 * @brief Format a write_single_register request, function code 06, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param address The address of the register to write to
     * @param data The value to write to the register
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
    int write_single_register_fn(uint8_t device_address, uint16_t address, uint16_t data){
		//exception checking
		if(data < 0 || data > MAX_WRITE_VALUE) {

			return 0;
		}

		uint8_t data_bytes[4] = {uint8_t(address >> 8), uint8_t(address), uint8_t(data >> 8), uint8_t(data)};
		my_temp_transaction.load_transmission_data(
				device_address, write_single_register, data_bytes, 4,
				WRITE_OR_GET_COUNTER_RESPONSE_LEN);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
    }

	/**
	 * @brief Format a read_exception_status request, function code 07, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	*/
	int read_exception_status_fn(uint8_t device_address){
		uint8_t* data_bytes = 0;
		my_temp_transaction.load_transmission_data(
				device_address, read_exception_status, data_bytes, 0,
				READ_EXCEPTION_STATUS_LEN);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

//	/**
//	 * @brief Format a diagnostics request, function code 08, according to the sub-function code passed and add the request to the buffer queue
//	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
//	 * @param sub_func The sub-function code that determines the type of diagnostic message/request
//	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
//	*/
//	int diagnostics_fn(uint8_t device_address, sub_function_codes_e sub_func){
//		uint8_t data_bytes[4] = {uint8_t(sub_func << 8), uint8_t(sub_func), uint8_t(0x00), uint8_t(0x00)};
//		my_temp_transaction.load_transmission_data(
//				device_address, diagnostics, data_bytes, 4,
//				get_diagnostic_reception_length(sub_func));
//		int ret = UART.enqueue_transaction(my_temp_transaction);
//		my_temp_transaction.reset_transaction();
//		return ret;
//	}
	
	/**
	 * @brief Format a return_query_data request, sub-function code 00, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param data The data to send and have echoed back from the server device
	 * @param num_data The quantity of data bytes being added to the transaction
	*/
	int return_query_data_fn(uint8_t device_address, uint8_t* data, int num_data){

		uint8_t data_bytes[2] = {uint8_t(return_query_data << 8), uint8_t(return_query_data)};
		my_temp_transaction.load_transmission_data(
				device_address, diagnostics, data_bytes, 2, data, num_data,
				num_data + 6);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}


	/**
	 * @brief Format a get_comm_event_counter request, function code 0x0B (11), and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	*/
	int get_comm_event_counter_fn(uint8_t device_address){
		uint8_t* data_bytes = 0;
		//uint8_t data_bytes[0];
		my_temp_transaction.load_transmission_data(
				device_address, get_comm_event_counter, data_bytes, 0,
				WRITE_OR_GET_COUNTER_RESPONSE_LEN);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}


	int write_multiple_coils_fn(uint8_t device_address, uint16_t starting_address, uint16_t num_coils, uint8_t* data) {
		//exception checking
		if(num_coils < 1 || num_coils > MAX_NUM_WRITE_COILS) return 0;

		uint8_t num_bytes;
		if(num_coils % 8 == 0){
			num_bytes = num_coils /= 8;
		} else {
			num_bytes = num_coils /= 8;
			num_bytes++;
		}

		// uint8_t data_bytes[5 + num_bytes] = {uint8_t(starting_address >> 8), uint8_t(starting_address), uint8_t(num_coils >> 8), uint8_t(num_coils), num_bytes};
		// for(int i = 0; i < num_bytes; i++) data_bytes[i + 5] = data[i];
		// return send_transaction(write_multiple_coils);

		uint8_t data_bytes[5] = {uint8_t(starting_address >> 8), uint8_t(starting_address), uint8_t(num_coils >> 8), uint8_t(num_coils), num_bytes};
		my_temp_transaction.load_transmission_data(
				device_address, write_multiple_coils, data_bytes, 5, data, num_bytes,
				WRITE_OR_GET_COUNTER_RESPONSE_LEN);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

	/**
	 * @brief Format a write_multiple_registers request, function code 16, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param starting_address The address of the first register to start writing to
     * @param num_registers The quantity of registers to write to
     * @param data An array of data that will be written, in order, to the registers beginning at starting_address
	 * @return An integer, 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
	int write_multiple_registers_fn(uint8_t device_address, uint16_t starting_address, uint16_t num_registers, uint8_t* data) {
		//exception checking
		if(num_registers < 1 || num_registers > MAX_NUM_WRITE_REG) return 0;

		uint8_t num_bytes = uint8_t(num_registers)*2;
		uint8_t data_bytes[5] = { uint8_t(starting_address >> 8), 
											  uint8_t(starting_address), 
											  uint8_t(num_registers >> 8), 
											  uint8_t(num_registers), 
											  num_bytes };
		my_temp_transaction.load_transmission_data(
				device_address, write_multiple_registers, data_bytes, 5, data, num_bytes,
				WRITE_OR_GET_COUNTER_RESPONSE_LEN);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;
	}

	/**
	 * @brief Format a report_server_id request, function code 17, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
	 */
//	int report_server_id_fn(uint8_t device_address){
//		uint8_t data_bytes[0];
//		my_temp_transaction.load_transmission_data(
//				device_address, report_server_id, data_bytes, 0,
//				//TODO: figure this out );
//		int ret = UART.enqueue_transaction(my_temp_transaction);
//		my_temp_transaction.reset_transaction();
//		return ret;
//	}

	/**
	 * @brief Format a mask_write_register request, function code 22, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
	 * @param address The address of the register to write to
	 * @param and_mask The value to preform a bitwise AND against the register contents with
	 * @param or_mask ??
	*/
//	int mask_write_register_fn(uint8_t device_address, uint16_t address, uint16_t and_mask, uint16_t or_mask){
//		uint8_t data_bytes[6] = { uint8_t(address >> 8),
//								  uint8_t(address),
//								  uint8_t(and_mask >> 8),
//								  uint8_t(and_mask),
//								  uint8_t(or_mask >> 8),
//								  uint8_t(or_mask) };
//		my_temp_transaction.load_transmission_data(
//				device_address, mask_write_register, data_bytes, 6,
//				get_reception_length(mask_write_register));
//		int ret = UART.enqueue_transaction(my_temp_transaction);
//		my_temp_transaction.reset_transaction();
//		return ret;
//	}

	/**
	 * @brief Format a read_write_multiple_registers request, function code 23, and add the request to the buffer queue
	 * @param device_address The address of the server device that will accept the transaction, 0 if broadcast
     * @param read_starting_address The address of the first register to start reading from
     * @param read_num_registers The quantity of registers to read from
	 * @param write_starting_address The address of the first register to write to
     * @param write_num_registers The quantity of registers to write to
     * @param data An array of data that will be written, in order, to the register(s) beginning at write_start_address
	 * @return An integer - 1 if the transaction is formatted and added to the buffer queue successfuly, 0 if an exception occurs
	 */
	int read_write_multiple_registers_fn(uint8_t device_address, 
										 uint16_t read_starting_address, uint16_t read_num_registers,
										 uint16_t write_starting_address, uint16_t write_num_registers,
										 uint8_t* data)
	{	
		//check for exceptions
		if(read_num_registers < 1 || read_num_registers > MAX_NUM_READ_REG) return 0;
		if(write_num_registers < 1 || write_num_registers > MAX_NUM_WRITE_REG_RW) return 0;

		uint8_t write_num_bytes = uint8_t(write_num_registers)*2;

		uint8_t data_bytes[9] = { uint8_t(read_starting_address >> 8), 
												    uint8_t(read_starting_address), 
													uint8_t(read_num_registers >> 8), 
													uint8_t(read_num_registers), 
													uint8_t(write_starting_address >> 8), 
													uint8_t(write_starting_address), 
													uint8_t(write_num_registers >> 8), 
													uint8_t(write_num_registers),
													write_num_bytes };
		//for(int i  = 0; i < write_num_bytes; i++) data_bytes[i + 9] = data[i];
		my_temp_transaction.load_transmission_data(
				device_address, read_write_multiple_registers, data_bytes, 9, data, write_num_bytes,
				5 + read_num_registers * 2);
		int ret = UART.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return ret;

	}


};


#endif /* MODBUS_APPLICATION_H_ */
