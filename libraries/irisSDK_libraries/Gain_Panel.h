/**
   @file Gain_Panel.h
   @author Michelle Aleman <maleman@irisdynamics.com>
   @brief interface for updating each the motor's PID controller gains and its f_max.
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

#include "ic4_library/iriscontrols4.h"


/**
 * @file Gain_Panel.h
   @class Gain_Panel
   @brief GUI panel provides an  interface for updating an Orca motor's PID controller gains and maximum force using an Actuator object
            
*/
class Gain_Panel {

    // GUI_Page object handles hiding/displaying all panel flex elements
    GUI_Page * panel_elements;

    // Actuator object used to update motor PID controller gain values
    Actuator * motor;

    // Button element for updating gain values in motor
    FlexButton update_btn;

    public:

        // Data elements for the p_gain, i_gain, and d_gain input fields
        FlexData p_gain, i_gain, d_gain, f_max;


        /** @brief Function initializes all panel flex elements and adds them to a new local GUI_Page */
        void add(Actuator * _motor, uint8_t y_anchor, uint8_t x_anchor){
            panel_elements = new GUI_Page();
            motor = _motor;
            init(y_anchor, x_anchor);
        }


        /** @brief Function initializes all panel flex elements and adds them to the GUI_Page provided */
        void add(GUI_Page * _panel_elements, Actuator * _motor, uint8_t y_anchor, uint8_t x_anchor){
            panel_elements = _panel_elements;
            motor = _motor;
            init(y_anchor, x_anchor);
        }
        

        /** @brief Initializes the label and data elements for each gain in the GUI view */
        void init(uint8_t y_anchor, uint8_t x_anchor){

            // Init GUI_Page object
           panel_elements->add();

            // Add FlexData input fields for p_gain, i_gain, d_gain
            p_gain.add(panel_elements, "  P gain", y_anchor, x_anchor, 2, 9, 200, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7 + FlexData::VALUE_ALIGN_LEFT);
            i_gain.add(panel_elements, "  I gain", y_anchor + 2, x_anchor, 2, 9, 0, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7 + FlexData::VALUE_ALIGN_LEFT);
            d_gain.add(panel_elements, "  D gain", y_anchor + 4, x_anchor, 2, 9, 0, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7 + FlexData::VALUE_ALIGN_LEFT);

            // Add FlexData element for f_max
            f_max.add(panel_elements,  "Max Force", y_anchor + 6, x_anchor, 2, 9, 10000, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7 + FlexData::VALUE_ALIGN_LEFT);

            // Add update button
           update_btn.add(panel_elements, "Update", -1, y_anchor + 5, x_anchor + 10, 2, 4);

        }


        /** @brief Function updates the actuator with the given id's p_gain, i_gain, and d_gain using values in the GUI input fields, and sets the maximum force value to 5000 */
        void run(){

            if(update_btn.pressed()){
                motor->tune_position_controller((uint16_t) p_gain.get(), (uint16_t) i_gain.get(), (uint16_t) d_gain.get(), f_max.get());
            }    
        }


        /**
         * @brief Hides all gain input elements in GUI view
        */
        void hide(){
            panel_elements->hide();
        }
        


        /**
         * @brief Shows all gain input elements in GUI view
        */
        void show(){
            panel_elements->show();
        }
        

};