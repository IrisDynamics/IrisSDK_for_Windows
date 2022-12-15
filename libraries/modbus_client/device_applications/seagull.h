/**
   @file seagull.h
   @author rebecca mcwilliam <rmcwilliam@irisdynamics.com>
   @brief  Seagull object that abstracts the use of the modbus library/communications for communications between seagull and Orca for current loop applications
   only the current loop section of the memory map can be read to using the seagull device 
   @version 2.2.0
    
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

#ifndef SEAGULL_H_
#define SEAGULL_H_


#include "../iris_client_application.h"
//#include "../device_drivers/atmega328/modbus_client_328.h"
#include "seagull_config.h"

#include "../../../orca600_api/orca600.h"


/**
   @class Seagull
   @brief Object that abstracts the communications between the client and a Orca motor server being commanded with 4+-20 mA current.
 */
class Seagull : public IrisClientApplication {

public:

	atmega328_ModbusClient modbus_client;

    const uint32_t my_cycle_per_us;					//!< client device clock cycles per microsecond

	uint16_t current_in;

public: 
	//Constructor
	Seagull(
			int channel,							
			const char * name,
			uint32_t cycle_per_us
	):
		IrisClientApplication(modbus_client, name, cycle_per_us),
		modbus_client(channel, cycle_per_us),
		my_cycle_per_us(cycle_per_us)
	{}

	/**
	 * @brief returns true when new data has been received from an actuator since the last time this function was called
	 * 
	 * @return boolean - returns true when new data has been received from the motor. 
	 */
	bool new_data() override {
		bool return_value = new_data_flag;
		new_data_flag = false;
		return return_value;
	}

	/**
	*@brief Get to a good handshake init state and set up the device driver with the default baud rate
	*/
	void init(){
		disconnect();	// dc is expected to return us to a good init state
		modbus_client.init(UART_BAUD_RATE);
	}

	/**
	 * @brief returns the number of successful messages between the actuator and the controller
	 * 
	 * @return uint16_t, number of successful messages
	 */
	uint16_t get_num_successful_msgs() {
		return success_msg_counter;
	}

	/**
	 * @brief returns the number of failed messages between the actuator and the controller
	 * 
	 * @return uint16_t, number of failed messages
	 */
	uint16_t	get_num_failed_msgs() {
		return failed_msg_counter;
	}

	void printme() { 
#ifdef IRISCONTROLS
		PRINTDL("am enabled ", is_enabled());
		PRINTDL("connected state ", connection_state);
#endif
	}

	/*@brief set the value for the 4-20 received value;
	* normalizes it to a 16 bit data range for the orca
	*/
	void set_4_20(uint16_t cur) {	
		current_in = cur;
	}

	uint16_t get_ch1_current() {
		return iloop_reg_contents[ILOOP_CH1_OFFSET];
	}
	uint16_t get_ch2_current() {
		return iloop_reg_contents[ILOOP_CH2_OFFSET];
	}
	uint16_t get_errors() {
		return iloop_reg_contents[ERROR_REG_OFFSET];
	}

	/**
	 * @brief handle the motor frame transmissions cadence
	 * @
	 * This dispatches transmissions for motor frames when connected and dispatches handshake messages when not.
	 * This function must be externally paced... i.e. called at the frequency that transmission should be sent
	 */
	void run_out() {
		
		// This object can queue messages on the UART with the either the handshake or the connected run loop
		if ( is_enabled() ) {
			
			if (connection_state != connected) {
				
				modbus_handshake();
				
			}
			else {
				enqueue_seagull_command();
			}
		}
		// This function results in the UART sending any data that has been queued
		modbus_client.run_out();
	}

	/**
	 * @brief Incoming message parsing and connection handling
	 *
	 * Polls uart polled timers
	 * Claims responses from the message queue.
	 * Maintains the connection state based on consecutive failed messages
	 * Parses successful messages
	 */
	void run_in() {

		
		modbus_client.run_in();

		if ( modbus_client.is_response_ready() ) {
			//digitalWrite(LED_BUILTIN, HIGH);
			response = modbus_client.dequeue_transaction();
			new_data_flag = true;		// communicate to other layers that new data was received

			if ( !response->is_reception_valid() ) {
				cur_consec_failed_msgs++;
				failed_msg_counter++;
				if(connection_state == connected && cur_consec_failed_msgs >= connection_config.max_consec_failed_msgs){
					disconnect();
				}
			}

			// Response was valid
			else {

				cur_consec_failed_msgs = 0;
				success_msg_counter++;

				switch (response->get_rx_function_code()) {
				///only able to read registers in the Current loop section of the orca memory map
				case read_holding_registers:{

					// Just don't allow reading holding registers since the seagull only cares about the 4 values defined in seagull_config.h

					// u16 register_start_address 		= ((response->get_tx_data()[0] << 8) + response->get_tx_data()[1]) - ILOOP_REG_START;
					// u16 num_registers 				= (response->get_tx_data()[2] << 8) + response->get_tx_data()[3];
					// for ( int i = 0; i < num_registers; i++ ) {
					// 	u16 register_data 			= (response->get_rx_data()[1 + i*2] << 8) + response->get_rx_data()[2 + i*2];
					// 	iloop_reg_contents[register_start_address + i] = register_data;
					// }
					break;
				}
				case seagull_command: {
					iloop_reg_contents[ILOOP_CH1_OFFSET] = (response->get_rx_data()[0] << 8) | response->get_rx_data()[1];
					iloop_reg_contents[ILOOP_CH2_OFFSET] = (response->get_rx_data()[2] << 8) | response->get_rx_data()[3];
					iloop_reg_contents[ERROR_REG_OFFSET] = (response->get_rx_data()[4] << 8) | response->get_rx_data()[5];
				}
				break;
				case write_single_register:
					// nothing to do
					break;
				case read_coils                   :
				case read_discrete_inputs         :
				case read_input_registers         :
				case write_single_coil            :
				case read_exception_status        :
				case diagnostics                  :
				case get_comm_event_counter       :
				case get_comm_event_log           :
				case write_multiple_coils         :
				case write_multiple_registers     :
				case report_server_id             :
				case mask_write_register          :
				case read_write_multiple_registers:
				default:
					// todo: warn about un-implemented function codes being received
					break;
				}
			}
		}
	}

	/**
	 * @brief provide access to the modbus client isr function for linking to an interrupt controller
	 */
	void isr() {
		UART.uart_isr();
	}



	/**
	* @brief return the name of the actuator object 
	* 
	* @return char* - Name of the actuator object
	*/
	const char* get_name(){
		return my_name;
	}

	/**
	* @brief Returns the UART channel number in use
	* 
	* @return int, channel number
	*/
	int channel_number() {
		return UART.channel_number;
	}

	/**
	 * @brief Request for a specific register in the local copy to be updated from the motor's memory map
	 * 
	 * @param reg_address register address
	 */
	// void read_register(uint16_t reg_address){    
	// 	read_holding_registers_fn(connection_config.server_address, reg_address, 1);
	// }

	/**
	 * @brief Request for a specific register in the motor's memory map to be updated with a given value.
	 * 
	 * @param reg_address register address
	 * @param reg_data data to be added to the register
	 */
	void write_register(uint16_t reg_address, uint16_t reg_data){    
		write_single_register_fn(connection_config.server_address, reg_address, reg_data);
	}

	/**
	* @brief Return the contents of the given register from the controller's copy of the motor's memory map. 
	* 
	* @param offset the register that will be read
	* @return uint16_t - register contents
	*/
	uint16_t get_orca_reg_content(uint16_t offset){
		return iloop_reg_contents[offset];
	}

private:

	/**
	 * @brief Determine the length of the request for an application specific function code
	 * 
	 * @param fn_code application specific function code
	 * @return int - length of request
	 */
	int get_app_reception_length(uint8_t fn_code){
		switch(fn_code){
		case seagull_command:
			return 10;
		default:
			return -1;
		}
	}

	/**
      @brief Enum of all seagull specific function codes, in decimal.
	 */
	enum seagull_function_codes_e {
		seagull_command = 102
	};

	/**
      @brief Format a seagull command request, function code 102, and add the request to the buffer queue

      @param device_address Server device address
      @param register_value The value to write to the input register
	 */
	int seagull_command_fn(uint8_t device_address, uint16_t register_value) {
		uint8_t data_bytes[2] = {
				uint8_t(register_value >> 8),
				uint8_t(register_value)
		};
		my_temp_transaction.load_transmission_data(device_address, seagull_command, data_bytes, 2, get_app_reception_length(seagull_command));
		int check = modbus_client.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return check;
	}

	void enqueue_seagull_command() {
		if(modbus_client.get_queue_size() >= 2) return; // Taken from enqueue motor frame, necessary?
		seagull_command_fn(connection_config.server_address, current_in);
	}

	uint16_t iloop_reg_contents[ILOOP_REG_SIZE];

	volatile bool new_data_flag = false;

	// These counters are used to find the success and failure rate of the comms
	int32_t success_msg_counter = 0, failed_msg_counter = 0;


	/**
	 * @brief Requests the actuator synchronize its memory map with the controller
	 */
	void synchronize_memory_map() override {
	//	read_holding_registers_fn(1, PARAM_REG_START     	, PARAM_REG_SIZE     			) ;
	}

	/**
	 * @brief Resets the memory map array to zeros
	 */
	void desynchronize_memory_map() override {
		for(int i=0; i < ILOOP_REG_SIZE; i++){
			iloop_reg_contents[i] = 0;
		}
	}

	////////////////////////////////////////////////////////////////////
};


#endif

