#pragma once
/**
    @file Main_GUI.h
    @author Rebecca McWilliam <rmcwilliam@irisdynamics.com>, Michelle Aleman <maleman@irisdynamics.com>
    @brief Main application that handles IrisControls connection and gui interaction
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

#ifndef MAIN_GUI_H_
#define MAIN_GUI_H_

#include <ic4_library/iriscontrols4.h>
#include <ic4_library/device_drivers/windows/ic4_windows.h>
#include "modbus_client/device_applications/actuator.h"
#include "Home_Page.h"                                  // Custom page displaying a motor's position data
#include "Position_Control_Page.h"                          // Position Control Page: an interface for updating Orca motor PID gains and target position using a generated signal
#include "irisSDK_libraries/Parse_Args.h" // Provides functions for parsing the args list and checking for arguments that it contains

/**
    @file Main_GUI.h
    @class GUI
    @brief Extension of the Windows virtual object for the IC4 library, handles connectivity with IrisControls4 and gui element interaction
*/
class GUI : public IC4_windows {

    Actuator* motor;                                    // Reference to an actuator object that will be passed in when this object is initialized
    uint32_t gui_timer = 0;                             // This will keep track of the last update
    uint8_t gui_update_period = 20;                     // This is the time between updates in milliseconds (100 ms = 10 fps)
    Home_Page home_page;                                // Define Home_Page object
    FlexButton home_page_btn, pos_ctrl_page_btn;                          // Define FlexButton element for hiding/displaying the home page

public:
    Position_Control_Page position_control_page;        // Define Position_Control_Page object

    int32_t target_force = 0;                           // Define a target force variable. Initialize to 0, it will be updated if the user send the force serial command "f"


     /* Constructor */
    GUI(
        /** Parameter for constructing the GUI object is a reference to a motor. This will allow this object to have access to control and feedback from the motor.*/
        Actuator* _motor,
        Signal_Generator& _signal_generator
    ) :
        /* Initialization list */
        motor(_motor),
        home_page(_motor),
        position_control_page(motor[0], _signal_generator)
    {
        /*  Server name, changes the name in the initial connection message with IrisControls.
            Device ID, changes how the device will show up in the com port drop down*/
        set_server_name("Tutorial 5");
        set_device_id("Windows Tutorial 5");
    }


    /**
        @brief Handles connectivity and gui interaction with IrisControls
    */
    void run() {
        check();                                        // Parses incoming serial communications (IrisControls -> Windows Device)
        switch (gui_frame_state) {
        case rx: {
            // If connection times out, set disconnected
            if (is_timed_out()) {
                set_disconnected();
                reset_all();
            }
            break;
        }//RX CASE
        case tx: {
            if (new_connection()) {                 // IRIS CONTROLS HAS ESTABLISHED CONNECTION
                setup();
            }

            if (is_connected()) {              // IRIS CONTROLS REGULAR UPDATES
                // When IrisControls is connected only update the gui at a specified gui period (in milliseconds)
                // This dictates the frame rate of the GUI, if the gui period is too large the gui might be choppy
                // If the period is too small there will be an unneeded amount of serial traffic and could cause lag
                if ((uint32_t)(millis() - gui_timer) > gui_update_period) {
                    gui_timer = millis();

                    // Update GUI
                    frame_update();

                    // Transmit end of transmission message
                    end_of_frame();
                }
            }
            send();                             // Sends anything in the transmit buffer (Windows Device -> IrisControls)
            break;
        }//TX CASE
        }//SWITCH
    }

private:

    /**
        @brief GUI initalization called from check for new IrisControls connection
    */
    void setup() {
        gui_set_grid(40, 60);                           // This will set the size of the IrisControls window,
        print_l("New Connection Message\r");            // Message is printed when IrisControls establishes a connection with the Windows device
        home_page_btn.add("Home", -1, 26, 1, 2, 4);      // Initializes position page button
        pos_ctrl_page_btn.add("Position", -1, 26, 6, 2, 4);

        home_page_btn.disable(true);
        reset_all();
        home_page.setup();                            // Initialize home page (landing page)
    }

    /**
        @brief action to be called every gui frame go here
        This is called inside the is connected gui update loop
    */
    void frame_update() {
        //check if home page is toggled on
        if (home_page_btn.pressed()) {
            position_control_page.hide();
            home_page.setup();
            home_page_btn.disable(true);
            pos_ctrl_page_btn.disable(false);
        }
        if (pos_ctrl_page_btn.pressed()) {
            home_page.hide();
            position_control_page.setup();
            home_page_btn.disable(false);
            pos_ctrl_page_btn.disable(true);
        }
        //call run on gui panels
        home_page.run();
        position_control_page.run();
    }

    /** @brief Hide all gui elements */
    void hide_all() {
        home_page.hide();
    }

    /** @brief Reset all gui elements */
    void reset_all() {
        home_page.reset();
    }

    /** @brief Parsing of serial messages at application layer */
    int parse_app(char* cmd, char* args) {

        START_PARSING

            /*
                Command handling format: COMMAND_IS "<command name>" THEN_DO <code to handle command>
                Note: all arguments following the command name are stored in the char pointer "args",
                with a white space char before each argument.
            */

            // Command "hello" prints "world" to IC4 console.
            COMMAND_IS "hello" THEN_DO
            print_l("world\r");                              // Print "world" to console

            // Command prints the values of the motors temperature, position, force, power , and voltage.
        COMMAND_IS "get_data" THEN_DO
            if (motor->is_connected()) {                        //Check to see if the motor is connected
                print_l("\rTemperature (*deg*C): ");
                print_d(motor->get_temperature_C());             // Print temperature from Actuator object
                print_l("\rPosition (*mu*m): ");
                print_d(motor->get_position_um());               // Print position from Actuator object
                print_l("\rForce (mN): ");
                print_d(motor->get_force_mN());                  // Print force from Actuator object
                print_l("\rPower (W): ");
                print_d(motor->get_power_W());                   // Print power from Actuator object
                print_l("\rVoltage (mV): ");
                print_d(motor->get_voltage_mV());                // Print voltage from Actuator object
            }
            else {
                print_l("\rPlease connect a motor to read data.");
            }

        // Command message of "max_temp 50" will give the motor a maximum allowable temperature of 50 degrees Celsius (by updating value in register 139)
        COMMAND_IS "max_temp" THEN_DO
            unsigned int arg_index = 0;
        if (motor->is_connected()) {
            uint16_t max_temp = parse_int(args, arg_index); // Get max temperature value from argument list
            print_l("\rSetting max temp: ");
            print_d(max_temp);
            motor->set_max_temp(max_temp);
        }
        else {
            print_l("\rPlease connect a motor to set the max temp.");
        }


        // Command message of "f 1000" will give a target force of 1000 to the motor.
        COMMAND_IS "f" THEN_DO
            unsigned int arg_index = 0;
        if (motor->is_connected()) {
            target_force = parse_int(args, arg_index);
            print_l("\rTarget force: ");
            print_d(target_force);
            motor->set_mode(Actuator::ForceMode);
        }
        else {
            print_l("\rPlease connect a motor to set the target force.");
        }


        COMMAND_IS "error" THEN_DO
            // Print descriptions of error codes
            IC4_virtual->print_l("Error Flags:\r1-configuration invalid\r32-force control clipping\r64-max temp exceeded\r128-max force exceeded\r256-max power exceeded\r512-low shaft quality\r1024-voltage invalid\r2048-comms timeout");
        IC4_virtual->print_l("\r\r");
        //if a motor is connected, print any active errors
        if (motor->is_connected()) {
            IC4_virtual->print_l(motor->get_name());
            IC4_virtual->print_l("\rActive Errors: ");
            uint16_t active_sum = motor->get_errors();
            uint16_t error_code_list[] = { 2048, 1024, 512, 256, 128, 64, 32, 1 };
            int error_index = 0;
            // Compare active error sum to each error code in list
            while (error_index < 8) {
                uint16_t error_code = error_code_list[error_index];
                if (active_sum >= error_code) {
                    // If active error sum contains error code, print error code and subtract it from the active error sum
                    IC4_virtual->print_d(error_code);
                    IC4_virtual->print_l(", ");
                    active_sum -= error_code;
                }
                error_index += 1;
            }
            IC4_virtual->print_l("\r\r");
        }


        FINISH_PARSING
    }
};

#endif