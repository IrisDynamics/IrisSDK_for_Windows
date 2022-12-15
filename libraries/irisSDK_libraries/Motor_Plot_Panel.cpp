/**
    @file Motor_Plot_Panel.cpp
    @brief Orca motor plot interface displaying the current position, force, voltage, temperature, power, errors, and fps.
    @author  Dan Beddoes <dbeddoes@irisdynamics.com>, Rebecca McWilliam <rmcwilliam@irisdynamics.com>, Michelle Aleman <maleman@irisdynamics.com>
    @version 1.1
    Created on July 21, 2021

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

#include "Motor_Plot_Panel.h"
#include "device_config.h"
#include <string.h>

#define FORCE_ERROR_MASK    (1<<6)
#define TEMP_ERROR_MASK     (1<<7)
#define POWER_ERROR_MASK    (1<<8)


/**
    @brief function initializes a new GUI_Page object to handle hiding/displaying Motor_Plot elements before calling setup() to initialize all plot elements.
*/
void Motor_Plot::add(Actuator* _motor, const  char* _name, uint16_t _anchor_row, uint16_t _anchor_col, uint16_t _plot_height, uint16_t _plot_width) {
    plot_elements = new GUI_Page(); // Init new GUI_Page object instance
    motor = _motor; // Init Actuator object
    setup(_name, _anchor_row, _anchor_col, _plot_height, _plot_width); // Init all Motor_Plot flex elements
}


/**
    @brief function is the same as above but takes a GUI_Page pointer as an argument instead of initializing a new one before calling setup() to initialize all plot elements.
*/
void Motor_Plot::add(GUI_Page* _plot_elements, Actuator* _motor, const  char* _name, uint16_t _anchor_row, uint16_t _anchor_col, uint16_t _plot_height, uint16_t _plot_width) {
    plot_elements = _plot_elements; // Init GUI_Page object
    motor = _motor; // Init Actuator object
    setup(_name, _anchor_row, _anchor_col, _plot_height, _plot_width); // Init all Motor_Plot flex elements
}


/**
    @brief Setup the gui elements and datasets.
*/
void Motor_Plot::setup(const  char* name, uint16_t anchor_row, uint16_t anchor_col, uint16_t plot_height, uint16_t plot_width) {

    // Init GUI_Page object
    plot_elements->add();

    //flexelements
    label.add(plot_elements, name, anchor_row, anchor_col, 2, plot_width, FlexLabel::ALIGN_CENTER);

    plot.add(plot_elements, "", anchor_row + 2, anchor_col, plot_height, plot_width, -10, 310, FlexPlot::TIMEPLOT + FlexPlot::WALKING);

    volt.add(plot_elements, "", anchor_row + plot_height + 2, anchor_col + 0, 1, 4, -1, 1000, "V", FlexData::UNITS + FlexData::DIGITS_4 + FlexData::PRECISION_1);
    temp.add(plot_elements, "", anchor_row + plot_height + 2, anchor_col + plot_width / 5, 1, 4, -1, 1, "*degC*", FlexData::UNITS + FlexData::DIGITS_3);
    power.add(plot_elements, "", anchor_row + plot_height + 2, anchor_col + (2 * plot_width) / 5, 1, 4, -1, 1, "W", FlexData::UNITS + FlexData::DIGITS_4);
    errors.add(plot_elements, "", anchor_row + plot_height + 2, anchor_col + (3 * plot_width) / 5, 1, 4, -1, 1, "E", FlexData::UNITS + FlexData::DIGITS_4);
    fps.add(plot_elements, "", anchor_row + plot_height + 2, anchor_col + (4 * plot_width) / 5, 1, 5, -1, 1, "Fps", FlexData::UNITS + FlexData::DIGITS_4);
    output.add(plot_elements, " Force", anchor_row + plot_height + 3, anchor_col, 1, plot_width, -32000, 32000, 0, 1);

    //output element colours
    volt.set_colour(FlexData::VALUE_TEXT, OFF_WHITE);
    temp.set_colour(FlexData::VALUE_TEXT, OFF_WHITE);
    power.set_colour(FlexData::VALUE_TEXT, OFF_WHITE);
    errors.set_colour(FlexData::VALUE_TEXT, OFF_WHITE);
    fps.set_colour(FlexData::VALUE_TEXT, OFF_WHITE);

    label.set_colour(FlexLabel::TEXT, 169, 169, 169, 169);
    output.set_colour(FlexSlider::VALUE_TEXT, OFF_WHITE);
    output.set_colour(FlexSlider::SLIDER_SUBPAGE, TRANS_DARK_GREY);
    output.set_colour(FlexSlider::SLIDER_ADDPAGE, TRANS_DARK_GREY);
    output.set_colour(FlexSlider::HANDLE_DISABLED, RED);

    IC4_virtual->flush();

    //datasets
    temp_data.add(&plot, "Temp Data", "", "Temperature (C)", Dataset::TIMEPLOT + Dataset::NONE);
    volt_data.add(&plot, "Volt Data", "", "Voltage (mV)", Dataset::TIMEPLOT + Dataset::NONE);
    power_data.add(&plot, "Power Data", "", "Power (W)", Dataset::TIMEPLOT + Dataset::NONE + Dataset::SECONDARY_Y_AXIS);
    position_data.add(&plot, "Position Data", "", "Position(*mu*m)", Dataset::TIMEPLOT + Dataset::NONE);
    output_data.add(&plot, "Force Data", "", "Force (N)", Dataset::TIMEPLOT + Dataset::NONE);

    temp_data.set_max_data_points(25000);
    temp_data.set_max_data_points(25000);
    volt_data.set_max_data_points(25000);
    power_data.set_max_data_points(25000);
    position_data.set_max_data_points(25000);
    output_data.set_max_data_points(25000);

    temp_data.set_colour(ORANGE);
    volt_data.set_colour(ORANGE);
    power_data.set_colour(RED);
    position_data.set_colour(WHITE);
    output_data.set_colour(ORANGE);
    position_data.show();
    power_data.show();

    //plot config
    plot.set_axes_labels(&power_data);
    plot.set_axes_labels(&position_data);
    plot.set_domain(10);
    plot.set_range(0, 300000);
    plot.set_secondary_range(0, 1250);
    plot.set_colour(FlexPlot::COLOUR_SET_FLAG::TEXT, 169, 169, 169, 169);

    label.font_size(18);
    last_data_rate_check = millis();

}

/** @brief Function hides all panel elements */
void Motor_Plot::hide() {
    plot_elements->hide();
}


/** @brief Function hides all panel elements */
void Motor_Plot::show() {
    plot_elements->show();
}


/** @brief Function updates flex plot with data from Actuator object */
void Motor_Plot::run() {
    //Update outputs
    update_fps();
    temp.update(motor->get_temperature_C());
    volt.update(motor->get_voltage_mV());
    power.update(motor->get_power_W());
    errors.update(motor->get_errors());

    output.update(motor->get_force_mN());

    //Update Datasets
    u64 now = IC4_virtual->system_time();

    temp_data.add_data(now, temp.get());
    volt_data.add_data(now, volt.get());
    power_data.add_data(now, power.get());
    position_data.add_data(now, (int)(motor->get_position_um()));
    output_data.add_data(now, output.get());

    handle_error_codes();
    const char* filename;
    std::string string_pointer;

    switch (log_state) {
    case start:
        string_pointer = String(motor->get_serial_number()).concat("_Memory_Map");
        filename = string_pointer.c_str();
        logfile.add(filename);
        log_state = reading;
        break;

    case reading:
        logfile.write("Index\tValue");
        motor->read_holding_registers_fn(1, 0, ORCA_REG_SIZE);
        log_state = writing;
        break;
    case writing:
        for (int i = 0; i < ORCA_REG_SIZE; i++) {
            //write_string = (String(i).concat("\t").concat(String(motor->get_orca_reg_content(i))));
            //write_string = String(32);
            logfile.write((String(i).concat("\t").concat(String(motor->get_orca_reg_content(i)))).c_str());
            IC4_virtual->flush();
        }

        log_state = idle;
        logfile.write("===========================");
        break;
    case idle:
        break;
    }
}


/**
    @brief Method that changes the plot title depending on the actuator's state
    Sets the plot title colour to red with any error present, green with no errors present, light grey if disconnected
*/
void Motor_Plot::handle_error_codes() {

    current_errors = -1;

    uint16_t errors = motor->get_errors();

    if (current_errors == errors) return;

    if (motor->is_connected()) {
        if (errors)     label.set_colour(FlexLabel::TEXT, RED);
        else            label.set_colour(FlexLabel::TEXT, GREEN);
    }
    else {
        label.set_colour(FlexLabel::TEXT, 169, 169, 169, 169);
    }

    current_errors = errors;
}


/**
    @brief Updates the data rate FlexDatas on the second row of the ribbon
*/
void Motor_Plot::update_fps() {

    uint32_t tnow = millis();

    uint32_t total_success_rate = 0;

    if (tnow - last_data_rate_check >= 100) {

        total_success_rate = 0;

        uint16_t num_msgs = motor->get_num_successful_msgs();
        total_success_rate = (uint16_t)(num_msgs - last_num_success_msgs) * 1000 / (uint32_t)(tnow - last_data_rate_check);
        last_num_success_msgs = num_msgs;

        fps.update((int)total_success_rate);
        total_success_rate = 0;
        last_data_rate_check = tnow;
    }
}