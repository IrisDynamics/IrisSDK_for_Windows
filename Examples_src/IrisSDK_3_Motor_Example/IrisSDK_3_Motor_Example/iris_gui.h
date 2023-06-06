/**
 * @file windows_gui.h
 * @author Kate Colwell <kcolwell@irisdynamics.com>
 *
 * @brief  GUI displyaing multiple actuator object with options for configuring comports
 *
 * @version 1.1

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

#include "ic4_library/iriscontrols4.h"
#include "ic4_library/device_drivers/windows/IC4_windows.h"
#include "windows.h"
#include "modbus_client/device_applications/actuator.h"
 //#include "Comport_Select.h"
#include <vector>
#include "irisSDK_libraries/device_config.h"
#include "irisSDK_libraries/Motor_Plot_Panel.h"
#include "irisSDK_libraries/Motor_Control_Panel.h"
#include <memory>

#define NUM_MOTORS 3


class GUI : public IC4_windows {
public:
    uint16_t last_GUI_update;
    Actuator* motors;

public:
    uint32_t tnow = millis();
    int32_t* force_target;
    int32_t* position_target;
    Motor_Plot motor_plot[NUM_MOTORS];           //from gui_panels library
    Motor_Control motor_control[NUM_MOTORS];
    FlexData comport_selector[NUM_MOTORS];
    FlexButton connect_btn[NUM_MOTORS];                                 // Button for disabling connection to the motor
    //GUI stuff 
    uint32_t gui_timer = 0;                                 // This will keep track of the last update
    uint8_t gui_update_period = 10;                         // This is the time between updates in milliseconds (100 ms = 10 fps)


    uint8_t motor_id = 0;                                   // Value used to determine when the slider has been updated
    s32 motor_counter = 0;


    GUI(Actuator* _motors
        , int32_t* _force_target
        , int32_t* _position_target
    ) :
        motors(_motors)
        , force_target(_force_target)
        , position_target(_position_target)
    {
        set_server_name("Three Motor Example v 2.2.5");
        set_device_id("windows");
    }

    /** @brief Regularily called to update the GUI */
    void run() {
        LPCWSTR paramErr;
        check();                                        // Parses incoming serial communications (IrisControls -> Eagle)
        switch (gui_frame_state) {
        case rx:
            break;

        case tx:
            if (new_connection()) {                     // IRIS CONTROLS HAS ESTABLISHED CONNECTION
                setup();
            }
            if (is_connected()) {                       // IRIS CONTROLS REGULAR UPDATES
                // When IrisControls is connected only update the gui at a specified gui period (in milliseconds)
                // This dictates the frame rate of the GUI, if the gui period is too large the gui might be choppy
                // If the period is too small there will be an unneeded amount of serial traffic and could cause lag
                uint32_t millis = IC4_virtual->system_time() / 1000;
                if ((uint32_t)(millis - gui_timer) > gui_update_period) {
                    gui_timer = millis;

                    // Update GUI
                    frame_update();

                    // Transmit end of transmission message
                    end_of_frame();
                }
            }

            send();     // Sends anything in the transmit buffer (Windows Device -> IrisControls)
            break;
        }
    }

    /** @brief GUI initalization called from check for new IrisControls connection */
    void setup() {

        gui_set_grid(50, 60);                         // This will set the size of the IrisControls window

        // Message is printed when IrisControls establishes a connection with the device program
        print_l("Please select the comport of your USB to RS422 connection.\r");

        motor_control[0].add(&motors[0], &force_target[0], &position_target[0], 7, 43);
        motor_control[1].add(&motors[1], &force_target[1], &position_target[1], 22, 43);
        motor_control[2].add(&motors[2], &force_target[2], &position_target[2], 37, 43);

        motor_plot[0].add(&motors[0], "Select Port", 3, 19, 10, 20);
        motor_plot[1].add(&motors[1], "Select Port", 18, 19, 10, 20);
        motor_plot[2].add(&motors[2], "Select Port", 34, 19, 10, 20);

        comport_selector[0].add("COM: ", 4, 43, 1, 7, 0, 1, FlexData::ALLOW_INPUT);
        comport_selector[1].add("COM: ", 19, 43, 1, 7, 0, 1, FlexData::ALLOW_INPUT);
        comport_selector[2].add("COM: ", 34, 43, 1, 7, 0, 1, FlexData::ALLOW_INPUT);
        connect_btn[0].add("Connect", 0, 4, 51, 2, 4);
        connect_btn[1].add("Connect", 0, 19, 51, 2, 4);
        connect_btn[2].add("Connect", 0, 34, 51, 2, 4);
    }


    /** @brief Actions to be called every GUI frame go here. This is called each gui_update_period once an IC4 connection is established */
    void frame_update() {

        for (int i = 0; i < NUM_MOTORS; i++) {

            motor_plot[i].run();
            motor_control[i].run();

            if (connect_btn[i].toggled()) {
                if (connect_btn[i].get()) {
                    if (new_motor_connection(i, comport_selector[i].get())) {
                        motor_plot[i].label.rename((std::string((std::string)"Motor COM").append(String(comport_selector[i].get()))).c_str());
                    }
                }
                else {
                    disable_connection(i);
                    motor_plot[i].label.rename("Select Port");
                }

            }
        }

    }



    /** @brief Hide all GUI elements */
    void hide_all() {

    }


    /** @brief Reset all GUI elements */
    void reset_all() {

    }

    void disable_connection(int motor_id) {
        //motor_id = comport_select_panel.get_position();

        if (motors[motor_id].is_connected()) {

            // Print delete message
            IC4_virtual->print_l("Disabling motor on port ");
            IC4_virtual->print_l(String(motors[motor_id].modbus_client.get_port_number()).c_str());
            IC4_virtual->print_l("\r");

            //disable comms for this actuator object
            motors[motor_id].disable();
            motors[motor_id].disable_comport();

        }
    }

    /** @brief handles connection to a new comport - reuses the same actuator object */
    bool new_motor_connection(int motor_id, int new_motor_port) {

        //create new motor object and add it to the vector
        std::string* _name = new std::string(((std::string)"Motor ").append(String(motor_id)));
        const char* motor_name = (*_name).c_str();

        //get new comport value, pass it to the actuator object and re-init      
        motors[motor_id].set_new_comport(new_motor_port);

        motors[motor_id].init();

        //check to make sure the new actuator creation was successful
        if (motors[motor_id].modbus_client.serial_success && !(new_motor_port == motors[motor_id].modbus_client.get_port_number() && motors[motor_id].is_connected())) {

            // Print connection success message
            IC4_virtual->print_l("Connecting motor on port ");
            IC4_virtual->print_l((String(new_motor_port).c_str()));
            IC4_virtual->print_l("\r");

            // enable connected motor
            motors[motor_id].enable();

            return true;

        }
        else {
            //motor serial port connection fail
            const char* error_msg = "Unable to open a connection to a motor on port ";
            IC4_virtual->print_l(error_msg);
            IC4_virtual->print_l((String(new_motor_port).c_str()));
            IC4_virtual->print_l("\r");

            return false;
        }

    }
};