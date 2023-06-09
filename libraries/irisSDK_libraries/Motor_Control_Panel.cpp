/** 
	@file Motor_Control_Panel.cpp
	@author  Rebecca McWilliam <rmcwilliam@irisdynamics.com>, Michelle Aleman <maleman@irisdynamics.com>
	@version 1.1
	Created on July 21, 2021
	@brief Basic sliders to set target positions or forces and switch between modes (Sleep, Position, Force)
   
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

#include "Motor_Control_Panel.h"


/**
	@brief function initializes a new GUI_Page object to handle hiding/displaying Motor_Plot elements before calling setup() to initialize all plot elements.
*/
void Motor_Control::add(Actuator* _motor, int32_t* _f_target, int32_t* _p_target, uint16_t _anchor_row, uint16_t _anchor_col) {
	ctrl_elements = new GUI_Page(); // Init new GUI_Page object
	motor = _motor; // Init Actuator object
	force_target = _f_target;
	position_target = _p_target;
	setup(_anchor_row, _anchor_col); // Initialize Motor_control flex elements
}


/**
	@brief function is the same as above but takes a GUI_Page pointer as an argument instead of initializing a new one before calling setup() to initialize all plot elements.
*/
void Motor_Control::add(GUI_Page* _ctrl_elements, Actuator* _motor, int32_t* _f_target, int32_t* _p_target, uint16_t _anchor_row, uint16_t _anchor_col) {
	ctrl_elements = _ctrl_elements; // Init GUI_Page object
	motor = _motor; // Init Actuator object
	force_target = _f_target;
	position_target = _p_target;
	setup(_anchor_row, _anchor_col); // Initialize Motor_control flex elements
}


/**
	@brief Initializes all flex elements in Motor_Control panel
*/
void Motor_Control::setup(uint16_t anchor_row, uint16_t anchor_col) {
	// Init GUI_Page object
	ctrl_elements->add();
	//buttons
	enable_button.add(ctrl_elements,"Enable", 1, anchor_row, anchor_col, 2, 4);
	zero_position.add(ctrl_elements, "Zero Pos", -1, anchor_row, anchor_col + 5, 2, 4);
	clear_errors.add(ctrl_elements, "Clear Errors", -1, anchor_row, anchor_col + 10, 2, 4);
	sleep_button.add(ctrl_elements, "Sleep", -1, anchor_row + 3, anchor_col, 2, 4);
	position_button.add(ctrl_elements, "Position", -1, anchor_row + 3, anchor_col + 5, 2, 4);
	force_button.add(ctrl_elements, "Force", -1, anchor_row + 3, anchor_col + 10, 2, 4);
	//FlexDatas
	target_force.add(ctrl_elements, "Force", anchor_row + 6, anchor_col, 1, 16, 0, 1, "mN", FlexSlider::ALLOW_INPUT + FlexData::UNITS);
	target_position.add(ctrl_elements, "Position", anchor_row + 8, anchor_col, 1, 16, 0, 1000, "mm", FlexSlider::ALLOW_INPUT + FlexData::UNITS);
	show();
}


/** @brief Displays the motor control panel. */
void Motor_Control::show() {
	ctrl_elements->show();
	sleep_button.disable(true);
	position_button.disable(false);
	force_button.disable(false);
	target_force.disable(true);
	target_position.disable(true);
	motor->set_mode(Actuator::SleepMode);
}


/** @brief Hides the motor control panel. */
void Motor_Control::hide() {
	ctrl_elements->hide();
	sleep_button.disable(true);
	position_button.disable(false);
	force_button.disable(false);
	target_force.disable(true);
	target_position.disable(true);
}


/**
	@brief Updates motor's position/force if slider value is updated. Actions buttons that are clicked in panel, ignoring position and force button clicks if the motor has active errors.
*/
void Motor_Control::run() {

	// Handle toggling on of buttons `zero position`, `clear errors`, and `get latched errors`
	if (zero_position.pressed()) {
		motor->zero_position();
		target_position.update(0);
	}
	if (clear_errors.pressed()) {
		motor->clear_errors();
	}

	if (enable_button.toggled()) {
		if (enable_button.get()) {
			motor->enable();
		}
		else {
			motor->disable();
		}
	}


	if (sleep_button.pressed()) {
		IC4_virtual->print_l(motor->get_name());
		IC4_virtual->print_l("\tSleep Mode\r");
		sleep_button.disable(true);
		position_button.disable(false);
		force_button.disable(false);
		target_force.disable(true);
		target_position.disable(true);
		motor->set_mode(Actuator::SleepMode);
	}

	if (position_button.pressed()) {
		if (!motor->get_errors() && motor->is_connected()) {
			IC4_virtual->print_l(motor->get_name());
			IC4_virtual->print_l("\tPosition Mode\r");
			sleep_button.disable(false);
			position_button.disable(true);
			force_button.disable(false);
			motor->set_mode(Actuator::PositionMode);
			target_force.disable(true);
			target_position.disable(false);
			target_position.update(motor->get_position_um());
		}
		else {
			IC4_virtual->print_l("\rErrors must be clear and motor connected to enter position mode\r");
		}
	}

	if (force_button.pressed()) {
		if (!motor->get_errors() && motor->is_connected()) {
			IC4_virtual->print_l(motor->get_name());
			IC4_virtual->print_l(" Force Mode\r");
			sleep_button.disable(false);
			position_button.disable(false);
			force_button.disable(true);
			target_force.disable(false);
			target_position.disable(true);
			motor->set_mode(Actuator::ForceMode);
			target_force.update(0);
		}
		else {
			IC4_virtual->print_l("\rErrors must be clear and motor connected to enter force mode\r");
		}
	}

	if (!motor->is_connected()) {
		sleep_button.disable(true);
		position_button.disable(false);
		force_button.disable(false);
		target_force.disable(true);
		target_position.disable(true);
		motor->set_mode(Actuator::SleepMode);
	}

	*force_target = target_force.get();
	*position_target = target_position.get();
}