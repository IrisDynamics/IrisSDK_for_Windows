/**
 * @file iris_gui.h
 *  @author Rebecca McWilliam <rmcwilliam@irisdynamics.com>
 *
 * @brief  GUI displyaing motor feedback and allowing stream parameter updating
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
#include "ic4_library/io_elements.h"
#include "windows.h"
#include "modbus_client/device_applications/actuator.h"
 //#include "Comport_Select.h"
#include <vector>
#include "irisSDK_libraries/device_config.h"
#include "irisSDK_libraries/Motor_Plot_Panel.h"
#include <memory>



class GUI : public IC4_windows {
public:
    uint16_t last_GUI_update;
    Actuator& motor;

public:
    uint32_t tnow = millis();
    Motor_Plot motor_plot;           //from gui_panels library
    FlexData comport_selector;
    FlexButton connect_btn;                                 // Button for disabling connection to the motor
    //GUI stuff 
    uint32_t gui_timer = 0;                                 // This will keep track of the last update
    uint8_t gui_update_period = 10;                         // This is the time between updates in milliseconds (100 ms = 10 fps)
    static const uint8_t num_stream_modes = 3;
    FlexDropdown stream_mode_dropdown; 
    MenuOption stream_mode_options[num_stream_modes];
    const char* stream_option_names[num_stream_modes]{
        "Command Stream",
        "Read Stream",
        "Write Stream"
    };

    static const uint8_t num_operation_modes = 5;
    FlexDropdown operation_mode_dropdown;
    MenuOption operation_mode_options[num_operation_modes];
    const char* operation_mode_option_names[num_operation_modes]{
        "Sleep",
        "Force",
        "Position",
        "Haptic",
        "Kinematic"
    };

    FlexData reg_addr;
    FlexData reg_data;
    FlexData reg_width;
    FlexData force_command;
    FlexData position_command;
    FlexButton update_message;

    GUI(Actuator& _motor
    ) :
        motor(_motor)
    {
        set_server_name("Haptics Stream");
        set_device_id("WindowsApp");
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

        gui_set_grid(35, 50);                         // This will set the size of the IrisControls window

        // Message is printed when IrisControls establishes a connection with the device program
        print_l("Please select the comport of your RS422 connection.\r");
        comport_selector.add("COM: ", 0, 19, 1, 7, 0, 1, FlexData::ALLOW_INPUT);
        connect_btn.add("Connect", 0, 0, 28, 1, 4);

        motor_plot.add(&motor, "Orca", 3, 19, 25, 30);

        operation_mode_dropdown.add(25, 0, 1, 7, FlexDropdown::SORT_BY_OPTION_ID);
        for (int j = 0; j < num_operation_modes; j++) {
            operation_mode_dropdown.add_option(&operation_mode_options[j], operation_mode_option_names[j]);
        }
        //operation_mode_dropdown.set_colour(FlexDropdown::ACTIVE_OPTION_TEXT, OFF_WHITE);
        //operation_mode_dropdown.set_colour(FlexDropdown::DROPDOWN_MENU_TEXT, OFF_WHITE);

        stream_mode_dropdown.add(25, 10, 1, 7, FlexDropdown::SORT_BY_OPTION_ID);
        for (int j = 0; j < num_stream_modes; j++) {
            stream_mode_dropdown.add_option(&stream_mode_options[j], stream_option_names[j]);
        }

       
        reg_addr.add("address", 27, 0, 1, 9, 0, 1, FlexData::ALLOW_INPUT);
        reg_addr.hide();
        reg_width.add("width", 28, 0, 1, 9, 1, 1, FlexData::ALLOW_INPUT);
        reg_width.hide();
        reg_data.add("data", 29, 0, 1, 9, 0, 1, FlexData::ALLOW_INPUT);
        reg_data.hide();
        force_command.add("Force(N)", 27, 0, 1, 9, 0, 1000, FlexData::ALLOW_INPUT);
        force_command.hide();
        position_command.add("Position(mm)", 27, 0, 1, 9, 0, 1000, FlexData::ALLOW_INPUT);
        position_command.hide();
        update_message.add("Write\nData", -1, 27, 10, 3, 6);
        update_message.hide();
    }


    /** @brief Actions to be called every GUI frame go here. This is called each gui_update_period once an IC4 connection is established */
    void frame_update() {
        motor_plot.run();
        if (stream_mode_dropdown.new_value_received()) {
            reg_addr.hide();
            reg_width.hide();
            reg_data.hide();
            force_command.hide();
            position_command.hide();
            update_message.hide();
            uint8_t id = stream_mode_dropdown.get();
            if (id == stream_mode_options[Actuator::MotorCommand].id()) {
                motor.set_stream_mode(Actuator::MotorCommand);
                if (motor.get_mode() == Actuator::ForceMode) force_command.show();
                else if (motor.get_mode() == Actuator::PositionMode) position_command.show();
            }
            else if (id == stream_mode_options[Actuator::MotorRead].id()) {
                motor.set_stream_mode(Actuator::MotorRead);
                reg_addr.show();
                reg_width.show();
                reg_data.show();
                reg_data.disable(1);
            }
            else if (id == stream_mode_options[Actuator::MotorWrite].id()) {
                motor.set_stream_mode(Actuator::MotorWrite);
                reg_addr.show();
                reg_width.show();
                reg_data.show();
                reg_data.disable(0);
                update_message.show();
            }
        }

        if (operation_mode_dropdown.new_value_received()) {
            uint8_t id = operation_mode_dropdown.get();
            force_command.hide();
            position_command.hide();
            if (id == operation_mode_options[0].id()) {
                motor.set_mode(Actuator::SleepMode);
            }
            else if(id == operation_mode_options[1].id()) {
                motor.set_mode(Actuator::ForceMode);
                if (motor.get_stream_mode() == Actuator::MotorCommand) {
                    force_command.show();
                }
            }
            else if (id == operation_mode_options[2].id()) {
                motor.set_mode(Actuator::PositionMode);
                if (motor.get_stream_mode() == Actuator::MotorCommand) position_command.show();
            }
            else if (id == operation_mode_options[3].id()) {
                motor.set_mode(Actuator::HapticMode);
            }
            else if (id == operation_mode_options[4].id()) {
                motor.set_mode(Actuator::KinematicMode);
            }
        }

        if (connect_btn.toggled()) {
            if (connect_btn.get()) {
                if (new_motor_connection(comport_selector.get())) {
                    motor_plot.label.rename((std::string((std::string)"Motor COM").append(String(comport_selector.get()))).c_str());
                }
            }
            else {
                disable_connection();
                motor_plot.label.rename("Select Port");
            }

        }

        motor.set_force_mN(force_command.get());
        motor.set_position_um(position_command.get());
        if (motor.get_stream_mode() == Actuator::MotorRead) {
            if (reg_addr.new_value_received()) motor.update_read_stream(reg_width.get(), reg_addr.get());
            reg_data.update(motor.get_orca_reg_content(reg_addr.get()));
        }
        if (update_message.pressed()) {
            motor.update_write_stream(reg_width.get(), reg_addr.get(), reg_data.get());
        }

    }


    /** @brief Hide all GUI elements */
    void hide_all() {

    }


    /** @brief Reset all GUI elements */
    void reset_all() {

    }

    void disable_connection() {
        //motor_id = comport_select_panel.get_position();

        if (motor.is_connected()) {

            // Print delete message
            IC4_virtual->print_l("Disabling motor on port ");
            IC4_virtual->print_l(String(motor.modbus_client.get_port_number()).c_str());
            IC4_virtual->print_l("\r");

            //disable comms for this actuator object
            motor.disable();
            motor.disable_comport();
        }
    }

    /** @brief handles connection to a new comport - reuses the same actuator object */
    bool new_motor_connection(int new_motor_port) {

        //create new motor object and add it to the vector
        std::string* _name = new std::string(((std::string)"Motor ").append(String(new_motor_port)));
        const char* motor_name = (*_name).c_str();

        //get new comport value, pass it to the actuator object and re-init      
        motor.set_new_comport(new_motor_port);

        motor.init();

        //check to make sure the new actuator creation was successful
        if (motor.modbus_client.serial_success && !(new_motor_port == motor.modbus_client.get_port_number() && motor.is_connected())) {

            // Print connection success message
            IC4_virtual->print_l("Connecting motor on port ");
            IC4_virtual->print_l((String(new_motor_port).c_str()));
            IC4_virtual->print_l("\r");

            // enable connected motor
            motor.enable();

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