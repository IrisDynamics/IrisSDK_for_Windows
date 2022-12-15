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


#include "ConstantForce.h"
#include "Spring.h"
#include "Damping.h"
#include "Motion.h"
#include "../modbus_client/device_applications/actuator.h"


class Force_Effect_Generator {
public:
    ConstantForce constant_force;
    SpringEffect spring_effect;
    DampingEffect damping_effect;
    Derivative speed{0.02};
    int32_t force_value= 0;

    // Actuator Object
    Actuator & motor;

    bool is_running = false;

    

        Force_Effect_Generator(Actuator& _motor) :
            motor(_motor)
        {}


        /** @brief Updates the Actuator object's force with the sum of the force effect objects */
        int32_t run(){
            if (!is_running) force_value = 0;
            else {
                // Update spring effect, speed, and damping effect object values when the actuator object has new data
                if (motor.new_data()) {

                    // Update spring effect object with actuator position in millimeters
                    spring_effect.update(motor.get_position_um() / 1000.);

                    // Update speed object with actuator position in millimeters
                    speed.update(motor.get_position_um() / 1000.);

                    // Update damping effect object with speed
                    damping_effect.update(speed.get());
                }

                // Get sum of force effect object values
                force_value = 0;
                force_value += spring_effect.getForce();
                force_value += constant_force.getForce();
                force_value += damping_effect.getForce();
            }

            return force_value;
        }

        int32_t get_target_force() {
            return force_value;
        }

        void start() {is_running = true;}
        void stop() { is_running = false; }

        /** @brief Pauses panel by resetting force effect values to zero */
        void pause(){
            // Update force effect objects with new slider values of zero
           // constant_force.update(0);
           // spring_effect.adjustK(0);
           // damping_effect.adjustK(0);
        }

};