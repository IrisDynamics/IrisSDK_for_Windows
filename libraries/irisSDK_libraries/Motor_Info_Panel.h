/**
   @file Motor_Info_Panel.h
   @author Michelle Aleman <maleman@irisdynamics.com>
   @brief Panel uses two flex labels to display motor's serial number and firmware version
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

#pragma once

#include "modbus_client/device_applications/actuator.h"
#include "ic4_library/iriscontrols4.h"


class Motor_Info_Panel{

    Actuator* motor;

    GUI_Page* panel_elements;

    FlexLabel serial_number_label;
    FlexLabel firmware_version_label;

    std::string serial_number_string = "serial number: ";
    std::string firmware_version_string = "firmware version: ";

    bool serial_number_valid = false;
    bool firmware_version_valid = false;

    bool serial_number_updated = false;
    bool firmware_version_updated = false;

    static const u8 default_row_anchor = 27;
    static const u8 default_col_anchor = 1;
    static const u8 default_row_span = 1;
    static const u8 default_col_span = 16;


    /** @brief Updates serial number flex label with serial number read from motor */
    void _update_serial_number(){
        if (!serial_number_updated && serial_number_valid) {
            serial_number_label.rename(serial_number_string.c_str());
            serial_number_updated = true;
        }
    }

    /** @brief Returns the serial number stored in the motor's registers 406 and 407 as unsigned int */
    void _read_serial_number() {
        if (!serial_number_valid) {
            u32 serial_lo = motor->get_orca_reg_content(SERIAL_NUMBER_LOW);
            u32 serial_hi = motor->get_orca_reg_content(SERIAL_NUMBER_HIGH);
            u32 serial_number = serial_lo + (serial_hi << 16);

            // validate serial number and update string if valid
            if ((String(serial_number).length() == 9)) {
                serial_number_valid = true;
                serial_number_string += String(serial_number);
            }
        }
    }


    /** @brief Updates firmware version flex label with firmware version read from motor */
    void _update_firmware_version() {
        if (!firmware_version_updated && firmware_version_valid) {
            firmware_version_label.rename(firmware_version_string.c_str());
            firmware_version_updated = true;
        }
    }


    /** @brief Returns firmware version read from motor as a std::string - format: (<major version>.<release state>.<revision number>) */
    void _read_firmware_version() {
        if (!firmware_version_valid) {
            u16 motor_version = motor->get_major_version();
            u16 release_state = motor->get_release_state();
            u16 revision_number = motor->get_revision_number();

            // validate firmware version number and update string if valid                  // TODO: determine how version number should be validated 
            if (motor_version != 0 /* && release_state != 0 */ && revision_number != 0) {
                firmware_version_valid = true;
                std::string version = String(motor_version) + "." + String(release_state) + "." + String(revision_number);
                firmware_version_string += version;
            }

        } 
    }


    void _setup(u16 row_anchor, u16 col_anchor, u16 row_span, u16 col_span) {
        // Read motor registers
        motor->read_registers(SERIAL_NUMBER_LOW, 2);

        // Init Labels
        panel_elements->add();
        serial_number_label.add(panel_elements, serial_number_string.c_str(), row_anchor, col_anchor, row_span, col_span);
        firmware_version_label.add(panel_elements, firmware_version_string.c_str(), row_anchor + 1, col_anchor, row_span, col_span);
    }


public:


    /** @brief Constructor initializes pointer to actuator object, determines which motor connection to read data from */
    Motor_Info_Panel(Actuator* _motor):
    // Initilization list
    motor(_motor)
    {}


    /** @brief Adds GUI_Page object, serial number flex label, and firmware version flex label to GUI */
    void add(GUI_Page* _panel_elements, u8 row_anchor = default_row_anchor, u8 col_anchor = default_col_anchor, u8 row_span = default_row_span, u8 col_span = default_col_span) {
        panel_elements = _panel_elements;
        _setup(row_anchor, col_anchor, row_span, col_span);
    }


    /** @brief Updates the serial number and firmware version flex labels with the serial number and firmware version read from the motor */
    void update() {
        _read_serial_number();
        _update_serial_number();
        _read_firmware_version();
        _update_firmware_version();
    }

    
    /** @brief Function for showing panel in GUI if GUI_Page reference was not provided */
    void show() {
        panel_elements->show();
    }


    /** @brief Function for hiding panel from GUI if GUI_Page reference was not provided */
    void hide() {
        panel_elements->hide();
    }


};
