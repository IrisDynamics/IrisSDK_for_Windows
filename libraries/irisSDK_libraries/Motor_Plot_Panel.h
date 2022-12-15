
/**
   @file Motor_Plot_Panel.h
   @author  Dan Beddoes <dbeddoes@irisdynamics.com>, Michelle Aleman <maleman@irisdynamics.com>
   @brief Orca motor plot interface displaying the current position, force, voltage, temperature, power, errors, and fps.
 * @version 1.1
 *
 * Created on July 21, 2021
   
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
#include "../ic4_library/ic_app.h" // IrisControls4 device driver for the k20 linked against Arduino 1.8.13 and Teensyduio 1.53
#include "../modbus_client/device_applications/actuator.h" //actuator objects (abstracts communication to motor)
#include "../ic4_library/log.h"
#include "device_config.h"
#include <string>


class Motor_Plot {

    // Actuator object for updating plot data
    Actuator * motor;

    // GUI_Page object to handle hiding/displaying all panel elements
    GUI_Page * plot_elements;

	DataLog logfile;
public:
	int log_state = idle;
	enum {
		start,
		reading,
		writing,
		idle
	};

	FlexLabel label;

	FlexPlot plot;

	FlexData temp;
	FlexData volt;
	FlexData power;
	FlexData errors;
	FlexData fps;

	FlexSlider output;

	Dataset temp_data;
	Dataset volt_data;
	Dataset power_data;
	Dataset position_data;
	Dataset output_data;


    
    void add(Actuator * _motor, const  char * _name, uint16_t _anchor_row, uint16_t _anchor_col, uint16_t _plot_height=12, uint16_t _plot_width=22);
    void add(GUI_Page * _plot_elements, Actuator * _motor, const  char * _name, uint16_t _anchor_row, uint16_t _anchor_col, uint16_t _plot_height=12, uint16_t _plot_width=22);
    void setup(const  char * name, uint16_t anchor_row, uint16_t anchor_col, uint16_t plot_height, uint16_t plot_width);
    void hide();
    void show();
    void run();
    void update_fps();
    void handle_error_codes();
	void start_log() {
		log_state = start;
	}

protected:

    uint32_t prev_status = -1;
    uint32_t prev_errors = -1;
    uint32_t last_data_rate_check = 0;

    // Used to track the motor data rates
    uint16_t last_num_success_msgs; 
 
    enum PARAMETER_STATUS{
        OK      = 0,
        CAUTION = 1,
        WARNING = 2,
    };

private:
    int current_errors = -1;
};