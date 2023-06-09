/**
	@file Motor_Control_Panel.h
	@author  Rebecca McWilliam <rmcwilliam@irisdynamics.com>, Michelle Aleman <maleman@irisdynamics.com>
	@version 1.1
	Created on Feb 20, 2022
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


#pragma once

//#include "Arduino.h"
#include "../ic4_library/iriscontrols4.h"
#include "../ic4_library/ic_app.h"
#include "../modbus_client/device_applications/actuator.h"
#include "device_config.h"
#include <string>


class Motor_Control {

	Actuator* motor;
	
	// GUI_Page object to handle hiding/displaying all panel elements
	GUI_Page* ctrl_elements;

	uint16_t anchor_row, anchor_col;
	
public:
    int32_t* force_target;
	int32_t* position_target;
    FlexData target_force, target_position;
    FlexButton sleep_button, position_button, force_button;
	FlexButton enable_button, zero_position, clear_errors;

	void setup(uint16_t anchor_row, uint16_t anchor_col);
    void show();
	void hide();
	void run();
	void add(Actuator* _motor, int32_t* _f_target, int32_t* _p_target, uint16_t _anchor_row, uint16_t _anchor_col);
	void add(GUI_Page* _ctrl_elements, Actuator* _motor, int32_t* _f_target, int32_t* _p_target,  uint16_t _anchor_row, uint16_t _anchor_col);

};