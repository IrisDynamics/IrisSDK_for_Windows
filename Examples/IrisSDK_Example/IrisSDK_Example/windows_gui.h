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


#include "ic4_library/iriscontrols4.h"
#include "ic4_library/device_drivers/windows/IC4_windows.h"
#include "modbus_client/device_applications/actuator.h"
#include "irisSDK_libraries/Motor_Plot_Panel.h"
#include "irisSDK_libraries/Comport_Select.h"

#pragma once
class GUI : public IC4_windows {
public:

    //GUI stuff 
    FlexData position_element;
    uint32_t gui_timer = 0;                                 // This will keep track of the last update
    uint8_t gui_update_period = 10;                         // This is the time between updates in milliseconds (100 ms = 10 fps)
    Comport_Select comport_select_panel;                    // Panel for selecting a comport to open a motor connection on
    FlexSlider motor_slider;                                //used to choose which motor you want to view 

    Actuator* motors;
    int array_size;                                         //size of the actuator array - used for looping through all motors

    Motor_Plot plots[2]{};

    uint8_t motor_id = 0;                                   // Value used to determine when the slider has been updated
    s32 motor_counter = 0;


    GUI(Actuator* _motor, int _array_size
    ) :
        motors(_motor),
        array_size(_array_size),
        comport_select_panel(_motor)
    {
        set_device_id("windows virtual device");
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

        gui_set_grid(30, 60);                         // This will set the size of the IrisControls window

        // Message is printed when IrisControls establishes a connection with the device program
        print_l("Please select the comport of your USB to RS422 connection.\r");
        comport_select_panel.add(motors, 3, 20, 2);         //Initialize the comport select panel 

        for (int i = 0; i < array_size; i++) {
            plots[i].add(&motors[i], motors[i].get_name(), 12, 25);
            plots[i].hide();
        }
    }


    /** @brief Actions to be called every GUI frame go here. This is called each gui_update_period once an IC4 connection is established */
    void frame_update() {

        comport_select_panel.run_gui();

        //check for display motor panel that matches the indicated motor on the slider - if the plot is not indicated on the display then hide it. 
        //then call run for all
        for (int i = 0; i < array_size; i++) {
            if (i == comport_select_panel.get_position()) {
                plots[i].show();
                plots[i].run();
            }
            else {
                plots[i].hide();
            }
        }

    }

    /** @brief Hide all GUI elements */
    void hide_all() {

    }


    /** @brief Reset all GUI elements */
    void reset_all() {

    }

};