/**
   @file actuator.h
   @author Kali Erickson <kerickson@irisdynamics.com>, rebecca mcwilliam <rmcwilliam@irisdynamics.com>, kyle hagen <khagen@irisdynamics.com>, dan beddoes <dbeddoes@irisdynamics.com>
   @brief  Actuator object that abstracts the use of the modbus library/communications for communication with an Orca Series linear motor
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
	*@brief Sets the type of command that will be sent on high speed stream (ie when enable() has been used, this sets the type of message sent from enqueue motor frame)
	*/
	typedef enum {
		MotorCommand,
		MotorRead,
		MotorWrite
	}StreamMode;

	/**
	 * @brief this tracks the type of motor command stream that is currently being used
	 */
	typedef enum {

		SleepMode		= 1,
		ForceMode		= 2,
		PositionMode	= 3,
		HapticMode		= 4,
		KinematicMode	= 5

	} MotorMode;

	enum {
		ConstF	= 1 << 0,
		Spring0 = 1 << 1,
		Spring1 = 1 << 2,
		Spring2 = 1 << 3,
		Damper	= 1 << 4, 
		Inertia = 1 << 5,
		Osc0	= 1 << 6,
		Osc1	= 1	<< 7
	}HapticEffect;




	/**
	* @brief Write to the orca control register to change the mode of operation of the motor
	* note some modes require a constant stream to stay in that mode (eg. force, position)
	*/
	void set_mode(MotorMode orca_mode) {
		write_register(CTRL_REG_3, (uint8_t)orca_mode);
		comms_mode = orca_mode;
	}
	/**
	* @brief the communication mode determines which commands are sent by enqueue_motor_frame
	* *
	* @return CommunicationMode
	*/
	MotorMode get_mode() {
		return comms_mode;
	}

	/**
	* @brief Set the type of high speed stream to be sent on run out once handshake is complete
	*/

	void set_stream_mode(StreamMode mode) {
		stream_mode = mode;
	}

	/**	
	* @brief Get the current stream type to be sent on run out once handshake is complete
	*/
	StreamMode get_stream_mode() {
		return stream_mode;
	}

	/**
	* @brief This function can be continuously called and will update the values being sent when in motor write stream mode
	*/

	void update_write_stream(uint8_t width, uint16_t register_address, uint32_t register_value) {
		motor_write_data = register_value;
		motor_write_addr = register_address;
		motor_write_width = width;

	}

	/**
	* @brief This function can be continuously called and will update the values being sent when in motor read stream mode
	*/
	void update_read_stream(uint8_t width, uint16_t register_address) {
		motor_read_addr = register_address;
		motor_read_width = width;
	}


	/**
	* @brief Set/adjust the force that the motor is exerting when in motor_command stream mode
	* 
	* @param force force, in milli-Newtons
	*/
	void set_force_mN(int32_t force) {
		force_command = force;
		stream_timeout_start = modbus_client.get_system_cycles();
	}

	/**
	* @brief Set/adjust the position that the motor is aiming for when in motor command stream mode
	* 
	* @param position position, in micrometers
	*/
	void set_position_um(int32_t position) {
		position_command = position;
		stream_timeout_start = modbus_client.get_system_cycles();
	}

	/**
	* @brief Returns the total amount of force being sensed by the motor
	*
	* @return uint32_t - force in milli-Newtons
	*/
	int32_t get_force_mN() {
		return uint32_t(orca_reg_contents[FORCE_REG_H_OFFSET] << 16) | orca_reg_contents[FORCE_REG_OFFSET];
	}

	/**
	* @brief Returns the position of the shaft in the motor (distance from the zero position) in micrometers.
	*
	* @return uint32_t - position in micrometers
	*/
	int32_t get_position_um() {
		return (orca_reg_contents[POS_REG_H_OFFSET] << 16) | orca_reg_contents[POS_REG_OFFSET];
	}


	/**
	* @brief Enable or disabled desired haptic effects.
	*/
	void enable_haptic_effects(uint16_t effects) {
		write_register(HAPTIC_STATUS, effects);
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
				
				case motor_read: {
					u16 register_start_address = (response->get_tx_data()[0] << 8) + response->get_tx_data()[1];
					u8 width = response->get_tx_data()[2];
					u16 register_data = (response->get_rx_data()[2] << 8) + response->get_rx_data()[3];
					orca_reg_contents[register_start_address] = register_data;
					if (width > 1) {
						register_data = (response->get_rx_data()[0] << 8) + response->get_rx_data()[1];
						orca_reg_contents[register_start_address + 1] = register_data;
					}
					orca_reg_contents[MODE_OF_OPERATION] = response->get_rx_data()[4];
					orca_reg_contents[POS_REG_H_OFFSET] = (response->get_rx_data()[5] << 8) | response->get_rx_data()[6];
					orca_reg_contents[POS_REG_OFFSET] = (response->get_rx_data()[7] << 8) | response->get_rx_data()[8];
					orca_reg_contents[FORCE_REG_H_OFFSET] = (response->get_rx_data()[9] << 8) | response->get_rx_data()[10];
					orca_reg_contents[FORCE_REG_OFFSET] = (response->get_rx_data()[11] << 8) | response->get_rx_data()[12];
					orca_reg_contents[POWER_REG_OFFSET] = (response->get_rx_data()[13] << 8) | response->get_rx_data()[14];
					orca_reg_contents[TEMP_REG_OFFSET] = (response->get_rx_data()[15]);
					orca_reg_contents[VOLTAGE_REG_OFFSET] = (response->get_rx_data()[16] << 8) | response->get_rx_data()[17];
					orca_reg_contents[ERROR_REG_OFFSET] = (response->get_rx_data()[18] << 8) | response->get_rx_data()[19];
				}
					break; 
				case motor_write:
					orca_reg_contents[MODE_OF_OPERATION] = response->get_rx_data()[0];
					orca_reg_contents[POS_REG_H_OFFSET] = (response->get_rx_data()[1] << 8) | response->get_rx_data()[2];
					orca_reg_contents[POS_REG_OFFSET] = (response->get_rx_data()[3] << 8) | response->get_rx_data()[4];
					orca_reg_contents[FORCE_REG_H_OFFSET] = (response->get_rx_data()[5] << 8) | response->get_rx_data()[6];
					orca_reg_contents[FORCE_REG_OFFSET] = (response->get_rx_data()[7] << 8) | response->get_rx_data()[8];
					orca_reg_contents[POWER_REG_OFFSET] = (response->get_rx_data()[9] << 8) | response->get_rx_data()[10];
					orca_reg_contents[TEMP_REG_OFFSET] = (response->get_rx_data()[11]);
					orca_reg_contents[VOLTAGE_REG_OFFSET] = (response->get_rx_data()[12] << 8) | response->get_rx_data()[13];
					orca_reg_contents[ERROR_REG_OFFSET] = (response->get_rx_data()[14] << 8) | response->get_rx_data()[15];
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
	*@brief get the motor's mode of operations as currently updated by the local memory map
	*/
	uint16_t get_mode_of_operation() {
		return orca_reg_contents[MODE_OF_OPERATION];
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
	* @brief Returns true if the motor's firmware version is 'at least as recent' as the version designated 
	*		 by the parameters. 'At least as recent' can be thought of as a greater than or equal to comparison 
	*		 with version being the most significant digit, revision number being second most significant, and 
	*		 release state being the least significant.
	*
	* @param version - Desired major version number
	* @param release_state - Desired release state (0 - alpha, 1 - beta, 2 - release)
	* @param revision_number - Desired revision number
	* @return bool - True if motor's firmware version is at least as recent as the version designated by the parameters
	*/
	bool version_is_at_least(uint8_t version, uint8_t release_state, uint8_t revision_number) {
		return
			get_major_version() > version
			|| (get_major_version() == version && get_revision_number() > revision_number)
			|| (get_major_version() == version && get_revision_number() == revision_number && get_release_state() >= release_state);
	}

	/**
	 * @brief Set the zero position of the motor to be the current position 
	 */
	void zero_position(){
		write_register(ZERO_POS_REG_OFFSET, ZERO_POS_MASK);
	}

	/**
	 * @brief clear all errors stored on the motor
	 * note: errors that are still found will appear again
	 */
	void clear_errors(){
		write_register(CLEAR_ERROR_REG_OFFSET, CLEAR_ERROR_MASK);
	}

	/**
	 * @brief Copies the register for latched errors from the orca memory map into the local memory map 
	 * Latched errors are errors that were found by the motor, but are no longer active (not happening anymore)
	 */
	void get_latched_errors(){
		read_register(ERROR_1);
	}

	/**
	 * @brief Set the maximum force that the motor allows
	 * 
	 * @param max_force force in milli-Newtons
	 */
	void set_max_force(s32 max_force){
		uint8_t data[4] =	{
			uint8_t(max_force >> 8),
			uint8_t(max_force),
			uint8_t(max_force >> 24),
			uint8_t(max_force >> 16)
		};
		write_registers(USER_MAX_FORCE, 2, data);
	}

	/**
	 * @brief Set the maximum temperature that the motor allows
	 * 
	 * @param max_temp temperature in Celcius
	 */
	void set_max_temp(uint16_t max_temp){
		write_register(USER_MAX_TEMP  	,max_temp);
	}

	/**
	 * @brief Set the maximum power that the motor allows
	 * 
	 * @param max_power power in Watts
	 */
	void set_max_power(uint16_t max_power){
		write_register(USER_MAX_POWER 	,max_power);
	}

	/**
	 * @brief Sets the fade period when changing position controller tune in ms
	 * 
	 * @param t_in_ms time period in milliseconds
	*/
	void set_pctrl_tune_softstart(uint16_t t_in_ms){
		write_register(PC_SOFTSTART_PERIOD,t_in_ms);
	}

	/**
	 * @brief Sets the motion damping gain value used when communications are interrupted.
	 * 
	 * @param max_safety_damping damping value
	 */
	void set_safety_damping(uint16_t max_safety_damping){
		write_register(SAFETY_DGAIN 	,max_safety_damping);
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

		write_registers(PC_PGAIN, 6,  data);
		write_register(CONTROL_REG_1::address, CONTROL_REG_1::position_controller_gain_set_flag);
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
	void set_kinematic_motion(int ID,int32_t position, int32_t time, int16_t delay, int8_t type, int8_t auto_next, int8_t next_id = -1) {
		if (next_id == -1) {
			next_id = ID + 1;
		}
		
		uint8_t data[12] = {
							uint8_t(position >> 8),
							uint8_t(position),
					        uint8_t(position >> 24),
							uint8_t(position >> 16),
						    uint8_t(time >> 8),
							uint8_t(time),
							uint8_t(time >> 24),
							uint8_t(time >> 16),
							uint8_t(delay >> 8),
							uint8_t(delay),
							uint8_t (0),
							uint8_t((type<<1) | (next_id << 3) | auto_next)
							};
		write_registers(KIN_MOTION_0 + (6*ID), 6, data);
	}

	/** @brief update the spring effect in a single function
	*/
	void set_spring_effect(u8 spring_id, u16 gain, u32 center, u16 dead_zone = 0, u16 saturation = 0, u8 coupling = 0) {
		u8 data[12] = {
			u8(gain >> 8),
			u8(gain),
			u8(center >> 8),
			u8(center),
			u8(center >> 24),
			u8(center >> 16),
			u8(0),
			u8(coupling),
			u8(dead_zone >> 8),
			u8(dead_zone),
			u8(saturation >> 8),
			u8(saturation),
			
		};
		write_registers(S0_GAIN_N_MM + spring_id * 6, 6, data);
	}

	/**
	*/
	void set_osc_effect(u8 osc_id, u16 amplitude, u16 frequency_dhz, u16 duty, u16 type) {
		u8 data[8] = {
			u8(amplitude >> 8),
			u8(amplitude),
			u8(type >> 8),
			u8(type),
			u8(frequency_dhz >> 8),
			u8(frequency_dhz),
			u8(duty >> 8),
			u8(duty)
		};
		write_registers(O0_GAIN_N + osc_id * 4, 4, data);
	}


	/**
	* @brief Use the software trigger to start a kinematic motion, this will also run any chained motions 
	* @ID Identification of the motion to be triggered
	*/
	void trigger_kinematic_motion(int ID) {
		write_register(KIN_SW_TRIGGER, ID);
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
	 * @brief Request for multiple sequential registers in the local copy to be updated from the motor's memory map
	 *
	 * @param reg_address register address from the orca's memory map
	 * @param num_registers number of sequential registers to read
	 */
	void read_registers(uint16_t reg_address, uint16_t num_registers) {
		read_holding_registers_fn(connection_config.server_address, reg_address, num_registers);
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
	 * @brief Request for multiple registers in the motor's memory map to be updated with a given value.
	 *
	 * @param reg_address register address
	 * * @param num_registers number of sequential registers to write
	 * @param reg_data pointer to an array of data to be added to the registers
	 */
	void write_registers(uint16_t reg_address, uint16_t num_registers, uint8_t* reg_data) {
		write_multiple_registers_fn(connection_config.server_address, reg_address, num_registers, reg_data);
	}

	void write_registers(uint16_t reg_address, uint16_t num_registers, uint16_t* reg_data) {
		uint8_t data[126];
		for (int i = 0; i < num_registers; i++) {
			data[i*2] = reg_data[i] >> 8;
			data[i * 2 + 1] = reg_data[i];
		}
		write_multiple_registers_fn(connection_config.server_address, reg_address, num_registers, data);
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

	StreamMode stream_mode = MotorCommand;
	MotorMode comms_mode = SleepMode;

	uint32_t stream_timeout_start;
	uint32_t stream_timeout_cycles = 100000 * my_cycle_per_us;

	volatile bool new_data_flag = false;

	// Used to hold the last commanded force and position commands from the user of this object
	int32_t force_command;
	int32_t position_command;
	uint32_t haptic_command;
	//Used to hold the last data to stream in motor write and read streams
	uint32_t motor_write_data = 0;
	uint16_t motor_write_addr = 0;
	uint16_t motor_write_width = 1;
	uint16_t motor_read_addr = 0;
	uint16_t motor_read_width = 1;



	// These counters are used to find the success and failure rate of the comms
	int32_t success_msg_counter = 0, failed_msg_counter = 0;

	/**
	 * @brief Requests the actuator synchronize its memory map with the controller
	 */
	void synchronize_memory_map() override {
		read_registers(PARAM_REG_START     	, PARAM_REG_SIZE     			) ;
		read_registers(ERROR_0				, ADC_DATA_COLLISION-ERROR_0	) ;
		//read_registers(STATOR_CAL_REG_START	, STATOR_CAL_REG_SIZE			) ;
		//read_registers(SHAFT_CAL_REG_START 	, SHAFT_CAL_REG_SIZE 			) ;
		//read_registers(FORCE_CAL_REG_START 	, FORCE_CAL_REG_SIZE 			) ;
		read_registers(TUNING_REG_START    	, TUNING_REG_SIZE    			) ;
	}

	/**
	 * @brief Resets the memory map array to zeros
	 */
	void desynchronize_memory_map() override {
		for(int i=0; i < ORCA_REG_SIZE; i++){
			orca_reg_contents[i] = 0;
		}
	}
#define KIN_CMD 32 // Number that indicates a kinematic type motor frame. Not an actual register like POS_CMD and FORCE_CMD
#define HAP_CMD 34
	void motor_stream_command() {
		switch (comms_mode) {
			;
		case ForceMode: {
			if (uint32_t(modbus_client.get_system_cycles() - stream_timeout_start) > stream_timeout_cycles) {		//return to sleep mode if stream timed out
				comms_mode = SleepMode;
			}
			else {
				motor_command_fn(connection_config.server_address, FORCE_CMD, force_command);
			}
			break;
		}
		case PositionMode:
			if (uint32_t(modbus_client.get_system_cycles() - stream_timeout_start) > stream_timeout_cycles) {   //return to sleep mode if stream timed out
				comms_mode = SleepMode;
			}
			else {
				motor_command_fn(connection_config.server_address, POS_CMD, position_command);
			}
			break;
		case KinematicMode:
			motor_command_fn(connection_config.server_address, KIN_CMD, 0);
			break;
		case HapticMode:
			motor_command_fn(connection_config.server_address, HAP_CMD, 0);
			break;
		default:
			motor_command_fn(connection_config.server_address, 0, 0); //any register address other than force or position register_adresses will induce sleep mode and provided register_value will be ignored
			break;
		}
	}

	void motor_stream_read() {
		motor_read_fn(connection_config.server_address, motor_read_width, motor_read_addr);
	}

	void motor_stream_write() {
		motor_write_fn(connection_config.server_address, motor_write_width, motor_write_addr, motor_write_data);
	}

	/**
	 * @brief enqueue a motor message if the queue is empty
	 */
	void enqueue_motor_frame() {
		if(modbus_client.get_queue_size() >= 2) return;
		switch (stream_mode) {
		case MotorCommand:
			motor_stream_command();
			break;
		case MotorRead:
			motor_stream_read();
			break;
		case MotorWrite:
			motor_stream_write();
			break;
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
		case motor_read:
			return 24;
		case motor_write:
			return 20;
		default:
			return -1;
		}
	}

	/**
      @brief Enum of all actuator specific function codes, in decimal.
	 */
	enum orca_function_codes_e {
		motor_command = 100,
		motor_read = 104,
		motor_write = 105
	};

	/**
      @brief Format a motor command request, function code 0x64, and add the request to the buffer queue

      @param device_address Server device address
      @param command_code command code to specify command mode (sleep, force, position etc.)
      @param register_value The value to write to the register
	 */
	int motor_command_fn(uint8_t device_address, uint8_t command_code, int32_t register_value) {
		uint8_t data_bytes[5] = { 
				uint8_t(command_code),
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

	int motor_read_fn(uint8_t device_address, uint8_t width, uint16_t register_address) {
		uint8_t data_bytes[3] = {
				uint8_t(register_address>>8), 
				uint8_t(register_address),
				uint8_t(width)
		};

		my_temp_transaction.load_transmission_data(device_address, motor_read, data_bytes, 3, get_app_reception_length(motor_read));
		int check = modbus_client.enqueue_transaction(my_temp_transaction);
		my_temp_transaction.reset_transaction();
		return check;
	}

	int motor_write_fn(uint8_t device_address, uint8_t width, uint16_t register_address, uint32_t register_value) {
		uint8_t data_bytes[7] = {
				uint8_t(register_address >> 8),
				uint8_t(register_address),
				uint8_t(width),
				uint8_t(register_value >> 24),
				uint8_t(register_value >> 16),
				uint8_t(register_value >> 8),
				uint8_t(register_value)
		};

		my_temp_transaction.load_transmission_data(device_address, motor_write, data_bytes, 7, get_app_reception_length(motor_write));
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

