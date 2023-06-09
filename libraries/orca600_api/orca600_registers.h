/**
 * @file orca600_registers.h
 *
 *  Created on: Jan. 24, 2022
 *      Author: sjeffery
 */

#ifndef ORCA600_REGISTERS_H_
#define ORCA600_REGISTERS_H_

#include "types.h"


/**
 * @class CONTROL_REG
 * @brief Base class for a control register.
 *
 * Only defines virtual versions of read and write functions.
 * If the helper functions for control registers (i.e. set_flag) are to be used, these functions must be implemented.
 */
class CONTROL_REG {
public:
	virtual void write(u16 address, u16 value);

	virtual u16 read(u16 address);
};


 /**
  *  @class CONTROL_REG_0
  *  @brief Control Register 0.
  */
class CONTROL_REG_0 : public CONTROL_REG {
public:

	static const int address = CTRL_REG_0;

	typedef enum {
		reset_flag						= 1 << 0,
		clear_errors_flag				= 1 << 1,
		position_zero_flag				= 1 << 2
	} control_register_0_t;

};


/**
 *  @class CONTROL_REG_1
 *  @brief Control Register 1 contains flags for starting calibration routines.
 */
class CONTROL_REG_1 : public CONTROL_REG {
public:

	static const int address = CTRL_REG_1;

	typedef enum {
		hall_gain_set_flag					= 1 << 3,
		current_gain_set_flag				= 1 << 4,
		hall_zero_flag						= 1 << 5,
		current_zero_flag					= 1 << 6,
//		stator_calibration_flag				= 1 << 7,
//		shaft_calibration_flag				= 1 << 8,
//		force_calibration_flag				= 1 << 9,
		position_controller_gain_set_flag	= 1 << 10,
		current_controller_gain_set_flag	= 1 << 11,
		change_force_units_flag				= 1 << 12
	} control_register_1_t;

};


/**
 *  @class CONTROL_REG_2
 *  @brief Control Register 2 contains flags for saving data to nonvolatile memory.
 */
class CONTROL_REG_2 : public CONTROL_REG {
public:

	static const int address = CTRL_REG_2;

	typedef enum {
		parameter_save_flag				= 1 << 0,
		stator_calibration_save_flag 	= 1 << 2,
		shaft_calibration_save_flag 	= 1 << 3,
		force_calibration_save_flag 	= 1 << 4,
		tuning_save_flag 				= 1 << 5,
		usr_opt_save_flag 				= 1 << 6,
		motion_config_save_flag 		= 1 << 7,
		current_loop_save_flag 			= 1 << 8,
		haptic_config_save_flag			= 1 << 9
	} control_register_2_t;

};


/**
 *  @class CONTROL_REG_3
 *  @brief Control Register 3 controls the transitions between Orca modes.
 */
class CONTROL_REG_3 : public CONTROL_REG {
public:

	static const int address  = CTRL_REG_3;
	static const int mode_reg = MODE_OF_OPERATION;

	typedef enum {
		sleep_sid				= 1,
		force_control_sid		= 2,
		position_control_sid	= 3,
		haptic_control_sid		= 4,
		kinematic_control_sid	= 5,
		voltage_control_sid		= 6,
		current_control_sid		= 7,
		iloop_force_sid			= 8,
		iloop_pos_sid			= 9,
		iloop_kinematic_sid		= 10,
		stator_calibration_sid	= 50,
		force_calibration_sid	= 51,
		shaft_calibration_sid	= 52,
		adc_calibration_sid		= 53,
		current_tuning_sid		= 54
	} control_register_3_t;

	/**
	 * @brief Write a state id to control register 3, commanding the Orca to change to the selected mode.
	 */
	void set_mode (control_register_3_t sid) {
		write(address, sid);
	}

	/**
	 * @brief Return the value stored in control register 3, i.e. what mode the Orca is in.
	 */
	u16 get_mode() {
		return read(mode_reg);
	}
};

/**
 *  @class CONTROL_REG_4
 *  @brief Control Register 4 resets configurations to their default factory value
 */
class CONTROL_REG_4 : public CONTROL_REG {
public:

	static const int address  = CTRL_REG_4;

	typedef enum {
		parameter_default_flag 				= 1 << 0,
		tuning_default_flag			 		= 1 << 1,
		motor_user_options_default_flag		= 1 << 2,
		modbus_user_options_default_flag	= 1 << 3,
		kinematic_default_flag				= 1 << 4,
		haptic_default_flag					= 1 << 5,
		analog_interface_default_flag		= 1 << 6
	} control_register_4_t;


};

/**
 * @class ERROR_REG_0
 * @brief Error Register 0 contains the Orcas error flags.
 */
class ERROR_REG_0 {
public:

	static const int address_active_errors		= ERROR_0;
	static const int address_latched_errors		= ERROR_1;

	typedef enum {
		configuration_invalid		= 1 << 0,
		force_control_clipping		= 1 << 5,
		max_temp_exceeded			= 1 << 6,
		max_force_exceeded			= 1 << 7,
		max_power_exceeded			= 1 << 8,
		shaft_image_failed			= 1 << 9,
		voltage_invalid				= 1 << 10,
		comms_timeout				= 1 << 11,
		iloop_input_range			= 1 << 12
	} orca_error_t;


	static const int configuration_errors		= configuration_invalid;

	/* When a calibration error is active, PWM output is disabled. */
	static const int calibration_error_mask		= max_temp_exceeded | voltage_invalid;

	/* Force errors cause the force controller to stop producing outputs. */
	static const int force_error_mask			= configuration_errors | max_temp_exceeded | max_power_exceeded | shaft_image_failed | voltage_invalid;

	/* Position errors cause the position controller to stop producing outputs. */
	static const int position_error_mask		= configuration_errors | max_temp_exceeded | max_power_exceeded | shaft_image_failed | voltage_invalid;

};


#endif /* ORCA600_REGISTERS_H_ */
