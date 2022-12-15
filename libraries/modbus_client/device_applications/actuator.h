/**
   @file actuator.h
   @author Kali Erickson <kerickson@irisdynamics.com>, kyle hagen <khagen@irisdynamics.com>, dan beddoes <dbeddoes@irisdynamics.com>, rebecca mcwilliam <rmcwilliam@irisdynamics.com>
   @brief  Actuator object that abstracts the use of the modbus library/communications
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

#ifndef ACTUATOR_H_
#define ACTUATOR_H_


#include "../iris_client_application.h"

#include "actuator_config.h"


/**
   @class Actuator
   @brief Object that abstracts the communications between the client and a Orca motor server.
 */
class Actuator : public IrisClientApplication {

public:

//Use appropriate device driver	
#ifdef IRIS_ZYNQ_7000
	Zynq7000_ModbusClient modbus_client;
#elif defined(__MK20DX256__)
	k20_ModbusClient modbus_client;
#elif defined(WINDOWS)
	windows_ModbusClient modbus_client;

	bool set_new_comport(int _comport) {
		//return value
		bool comport_set = false;
		int curr_comport = modbus_client.get_port_number();
		//checks to see if the new comport is the same as the old one, and if the motor is connected 
		//if the comports are different and the motor is not connected then the comport will be updated. 
		if (!(_comport == curr_comport && is_connected())) {
			modbus_client.set_new_comport(_comport);
		}
		return comport_set;
	}

	void disable_comport() {
		modbus_client.disable_comport_comms();
	}

#elif defined(QT_WINDOWS)
	qt_ModbusClient modbus_client;
#endif

    const uint32_t my_cycle_per_us;					//!< client device clock cycles per microsecond

public: 
	//Constructor
	Actuator(
			int channel,							
			const char * name,
			uint32_t cycle_per_us
	):
		IrisClientApplication(modbus_client, name, cycle_per_us),
		modbus_client(channel, cycle_per_us),
		my_cycle_per_us(cycle_per_us)
	{}

	/**
	 * @brief this tracks the type of motor command stream that is currently being used
	 */
	typedef enum {
		SleepMode		= 0b0000,
		ForceMode		= 0b0010,
		PositionMode	= 0b0100
	} CommunicationMode;


	/**
	 * @brief the communication mode determines which commands are sent by enqueue_motor_frame
	 * @param mode command stream type
	 */
	void set_mode(CommunicationMode mode) {
		comms_mode = mode;
	}

	/**
	 * @brief the communication mode determines which commands are sent by enqueue_motor_frame
	 * * 
	 * @return CommunicationMode
	 */
	CommunicationMode get_mode() {
		return comms_mode;
	}

	/**
	* @brief Set/adjust the force that the motor is exerting 
	* 
	* @param force force, in milli-Newtons
	*/
	void set_force_mN(int32_t force) {
		force_command = force;
		stream_timeout_start = modbus_client.get_system_cycles();
	}

	/**
	* @brief Set/adjust the position that the motor is aiming for
	* 
	* @param position position, in micrometers
	*/
	void set_position_um(int32_t position) {
		position_command = position;
		stream_timeout_start = modbus_client.get_system_cycles();
	}

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
	* @brief Set the maximum time required between calls to set_force or set_position, in force or position mode respectively, before timing out and returning to sleep mode. 
	* 
	* @param timout_us time in microseconds
	*/
	void set_stream_timeout(uint64_t timeout_us){
		stream_timeout_cycles = timeout_us * my_cycle_per_us;
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
				enqueue_motor_frame();
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

				case read_holding_registers:{
					// add the received data to the local copy of the memory map
					u16 register_start_address 		= (response->get_tx_data()[0] << 8) + response->get_tx_data()[1];
					u16 num_registers 				= (response->get_tx_data()[2] << 8) + response->get_tx_data()[3];
					for ( int i = 0; i < num_registers; i++ ) {
						u16 register_data 			= (response->get_rx_data()[1 + i*2] << 8) + response->get_rx_data()[2 + i*2];
						orca_reg_contents[register_start_address + i] = register_data;
					}
					break;
				}
				case write_single_register:
					// nothing to do
					break;

				case motor_command:
					orca_reg_contents[POS_REG_H_OFFSET] 	= (response->get_rx_data()[ 0] << 8) | response->get_rx_data()[ 1];
					orca_reg_contents[POS_REG_OFFSET]  		= (response->get_rx_data()[ 2] << 8) | response->get_rx_data()[ 3];
					orca_reg_contents[FORCE_REG_H_OFFSET] 	= (response->get_rx_data()[ 4] << 8) | response->get_rx_data()[ 5];
					orca_reg_contents[FORCE_REG_OFFSET] 	= (response->get_rx_data()[ 6] << 8) | response->get_rx_data()[ 7];
					orca_reg_contents[POWER_REG_OFFSET] 	= (response->get_rx_data()[ 8] << 8) | response->get_rx_data()[ 9];
					orca_reg_contents[TEMP_REG_OFFSET] 		= (response->get_rx_data()[10]);
					orca_reg_contents[VOLTAGE_REG_OFFSET] 	= (response->get_rx_data()[11] << 8) | response->get_rx_data()[12];
					orca_reg_contents[ERROR_REG_OFFSET] 	= (response->get_rx_data()[13] << 8) | response->get_rx_data()[14];
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
	* @brief Returns the total amount of force being sensed by the motor
	* 
	* @return uint32_t - force in milli-Newtons
	*/
	int32_t get_force_mN(){
		return uint32_t(orca_reg_contents[FORCE_REG_H_OFFSET]<<16) | orca_reg_contents[FORCE_REG_OFFSET];
	}

	/**
	* @brief Returns the position of the shaft in the motor (distance from the zero position) in micrometers. 
	* 
	* @return uint32_t - position in micrometers
	*/
	int32_t get_position_um(){
		return (orca_reg_contents[POS_REG_H_OFFSET]<<16) | orca_reg_contents[POS_REG_OFFSET];
	}

	/**
	* @brief Returns the amount of power being drawn by the motor, in Watts
	* 
	* @return uint16_t - power in Watts
	*/
	uint16_t get_power_W(){
		return orca_reg_contents[POWER_REG_OFFSET];
	}

	/**
	* @brief Returns the temperature of the motor in Celcius
	* 
	* @return uint8_t - temperature in Celcius
	*/
	uint8_t get_temperature_C(){
		return orca_reg_contents[TEMP_REG_OFFSET];
	}

	/**
	* @brief Returns the amount of voltage the motor is recieving, in milli-Volts. 
	* 
	* @return uint16_t - voltage in milli-Voltage 
	*/
	uint16_t get_voltage_mV(){
		return orca_reg_contents[VOLTAGE_REG_OFFSET];
	}

	/**
	* @brief Returns the sum of all error messages being sent by the motor
	* 
	* @return uint16_t - sum or all active error codes
	*/
	uint16_t get_errors(){
		return orca_reg_contents[ERROR_REG_OFFSET];
	}

	/**
	* @brief Returns the actuator serial number
	* 
	* @return uint32_t - actuator serial number
	*/
	uint32_t get_serial_number() {

		uint16_t sn_high = orca_reg_contents[SERIAL_NUMBER_HIGH];
		uint16_t sn_low  = orca_reg_contents[SERIAL_NUMBER_LOW];

		uint32_t serial_number = (sn_high * (1<<16)) + sn_low;
		return serial_number;
	}

	/**
	* @brief Return the firmware major version
	* 
	* @return uint16_t - firmware major version
	*/
	uint16_t get_major_version() {
		return orca_reg_contents[MAJOR_VERSION];
	}

	/**
	* @brief Return the firmware release state (minor version)
	* 
	* @return uint16_t - firmware release state
	*/
	uint16_t get_release_state() {
		return orca_reg_contents[RELEASE_STATE];
	}

	/**
	* @brief Return the firmware revision number
	* 
	* @return uint16_t - firmware revision number
	*/
	uint16_t get_revision_number() {
		return orca_reg_contents[REVISION_NUMBER];
	}

	/**
	 * @brief Set the zero position of the motor to be the current position 
	 */
	void zero_position(){
		write_single_register_fn(connection_config.server_address, ZERO_POS_REG_OFFSET, ZERO_POS_MASK);
	}

	/**
	 * @brief clear all errors stored on the motor
	 * note: errors that are still found will appear again
	 */
	void clear_errors(){
		write_single_register_fn(connection_config.server_address, CLEAR_ERROR_REG_OFFSET, CLEAR_ERROR_MASK);
	}

	/**
	 * @brief Copies the register for latched errors from the orca memory map into the local memory map 
	 * Latched errors are errors that were found by the motor, but are no longer active (not happening anymore)
	 */
	void get_latched_errors(){
		read_holding_registers_fn(connection_config.server_address, ERROR_1, 1);   
	}

	/**
	 * @brief Set the maximum force that the motor allows
	 * 
	 * @param max_force force in milli-Newtons
	 */
	void set_max_force(s32 max_force){
		write_single_register_fn(1,USER_MAX_FORCE 	,max_force);
	}

	/**
	 * @brief Set the maximum temperature that the motor allows
	 * 
	 * @param max_temp temperature in Celcius
	 */
	void set_max_temp(uint16_t max_temp){
		write_single_register_fn(1,USER_MAX_TEMP  	,max_temp);
	}

	/**
	 * @brief Set the maximum power that the motor allows
	 * 
	 * @param max_power power in Watts
	 */
	void set_max_power(uint16_t max_power){
		write_single_register_fn(1,USER_MAX_POWER 	,max_power);
	}

	/**
	 * @brief Sets the fade period when changing position controller tune in ms
	 * 
	 * @param t_in_ms time period in milliseconds
	*/
	void set_pctrl_tune_softstart(uint16_t t_in_ms){
		write_single_register_fn(1, PC_SOFTSTART_PERIOD,t_in_ms);
	}

	/**
	 * @brief Sets the motion damping gain value used when communications are interrupted.
	 * 
	 * @param max_safety_damping damping value
	 */
	void set_safety_damping(uint16_t max_safety_damping){
		write_single_register_fn(1,SAFETY_DGAIN 	,max_safety_damping);
	}

	/**
	 * @brief Sets the PID tuning values on the motor in non-scheduling mode. Disabled the gain scheduling in motors that support it
	 * 
	 * @param pgain proportional gain
	 * @param igain integral gain
	 * @param dgain derivative gain
	 * @param sat maximum force (safety value)
	 */
	void tune_position_controller(uint16_t pgain, uint16_t igain, uint16_t dvgain, uint32_t sat, uint16_t degain=0) {

		uint8_t data[12] = {
				uint8_t(pgain>>8),
				uint8_t(pgain),
				uint8_t(igain>>8),
				uint8_t(igain),
				uint8_t(dvgain>>8),
				uint8_t(dvgain),
				uint8_t(degain>>8),
				uint8_t(degain),
				uint8_t(sat>>8),
				uint8_t(sat),
				uint8_t(sat>>24),
				uint8_t(sat>>16)
		};

		write_multiple_registers_fn	(1, PC_PGAIN, 6,  data);
		write_single_register_fn	(1, CONTROL_REG_1::address, CONTROL_REG_1::position_controller_gain_set_flag);
	}


	/**
	* @brief Set the motor to kinematic mode and disable high frequency stream
	*/
	void enable_kinematic() {
		enabled = false; 
		write_register(CTRL_REG_3, CONTROL_REG_3::kinematic_control_sid );
	}

	/**
	* @brief Set the overall kinematic configuration
	* @param num_motions number of configured motions
	* @param trig_period when hardware triggering is enabled this will be the debounce option for the button 0 = 0 ms, 1 = 10 ms, 2 = 50 ms, 3 = 100 ms
	* @param HW_trig Enabling of the hardware triggering option, this will prevent further modbus communication.
	*/
	void set_kinematic_config(int8_t num_motions, int8_t trig_period = 0, int8_t HW_trig = 0 ) {
		uint16_t data = (trig_period << 7) | (HW_trig << 6) | (num_motions-1);
		write_single_register_fn(1, KIN_CONFIG, data);
	}

	/**
	* @brief Set the parameters to define a kinematic motion 
	* @param ID	Motion identifier
	* @param position Target position to reach
	* @param time	Time to get to the target
	* @param chain_delay	delay between this motion and the next
	* @param type	0 = minimize power, 1 = maximize smoothness
	* @param chain	Enable linking this motion to the next
	*/
	void set_kinematic_motion(int ID,int32_t position, int32_t time, int16_t chain_delay, int8_t type, int8_t chain) {
		uint8_t data[12] = {
							uint8_t(position >> 8),
							uint8_t(position),
					        uint8_t(position >> 24),
							uint8_t(position >> 16),
						    uint8_t(time >> 8),
							uint8_t(time),
							uint8_t(time >> 24),
							uint8_t(time >> 16),
							uint8_t(chain_delay >> 8),
							uint8_t(chain_delay),
							uint8_t (0),
							uint8_t((type<<1) | chain)
							};
		write_multiple_registers_fn(1, KIN_MOTION_0 + (6*ID), 6, data);
	}



	/**
	* @brief Use the software trigger to start a kinematic motion, this will also run any chained motions 
	* @ID Identification of the motion to be triggered
	*/
	void trigger_kinematic_motion(int ID) {
		write_single_register_fn(1, KIN_SW_TRIGGER, ID);
	}

	/**
	 * @brief Request for a specific register in the local copy to be updated from the motor's memory map
	 * 
	 * @param reg_address register address
	 */
	void read_register(uint16_t reg_address){    
		read_holding_registers_fn(connection_config.server_address, reg_address, 1);
	}

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
		return orca_reg_contents[offset];
	}

private:

	uint16_t orca_reg_contents[ORCA_REG_SIZE];

	CommunicationMode comms_mode = SleepMode;

	uint32_t stream_timeout_start;
	uint32_t stream_timeout_cycles = 100000 * my_cycle_per_us;

	volatile bool new_data_flag = false;

	// Used to hold the last commanded force and position commands from the user of this object
	int32_t force_command;
	int32_t position_command;

	// These counters are used to find the success and failure rate of the comms
	int32_t success_msg_counter = 0, failed_msg_counter = 0;


	/**
	 * @brief Requests the actuator synchronize its memory map with the controller
	 */
	void synchronize_memory_map() override {
		read_holding_registers_fn(1, PARAM_REG_START     	, PARAM_REG_SIZE     			) ;
		read_holding_registers_fn(1, ERROR_0				, ADC_DATA_COLLISION-ERROR_0	) ;
		read_holding_registers_fn(1, STATOR_CAL_REG_START	, STATOR_CAL_REG_SIZE			) ;
		read_holding_registers_fn(1, SHAFT_CAL_REG_START 	, SHAFT_CAL_REG_SIZE 			) ;
		read_holding_registers_fn(1, FORCE_CAL_REG_START 	, FORCE_CAL_REG_SIZE 			) ;
		read_holding_registers_fn(1, TUNING_REG_START    	, TUNING_REG_SIZE    			) ;
	}

	/**
	 * @brief Resets the memory map array to zeros
	 */
	void desynchronize_memory_map() override {
		for(int i=0; i < ORCA_REG_SIZE; i++){
			orca_reg_contents[i] = 0;
		}
	}

	/**
	 * @brief enqueue a motor message if the queue is empty
	 */
	void enqueue_motor_frame() {


		if(modbus_client.get_queue_size() >= 2) return;

		switch(comms_mode){
		case SleepMode:
			motor_command_fn(connection_config.server_address, 0, 0); //any register address other than force or position register_adresses will induce sleep mode and provided register_value will be ignored
			break;
		case ForceMode: {
			if(uint32_t(modbus_client.get_system_cycles() - stream_timeout_start) > stream_timeout_cycles){		//return to sleep mode if stream timed out
				comms_mode = SleepMode;
			}
			else {
				motor_command_fn(connection_config.server_address, FORCE_CMD, force_command);
			}
			break;
		}
		case PositionMode:
			if(!POS_CTRL){
				return;
			}

			if(uint32_t(modbus_client.get_system_cycles() - stream_timeout_start) > stream_timeout_cycles){   //return to sleep mode if stream timed out
				comms_mode = SleepMode;
			}
			else {
				motor_command_fn(connection_config.server_address, POS_CMD, position_command);
			}
			break;
		//case KinematicMode:
		//		motor_command_fn(connection_config.server_address, 32, 0);
		//	break;
		}

	}

	/**
	 * @brief Determine the length of the request for an application specific function code
	 * 
	 * @param fn_code application specific function code
	 * @return int - length of request
	 */
	int get_app_reception_length(uint8_t fn_code){
		switch(fn_code){
		case motor_command:
			return 19;
		default:
			return -1;
		}
	}

	/**
      @brief Enum of all actuator specific function codes, in decimal.
	 */
	enum orca_function_codes_e {
		motor_command = 100
	};

	/**
      @brief Format a motor command request, function code 65, and add the request to the buffer queue

      @param device_address Server device address
      @param register_address The address of the register to be written to
      @param register_value The value to write to the register
	 */
	int motor_command_fn(uint8_t device_address, uint8_t register_address, int32_t register_value) {
		uint8_t data_bytes[5] = { uint8_t(register_address),
				uint8_t(register_value >> 24),
				uint8_t(register_value >> 16),
				uint8_t(register_value >> 8),
				uint8_t(register_value)
		};
		my_temp_transaction.load_transmission_data(device_address, motor_command, data_bytes, 5, get_app_reception_length(motor_command));
		int check = modbus_client.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return check;
	}

	////////////////////////////////////////////////////////////////////
};

#ifdef IRIS_ZYNQ_7000
extern Actuator actuator[6];
#else
extern Actuator actuator;
#endif

#endif

