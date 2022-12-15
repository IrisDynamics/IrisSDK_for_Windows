/**
    @file Force_Effect_Panel.h
    @class Force_Effect_Panel
    @brief Contains slider input fields for force effects: spring gain and center, damper gain, and force control.
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


#pragma once


// #include <k20_libraries.h>
#include "ConstantForce.h"
#include "Spring.h"
#include "Damping.h"
#include "Motion.h"
#include "../ic4_library/iriscontrols4.h"
#include "Force_Effect_Generator.h"


class Force_Effect_Panel {

    Force_Effect_Generator* ff_gen;
    //ConstantForce constant_force;
    //SpringEffect spring_effect;
    //DampingEffect damping_effect;
    //Derivative speed{0.02};

    // Actuator Object
    Actuator * motor;
    
    // GUI_Page to handle displaying/hiding elements in panel
    GUI_Page * panel_elements;

    // Sum of force effect values
    int32_t force_value = 0;

    // Label elements for spring, spring gain, and spring center
    FlexLabel spring_label, spring_gain_label, spring_center_label;

    // Slider elements for spring gain and spring center
    FlexSlider spring_gain, spring_center;

    // Label elements for damper and damper gain
    FlexLabel damper_label, damper_gain_label;

    // Slider element for damper gain
    FlexSlider damper_gain;

    // Label and slider element for constant force
    FlexLabel cforce_label;
    FlexSlider cforce_slider;

    // Button to trigger setting force effects in motor
    FlexButton update_btn;


    public:


        /** @brief Initializes all flex elements in the force effect panel and adds them to a new local GUI_Page object */
        void add(Force_Effect_Generator* _ff_gen, uint8_t y_anchor, uint8_t x_anchor){
            panel_elements = new GUI_Page();
            ff_gen = _ff_gen;
            init(y_anchor, x_anchor);
        }

        /** @brief Initializes all flex elements in the force effect panel and adds them to the specified GUI_Page object argument  */
        void add(GUI_Page * _panel_elements, Force_Effect_Generator * _ff_gen, uint8_t y_anchor, uint8_t x_anchor){
            panel_elements = _panel_elements;
            ff_gen = _ff_gen;
            init(y_anchor, x_anchor);
        }


        /** @brief Initializes all label and slider elements in GUI view */
        void init(uint8_t y_anchor, uint8_t x_anchor){

            // Init GUI_Page object
            panel_elements->add();
            
            // Add spring elements
            spring_label.add(panel_elements, "<p_style=\"font-size:14px;\">Spring</p>", y_anchor + 2, x_anchor + 5, 1, 4);
            spring_gain_label.add(panel_elements, "gain:", y_anchor + 3, x_anchor, 1, 3);
            spring_center_label.add(panel_elements, "center:", y_anchor + 4, x_anchor, 1, 3);
            spring_gain.add(panel_elements, "", y_anchor + 3, x_anchor + 4, 1, 11, 0, 100, 0, 1, FlexData::ALLOW_INPUT);
            spring_center.add(panel_elements, "", y_anchor + 4, x_anchor + 4, 1, 11, 0, 150, 0, 1, FlexData::ALLOW_INPUT);

            // Add damper elements
            damper_label.add(panel_elements, "<p_style=\"font-size:14px;\">Damper</p>", y_anchor + 6, x_anchor + 5, 1, 4);
            damper_gain_label.add(panel_elements, "gain:", y_anchor + 7, x_anchor, 1, 3);
            damper_gain.add(panel_elements, "", y_anchor + 7, x_anchor + 4, 1, 11, 0, 100, 0, 1, FlexData::ALLOW_INPUT);

            // Add constant force elements
            cforce_label.add(panel_elements, "<p_style=\"font-size:14px;\">Constant Force</p>", y_anchor + 9, x_anchor + 5, 1, 6);
            cforce_slider.add(panel_elements, "", y_anchor + 10, x_anchor + 4, 1, 11, -2000, 2000, 0, 1, FlexData::ALLOW_INPUT);

            // Add update button
            update_btn.add(panel_elements, "Update", -1, y_anchor, x_anchor, 2, 4);
            ff_gen->start();
        }


        /** @brief Updates the force effect objects with the force effect slider values */
        void run(){

            if(update_btn.pressed()){

                // Update constant force effect object with slider value
                ff_gen->constant_force.update(cforce_slider.get());

                // Adjust spring effect object with negative spring gain slider value
                ff_gen->spring_effect.adjustK(-1 * spring_gain.get());

                // Update spring effect center with spring center slider value
                ff_gen->spring_effect.moveCenter(spring_center.get());

                // Adjust damping effect object with negative damper gain slider value
                ff_gen->damping_effect.adjustK(-1 * damper_gain.get());
            }
        }



        /** @brief Pauses panel by resetting force effect values to zero */
        void pause(){
            // Update all sliders with a value of zero
            spring_gain.update(0);
            spring_center.update(0);
            damper_gain.update(0);
            cforce_slider.update(0);
            // Update force effect objects with new slider values of zero
            ff_gen->constant_force.update(cforce_slider.get());
            ff_gen->spring_effect.adjustK(-1 * spring_gain.get());
            ff_gen->spring_effect.moveCenter(spring_center.get());
            ff_gen->damping_effect.adjustK(-1 * damper_gain.get());
            // Update target force with value of zero
            force_value = 0;
        }


        /** @brief Hides all force effect elements in GUI view */
        void hide(){
            panel_elements->hide();   
            ff_gen->stop();
        }
        

        /** @brief Shows all force effect elements in GUI view */
        void show(){
            panel_elements->show();     
            ff_gen->start();
        }

};