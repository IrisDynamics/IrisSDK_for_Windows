/**
   @file pneumatic_controller.h
   @author Rebecca McWilliam <rmcwilliam@irisdynamics.com> Kali Erickson <kerickson@irisdynamics.com> kyle hagen <khagen@irisdynamics.com>
   @brief  Pneumatic Controller object that abstracts the use of the modbus library/ communication with an Pneumatic controller server (Eagle with Pneumatic Hat IO)
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

#ifndef PNEUMATIC_CONTROLLER_H_
#define PNEUMATIC_CONTROLLER_H_

#include "pneumatic_ctrl.h"
#include "../../shared/mb_config.h"
#include "actuator.h"

#ifdef IRIS_ZYNQ_7000
#include "../device_drivers/zynq7000/zynq7000_modbus_client.h"
#elif defined(__MK20DX256__)
#include "../device_drivers/k20/modbus_client_k20.h"
#elif defined(WINDOWS)
#include "windows_modbus_client.h"
#elif defined(QT_WINDOWS)
#include "qt_modbus_client.h"
#endif

#define OPEN_VALVE_VOLTAGE  24000  	//mV
#define	LIGHT_ON_VOLTAGE	200	// mV

#define PRESSURE_ENVELOPE	1
#define FORCE_ENVELOPE		2000

class PneumaticController : public IrisClientApplication {

   friend class GUI;

   int device_address = 1;

   //Use appropriate device driver	
#ifdef IRIS_ZYNQ_7000
   Zynq7000_ModbusClient modbus_client;
#elif defined(__MK20DX256__)
   k20_ModbusClient modbus_client;
#elif defined(WINDOWS)
   windows_ModbusClient modbus_client;
#elif defined(QT_WINDOWS)
   qt_ModbusClient modbus_client;
#endif

   const uint32_t my_cycle_per_us;

   volatile bool new_data_flag = false;

   uint16_t pneumatic_reg_contents[PNEUMATIC_REG_SIZE];

   // These ers are used to find the success and failure rate of the comms
   uint32_t success_msg_counter = 0, failed_msg_counter = 0;

   uint16_t solenoid_states = 0;

   public:
   uint16_t target_pressure[3];

   enum{
	   flashy_light,
	   big_tank_supply,
       actuator_vents,
       actuator_supply,
       front_fill,
       front_drain,
       rear_drain,
       rear_fill,
       mid_drain,
       mid_fill
   }solenoid_names;
//
   typedef enum{
	   front,
	   mid,
	   rear,
	   feed
   }tank;

   typedef enum{
	   vent_mode,
	   lock_mode,
	   minimize_power,
	   seek_mode,
	   manual
   }mode;

   enum{
	   hold,
	   up,
	   down
   }hysteresis;

   mode behaviour_mode = manual;

   bool state_target_reached = false;

   int seeking_states[3] = {hold, hold, hold};
   int min_power_states[3] = {hold, hold, hold};

   bool alt_command=false;


   PneumaticController(
		int channel,
		const char * name,
		uint32_t cycle_per_us
   ):
		IrisClientApplication(modbus_client, name, cycle_per_us),
		modbus_client(channel, cycle_per_us),
		my_cycle_per_us(cycle_per_us)
   {}


	void init(){
		disconnect();
		modbus_client.init(UART_BAUD_RATE);
	}

	/// returns the last-known memory contents of the passed register
	int get_reg_contents(int v) {
		return pneumatic_reg_contents[v];
	}


	bool solenoid_action(Actuator* actuator){
		bool ret = false;
		switch(behaviour_mode){
			case vent_mode:
				ret = vent();
				break;
			case lock_mode:
				ret = lock();
				break;
			case minimize_power:
				ret = min_powers(actuator);
				break;
			case seek_mode:
				ret = seek_targets();
				break;
			case manual:
				ret = false;
				break;
		}
		return ret;

	}

	/*
	 * @brief vent all tanks and ensure none are filling
	 * @return return true if all tanks have 0 PSI
	 */
	bool vent(){
		if ((get_tank_pressure_PSI(front) < 1) && (get_tank_pressure_PSI(mid) < 1) &&  (get_tank_pressure_PSI(rear) < 1)) return true;
		solenoid_states &= ~((1<<front_fill) | (1<<mid_fill)| (1<<rear_fill));
		solenoid_states |= ((1<<front_drain) | (1<<mid_drain)| (1<<rear_drain));
		return false;
	}

	/*
	 * @brief lock the current pressure in the tanks by closing all solenoids
	 * @return return true when locked
	 */
	bool lock(){
		power_off_all();
		return true;
	}

	bool min_powers(Actuator* actuator){
		int targets_reached = 0;
		int force_avg[3];
		force_avg[front] = (actuator[2].get_force_mN() + actuator[3].get_force_mN())>>1;
		force_avg[mid] = (actuator[1].get_force_mN() + actuator[4].get_force_mN())>>1;
		force_avg[rear] = (actuator[0].get_force_mN() + actuator[5].get_force_mN())>>1;
		targets_reached += min_power(front, force_avg[front]) + min_power(mid, force_avg[mid]) + min_power(rear, force_avg[rear]);
		if (targets_reached == 3) return true;
		else return false;
	}

	bool min_power(int section, int force_avg){
		switch(min_power_states[section]){
		case hold:
			drain_tank(section, false);
			fill_tank(section, false);
			if (force_avg > ( FORCE_ENVELOPE))	min_power_states[section] = up;
			else if (force_avg < (-FORCE_ENVELOPE))	min_power_states[section] = down;
			return true;
			break;
		case up:
			if (force_avg <= 0)	min_power_states[section] = hold;
			else fill_tank(section, true);
			return false;
			break;
		case down:
			if (force_avg >= 0)	min_power_states[section] = hold;
			else drain_tank(section, true);
			return false;
			break;
		}
		return false;
	}


	bool seek_targets(){
		int targets_reached = 0;
		targets_reached += seek_target(front) + seek_target(mid) + seek_target(rear);
		if (targets_reached == 3) return true;
		else return false;
	}

	/*
	 * @brief Seek to reach a target pressure, hysteresis to not bounce around
	 * @param tank section to seek on
	 * @return true if holding target if seeking false
	 */
	bool seek_target(int section){
		switch(seeking_states[section]){
		case hold:
			drain_tank(section, false);
			fill_tank(section, false);
			if (get_tank_pressure_PSI(section) < (target_pressure[section] - PRESSURE_ENVELOPE))	seeking_states[section] = up;
			else if (get_tank_pressure_PSI(section) > (target_pressure[section] + PRESSURE_ENVELOPE))	seeking_states[section] = down;
			return true;
			break;
		case up:
			if (get_tank_pressure_PSI(section) >= target_pressure[section])	seeking_states[section] = hold;
			else fill_tank(section, true);
			return false;
			break;
		case down:
			if (get_tank_pressure_PSI(section) <= target_pressure[section])	seeking_states[section] = hold;
			else drain_tank(section, true);
			return false;
			break;
		}
		return false;
	}
	/**
	* @brief handle the motor frame transmissions cadence
	*
	* This dispatches transmissions for motor frames when connected and dispatches handshake messages when not.
	* This function must be exteranlly paced... i.e. called at the frequency that transmission should be sent
	*/
	void run_out() {

		// This object can queue messages on the UART with the either the handshake or the connected run loop
		if ( is_enabled() ) {

			if (connection_state != connected) {
				modbus_handshake();
			}
			else {
				if (alt_command){
					enqueue_read_status();

				}
				else{
					send_solenoid_states();
				}
				alt_command = !alt_command;

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
					u16 num_registers			 = response->get_rx_data()[0];
					u16 register_address 		 = uint16_t(response->get_tx_data()[0] << 8) + response->get_tx_data()[1];
					int reg_index = 0;
					for (int i = 1; i<num_registers; i +=2){
						u16 register_data = uint16_t(response->get_rx_data()[i] << 8) + response->get_rx_data()[i+1];
						pneumatic_reg_contents[register_address+reg_index] = register_data;
						reg_index++;
					}

					break;
				}
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

	void synchronize_memory_map() override {
		//todo read the pneumatic memory map sections
//		//read sensor data registers !
		read_holding_registers_fn(1, P_RUN_REG_START, P_RUN_REG_SIZE);

		read_holding_registers_fn(1, P_RAW_REG_START, P_RAW_REG_SIZE);
//
//		//read system parameter registers !
		read_holding_registers_fn(1, P_PARAM_REG_START, P_PARAM_REG_SIZE) ;

		//Inform the pneumatic controller of the target ON voltage for each of the solenoids
		set_on_voltage(LIGHT_ON_VOLTAGE, flashy_light);
		for (int i = front_fill; i<=mid_fill; i++){
			set_on_voltage(OPEN_VALVE_VOLTAGE, i);
		}



	}

	void send_solenoid_states(){
		write_single_register_fn(connection_config.server_address, SOL_STATES, solenoid_states);
	}

	void set_control_bit(int bit, bool enable){
		if (enable)	solenoid_states |= (1 << bit);
		else solenoid_states &= ~(1 << bit);
	}

	void set_control_byte(uint16_t byte){
		solenoid_states = byte;
	}

	/*
	 * @brief Enable or disable a single solenoid
	 * @param solenoid_num corresponds to the solenoid_names enum values
	 * @param enable, determine whether to open or close the solenoid valve
	 */
	void control_solenoid(int solenoid_num, bool enable){
		if (enable)	solenoid_states |= (1 << solenoid_num);
		else solenoid_states &= ~(1 << solenoid_num);
		//write_single_register_fn(connection_config.server_address, CTRL_REG_START + SOL_STATES_OFFSET, solenoids_state);
	}

	/*
	 * @brief set the desired on voltage for the solenoid valves
	 * @param ON_mV value in mV of the voltage to be used to power the solenoid valves
	 */
	void set_on_voltage(int ON_mV, int i){
		write_single_register_fn(connection_config.server_address, TARG_VOLT_0 + i, ON_mV);
	}
	/*
	 * @brief solenoid state control for increasing the pressure in a specified actuator pair
	 */
	void fill_tank(int tank, bool enable){
		switch(tank){
			case front:
				control_solenoid(front_fill, enable);
				control_solenoid(front_drain, false);

				break;
			case mid:
				control_solenoid(mid_fill, enable);
				control_solenoid(mid_drain, false);

				break;
			case rear:
				control_solenoid(rear_fill, enable);
				control_solenoid(rear_drain, false);

				break;
			case feed: break;
		}
	}
	/*
	 * @brief solenoid state control for decreasing the pressure in a specified actuator pair
	 */
	void drain_tank(int tank, bool enable){
		switch(tank){
			case front:
				control_solenoid(front_fill, false);
				control_solenoid(front_drain, enable);
				break;
			case mid:
				control_solenoid(mid_fill, false);
				control_solenoid(mid_drain, enable);
				break;
			case rear:
				control_solenoid(rear_fill, false);
				control_solenoid(rear_drain, enable);
				break;
			case feed: break;
		}
	}
	/*
	 * @brief solenoid state control for increasing the pressure to all actuator pairs
	 */
	void fill_all_tanks(bool enable){
		solenoid_states |= (1<<front_fill) | (1<<mid_fill)| (1<<rear_fill);
		solenoid_states &= ~((1<<front_drain) | (1<<mid_drain)| (1<<rear_drain));
	}
	/*
	 * @brief solenoid state control for decreasing the pressure to all actuator pairs
	 */
	void drain_all_tanks(bool enable){
		solenoid_states &= ~((1<<front_fill) | (1<<mid_fill)| (1<<rear_fill));
		solenoid_states |= ((1<<front_drain) | (1<<mid_drain)| (1<<rear_drain));
	}

	void flashy_light_on(){
		set_on_voltage(LIGHT_ON_VOLTAGE, flashy_light);
		control_solenoid(flashy_light, true);
	}
	void flashy_light_off(){
		set_on_voltage(LIGHT_ON_VOLTAGE, flashy_light);
		control_solenoid(flashy_light, false);
	}

	void read_status(){
		read_holding_registers_fn(connection_config.server_address, P_RUN_REG_START, 25);
	}

	void set_mode(mode new_mode){
		behaviour_mode = new_mode;
		if (new_mode == minimize_power) {
			min_power_states[0] = hold;
			min_power_states[1] = hold;
			min_power_states[2] = hold;
		}
		if (new_mode == seek_mode) {
			seeking_states[0] = hold;
			seeking_states[1] = hold;
			seeking_states[2] = hold;
		}
	}

	mode get_mode(){
		return behaviour_mode;
	}
	/*
	 * @brief Get the current pressure of a specified tank
	 * @param tank section
	 * @return value in PSI of the pressure in a tank
	 */
	//todo there is a problem when the zero of the adc isn't quite right that we are getting -1 back which should actually be 0
	uint16_t get_tank_pressure_PSI(int tank){
		if (pneumatic_reg_contents[PSI_0 + tank] > 1000) return 0;
		else return pneumatic_reg_contents[PSI_0 + tank];
	}

	/*
	 * @brief todo filter the current tank pressure to be used for maintaining targets
	 */
	uint16_t get_filtered_pressure_PSI(int tank){
		return 1;
	}

	/*
	 * @brief send control to turn power off to all solenoids
	 */
	void power_off_all(){
		solenoid_states &= ~((1<<front_fill) | (1<<mid_fill)| (1<<rear_fill) | (1<<front_drain) | (1<<mid_drain)| (1<<rear_drain));
	}

	uint16_t get_vdd_V(){
		return pneumatic_reg_contents[INPUT_VOLT]/1000.;
	}

	uint16_t get_PWM_duty_values(int solenoid_num){
		return (pneumatic_reg_contents[PWM0 + solenoid_num]*100)/pneumatic_reg_contents[DUTY_DENOM];
	}

	/*
	 * @brief return the byte that describes the on/off state of the solenoids
	 */
	uint16_t get_solenoid_states(){
		return solenoid_states;
	}
	/**
	 * @brief returns true when new data has been received from an actuator since the last time this function was called
	 */
	bool new_data() override {
		bool return_value = new_data_flag;
		new_data_flag = false;
		return return_value;
	}

	/*
	 * @brief enqueue a frame that will read sensor values cycle between the relevant registers, so don't need to do giant read at once
	 */
	void enqueue_read_status(){
		if(modbus_client.get_queue_size() >= 1) return;
		read_status();
	}

	/**
	 * @brief provide access to the modbus client isr function for linking to an interrupt controller
	 */
	void isr() {
		UART.uart_isr();
	}

	/**
	 * @brief returns the number of successful messages
	 */
	uint16_t get_num_successful_msgs() {
		return success_msg_counter;
	}

	/**
	 * @brief return the number of failed messages
	 */
	uint16_t	get_num_failed_msgs() {
		return failed_msg_counter;
	}
    
};

extern PneumaticController pneumatic_controller;

#endif
