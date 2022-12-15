/**
   @file Triangle_Wave_Signal.h
   @class TriangleWave
   @brief Initializes and runs a triangle wave signal using SignalParameters to save values such as the wave's minimum and maximum values and period
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

#include "Signal_Parameters.h"


class TriangleWave : public Signal_Parameters {

    public:

        /**
         * @brief Initializes Signal_Parameters for a triangle wave signal. These include the starting value, maximum target value, minimum target value, period, and start time.
        */
        void init(int32_t init_value, uint32_t max_target, uint32_t min_target, float frequency){
    
            // Set start position = current position
            start_value = init_value; //actuator.get_position_um();
    
            // Set start time
            start_time = millis();
    
            // Set the # of microseconds it should take to reach the target position
            period = 500 * (1 / frequency); // Multiply by 1000/2 to get the # of microseconds between the maximum and minimum
            //period = 500000000/frequency;
            
            // Set min and max target values
            target_1 = max_target;
            target_2 = min_target;

        }


        /**
         * @brief Updates the target value with the next value in the triangle wave. Value depends on the signal parameters specified and the time elapsed since the signal was initialized. 
         * Note the initial target value will be ramped to either the min or max value to smoothly transition the start value into the signal. 
        */
        uint32_t run(){

            unsigned long long elapsed_time = millis() - start_time;
    
            if(start_value < target_1){  

                 // Update value when slope is positive (below expression = start_value at time 0, and = (start_value + ((start_value - target_2)) at time `period`)
                 target_value = start_value + ((target_1 - start_value) * elapsed_time / period);
    
                  // If the slope is positive and the current position is equal to or above the peak 
                  if((elapsed_time >= period) ||  (target_value >= target_1)){
                    // Reached maximum: Set target_value to the maximum target value, change direction by resetting start_value and start_time.
                    start_time = millis();
                    start_value = target_1; 
                    target_value = target_1; 
                }
            }
    
            else{

                // Update value when slope is negative (below expression = start_value at time 0, and = (start_value - ((start_value - target_2)) at time `period`)
                target_value = start_value - ((start_value - target_2) * elapsed_time / period);
    
                // If the slope is negative and the current position is equal to or below the peak 
                if(elapsed_time >= period  ||  (target_value < target_2)){
                    // Reached minimum: Set target_value to the minimum target value, change direction by resetting start_value and start_time. 
                    start_time = millis();
                    start_value = target_2; 
                    target_value = target_2; 
                }
            }
    
            // Return target value
            return target_value;
        }

};
