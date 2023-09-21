#pragma once
/**
    @file Comport_Select.h
    @author Kate Colwell <kcolwell@irisdynamics.com>
    @brief Comport select allows users to connect to a new motor on the selected port
    @version 1.1

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

#ifndef _COMPORT_SELECT_H
#define _COMPORT_SELECT_H

#include "ic4_library/io_elements.h"
#include "modbus_client/device_applications/actuator.h"

/**
    @file Comport_Select.h
    @class Comport_Select
    @brief Small panel for displaying comport selection elements
*/
class Comport_Select {

    // GUI_Page object handles hiding/displaying all panel flex elements
    GUI_Page* panel_elements;

    FlexData comport_selector;                          // Data element for displaying the motor's position
   
    FlexButton conn_button;                             //Button for initiating connection
    FlexSlider motor_slider;                            //slider for selecting which motor to interact with/display
    FlexButton disable_btn;                                 // Button for disabling connection to the motor

    bool first_setup = true;                            // Boolean value tracks whether or not the page has been initialized yet

    int last_slider_value;

    int new_motor_port;

    Actuator* motors;
    int motor_id = 0;
    bool see_slider = false;
    int num_motors = 1;

public:

    bool is_running = false;                             // Boolean value tracks whether or not the page is active
    bool new_motor = false;
    int new_port = 0;

    /* Constructor */
    Comport_Select(Actuator * _motors):
        motors(_motors)
    {}


    /** @brief Function initializes all panel flex elements and adds them to a new local GUI_Page */
    void add(Actuator* _motor, uint8_t y_anchor, uint8_t x_anchor, int num_motors = 1) {
        panel_elements = new GUI_Page();
        motors = _motor;
        init(y_anchor, x_anchor, num_motors);
    }


    /** @brief Function initializes all panel flex elements and adds them to the GUI_Page provided */
    void add(GUI_Page* _panel_elements, Actuator* _motor, uint8_t y_anchor, uint8_t x_anchor, int num_motors = 1) {
        panel_elements = _panel_elements;
        motors = _motor;
        init(y_anchor, x_anchor, num_motors);
    }

    void show() {
        panel_elements->show();
    }

    void hide() {
        panel_elements->hide();
    }

    /** @brief Handles initializing and displaying the home page */
    void init(int row_anchor, int column_anchor, int _num_motors) {
        if (first_setup) {
            // Initialize GUI_Page object
            panel_elements->add();
            num_motors = _num_motors;
            //initialize flex data
            comport_selector.add(panel_elements, "Selected Comport: ", row_anchor, column_anchor, 3, 15, 0, 1, FlexData::ALLOW_INPUT+FlexData::FRAME);

            //initialize the flex button
            conn_button.add(panel_elements, "Connect Motor", -1, row_anchor, column_anchor + 20, 2, 6);

            //initiate and add the flex slider
            if (num_motors > 1) {
                
                motor_slider.add(panel_elements, "View Motor:", row_anchor + 3, column_anchor, 2, 30, 0, (num_motors-1),0, 1, FlexSlider::NOT_TRACKING + FlexSlider::ALLOW_INPUT + FlexSlider::UNITS);
                see_slider = true;

            }

            disable_btn.add(panel_elements, "Disable", -1, row_anchor, column_anchor + 26, 2, 4);

            reset_comport_selector();

            first_setup = false;
        }

        //show elements 
        panel_elements->show();
        is_running = true;
    }


    /** @brief Handles updating the home page with motor data */
    void run_gui() {
        if (!is_running) return;
        if (see_slider) motor_id = motor_slider.get();

        if (conn_button.pressed()) {
            new_motor = true;
            new_port = comport_selector.get();
        }
        // Handle disable button if pressed
        if (disable_btn.pressed()) {

            disable_connection();

        }
        //a new comport has been selected
        if (new_motor) {
            //reset flag 
            new_motor = false;

            //copy port info 
          ///  new_motor_port = new_port;

            // com port 1 is reserved on windows so cannot be used as actuator port
            new_motor_connection();
            
        }
        if (see_slider &&(last_slider_value != get_position())) {
            reset_comport_selector();
            last_slider_value = get_position();
        }
    }



    /** @brief handles connection to a new comport - reuses the same actuator object */
    void new_motor_connection() {

        //IC4_virtual->print_l("Into new motor connection\r");

        //create new motor object and add it to the vector
        std::string* _name = new std::string(((std::string)"Motor ").append(String(motor_id)));
        const char* motor_name = (*_name).c_str();

        //get new comport value, pass it to the actuator object and re-init      
        motors[motor_id].set_new_comport(new_port);

        motors[motor_id].init();

        //check to make sure the new actuator creation was successful
        if (motors[motor_id].modbus_client.connection_state() && !(new_port == motors[motor_id].modbus_client.get_port_number() && motors[motor_id].is_connected())) {

            // Print connection success message
            IC4_virtual->print_l("Connecting motor on port ");
            IC4_virtual->print_l((String(new_port).c_str()));
            IC4_virtual->print_l("\r");

            // enable connected motor
            motors[motor_id].enable();
            conn_button.disable(1);
        }
        else {
            //motor serial port connection fail
            // update com port value to 0 or previous value
            //reset the selected element 
            reset_comport_selector();
            //display an error to the user and delete motor
            const char* error_msg = "Unable to open a connection to a motor on port ";
            IC4_virtual->print_l(error_msg);
            IC4_virtual->print_l((String(new_motor_port).c_str()));
            IC4_virtual->print_l("\r");
        }

        //IC4_virtual->print_l("new motor connection over\r");

    }

    void disable_connection() {
        motor_id = get_position();

        if (motors[motor_id].modbus_client.connection_state()) {

            // Print delete message
            IC4_virtual->print_l("Disabling motor on port ");
            IC4_virtual->print_l(String(motors[motor_id].modbus_client.get_port_number()).c_str());
            IC4_virtual->print_l("\r");

            //disable comms for this actuator object
            motors[motor_id].disable();
            motors[motor_id].disable_comport();
            conn_button.disable(0);
        }
    }


    // Return current slider value
     /** @brief Return the current slider value */
    int get_position() {
        if (num_motors>1) return motor_slider.get();
        return 0;
    }



    void reset_comport_selector() {
        comport_selector.update(motors[motor_id].modbus_client.get_port_number());
    }

    void update_selctor_data(int comport) {
        comport_selector.update(comport);
    }


    /** @brief Resets all home page elements in GUI view */
    void reset() {
        if (is_running) {
            hide();
        }
        first_setup = true;
    }
};
#endif
