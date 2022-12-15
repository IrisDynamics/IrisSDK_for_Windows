/**
    @file Force_Control_Page.h
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

#ifndef _FORCE_CONTROL_PAGE_H_
#define _FORCE_CONTROL_PAGE_H_

#include "irisSDK_libraries/Signal_Panel.h"
#include "irisSDK_libraries/Motor_Plot_Panel.h"
#include "irisSDK_libraries/Force_Effect_Panel.h"

/**
    @file Force_Control_Page.h
    @class Force_Control_Page
    @brief Provides an interface for updating an Orca motor's force using a generated signal and force effects
*/
class Force_Control_Page {

    Actuator& motor;
    Force_Effect_Generator& ff_gen;
    Force_Effect_Panel force_effect_panel;                          // Interface for updating the motor's force using a generated signal and force effects
    FlexButton sleep_btn;                                           // Button for switching the motor's mode between 'Force' and 'Sleep'
    Signal_Panel signal_panel;                                      // Provides interface for entering signal type and parameters                                     
    Dataset force_data;                                             // Dataset to add to plot. Data will always take the sum of the Signal_Panel signal value and the Force_Effect_Panel value
    Motor_Plot motor_plot;                                          // Motor plot object to display live motor data
    FlexLabel page_label;                                           // Force Control Page label element
    GUI_Page  page_elements;                                        // GUI_Page object to handle displaying/hiding all elements on force control page
    bool first_setup = true;                                        // Boolean value tracks whether or not the page has been initialized yet

public:

    bool is_running = false;                                        // Boolean value tracks whether or not the page is active


    /* Constructor */
    Force_Control_Page(
        /** Parameter for constructing the GUI object is a reference to a motor. This will allow this object to have access to control and feedback from the motor.*/
        Actuator& _motor,
        Force_Effect_Generator& _ff_gen,
        Signal_Generator& sig_gen
    ) :
        /* Initialization List */
        motor(_motor),
        ff_gen(_ff_gen),
        signal_panel(sig_gen)
    {}


    /** @brief Handles initializing force control page elements */
    void setup() {
        if (first_setup) {

            // Update first_setup to indicate the setup for this page is complete
            first_setup = false;

            // Initialize GUI_Page object
            page_elements.add();

            // Initialize force effect panel
            force_effect_panel.add(&page_elements, &ff_gen, 17, 19);

            // Initialize sleep button
            sleep_btn.add(&page_elements, "Sleep", 0, 3, 19, 2, 4);

            // Initialize signal panel
            signal_panel.add(&page_elements, 5, 19, "mN");

            // Initialize motor plot
            motor_plot.add(&page_elements, &motor, "Orca Motor", 1, 36);

            // Add target force dataset to plot
            force_data.add(&motor_plot.plot, "Target Force", "Time", "Force(mN)", Dataset::TIMEPLOT + Dataset::NONE + Dataset::SECONDARY_Y_AXIS);
            force_data.set_max_data_points(25000);
            force_data.set_colour(BLUE);
            force_data.show();

            // Update plot axis by replacing power with force
            motor_plot.plot.set_axes_labels(&force_data);
            motor_plot.plot.set_secondary_range(-15000, 15000);

            // Initialize force page label element
            page_label.add(&page_elements, "<p_style=\"font-size:20px;\">Force Control</p>", 1, 19, 2, 10);

        }
        else {
            page_elements.show();
        }
        is_running = true;
        motor.set_mode(Actuator::ForceMode);
    }


    /** @brief Handles updating the force control page with Orca data and running the signal and force effect panels. */
    void run_gui() {
        if (!is_running) return;

        // Put motor into sleep mode if sleep button toggled on
        if (sleep_btn.get()) {
            motor.set_mode(Actuator::SleepMode);
        }
        else {
            motor.set_mode(Actuator::ForceMode);
        }

        // Update force effect objects with slider values
        force_effect_panel.run();

        // Update signal fields displayed depending on slider value
        signal_panel.run();

        // Update target force dataset with target force value
        force_data.add_data(IC4_virtual->system_time(), ff_gen.get_target_force() + signal_panel.get_target_value());

        // Update plot with motor data
        motor_plot.run();
    }


    /* Hides all force control page elements in GUI view */
    void hide() {
        if (is_running) {
            //   signal_panel.pause();           // Pause signal panel
            force_effect_panel.pause();     // Pause force effect panel
            page_elements.hide();           // Hide elements
            is_running = false;             // Set page status to not running
        }
    }


    /* Resets all force control page elements in GUI view */
    void reset() {
        hide();
        first_setup = true;
    }
};

#endif
