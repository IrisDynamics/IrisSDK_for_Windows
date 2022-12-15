/**
    @file Position_Control_Page.h
    @author Michelle Aleman <maleman@irisdynamics.com>
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

#ifndef _POSITION_CONTROL_PAGE_H_
#define _POSITION_CONTROL_PAGE_H_

#include "irisSDK_libraries/Gain_Panel.h"
#include "irisSDK_libraries/Signal_Panel.h"
#include "irisSDK_libraries/Motor_Plot_Panel.h"

/**
    @file Position_Control_Page.h
    @class Position_Control_Page
    @brief Provides an interface containing signal and gain panels for updating the motor's target position
*/
class Position_Control_Page {

    Actuator& motor;                                                // Reference to an actuator object that will be passed in when this object is initialized

    Gain_Panel gain_panel;                                          // Interface for updating Motor PID gain and maximum force values
    FlexButton sleep_btn;                                           // Button for switching the motor's mode between 'Position' and 'Sleep'
    Signal_Panel signal_panel;                                      // Provides interface for entering signal type and parameters
    int32_t signal_init_value;                                      // Value used by the signal panel to track the motor's starting position, used to avoid sudden extreme changes in position
    int32_t signal_target_value;                                    // Target position value
    Dataset position_signal;                                        // Dataset to add to plot. Data will always take the Signal_Panel signal value
    Motor_Plot motor_plot;                                          // Motor plot object to display live motor data
    FlexLabel page_label;                                           // Position Control page label element
    GUI_Page  page_elements;                                        // GUI_Page object to handle displaying/hiding all elements in position control page

    bool first_setup = true;                                        // Boolean value tracks whether or not the page has been initialized yet

public:

    bool is_running = false;                                         // Boolean value tracks whether or not the page is active


    /* Constructor */
    Position_Control_Page(
        /** Parameter for constructing the GUI object is a reference to a motor. This will allow this object to have access to control and feedback from the motor.*/
        Actuator& _motor,
        Signal_Generator& _signal_generator
    ) :
        /* Initialization List */
        motor(_motor),
        signal_panel(_signal_generator)
    {}


    /** @brief Handles initializing position control page elements */
    void setup() {
        if (first_setup) {

            // Update first_setup to indicate the setup for this page is complete
            first_setup = false;

            // Initialize GUI_Page object
            page_elements.add();

            // Initialize sleep button
            sleep_btn.add(&page_elements, "Sleep", 0, 3, 19, 2, 4);

            // Initialize gain panel
            gain_panel.add(&page_elements, &motor, 18, 19);


            motor.tune_position_controller(gain_panel.p_gain.get(), gain_panel.i_gain.get(), gain_panel.d_gain.get(), gain_panel.f_max.get());

            // Initialize signal panel
            signal_panel.add(&page_elements, &signal_init_value, 5, 19);

            // Initialize motor plot
            motor_plot.add(&page_elements, &motor, "Orca Motor", 1, 35);

            // Add position signal to plot
            position_signal.add(&motor_plot.plot, "Position Signal", "Time", "*mu*m", Dataset::TIMEPLOT + Dataset::NONE);
            position_signal.set_max_data_points(25000);
            position_signal.set_colour(GREEN);
            position_signal.show();

            // Initialize position control page label element
            page_label.add(&page_elements, "<p_style=\"font-size:20px;\">Position Control</p>", 1, 19, 2, 10);

        }
        else {
            signal_init_value = motor.get_position_um();
            signal_target_value = signal_init_value;
            page_elements.show();
        }
        is_running = true;
        motor.set_mode(Actuator::PositionMode);
    }


    /** @brief Handles updating the position control page with motor data */
    void run() {
        if (!is_running) return;

        // Put motor into sleep mode if sleep button toggled on
        if (sleep_btn.get()) {
            motor.set_mode(Actuator::SleepMode);
        }
        else {
            motor.set_mode(Actuator::PositionMode);
        }

        // Update gain values if update button pressed
        gain_panel.run();

        // Update signal init value only if the signal panel is not paused
        if (!signal_panel.signal_generator.signal_paused) {
            signal_init_value = motor.get_position_um();
        }

        // Update signal fields displayed depending on slider value
        signal_panel.run();

        // Update position signal dataset with signal value
        position_signal.add_data(IC4_virtual->system_time(), (int)(signal_panel.signal_generator.get_target_value()));

        // Update plot with motor data
        motor_plot.run();
    }

    /** @brief Hides all Position Control Page elements in GUI view */
    void hide() {
        if (is_running) {
            signal_panel.pause();
            page_elements.hide();
            is_running = false;
        }
    }


    /** @brief Resets all Position Control Page elements in GUI view */
    void reset() {
        hide();
        first_setup = true;
    }
};

#endif
