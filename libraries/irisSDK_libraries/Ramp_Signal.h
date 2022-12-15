/**
 * @file Ramp_Signal
 * @class RampSignal
 * @brief Initializes SignalParameters given a start value, a target value, and a time-to-reach value, and generates a current target value to ramp to the final target value of the time `period`.
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


class RampSignal : public Signal_Parameters {

    public:

        /**
         * @brief Initializes Signal_Parameters for a ramp signal. These are the start value, start time, target value, and time to reach the target from the current value (period).
        */
        void init(int32_t init_value, int32_t target, uint32_t time_in){
            
            // Set start position
            start_value = init_value;
    
            // Set start time
            start_time = millis();
    
            // Set the # of microseconds it should take to reach the target position
            period = time_in;
    
            // Set final target position and current target position        
            target_1 = target;
      
            target_value = start_value;

        }



        /**
         * @brief Updates the target value with the next value in the ramp signal. Value depends on the signal parameters specified and the time elapsed since the signal was initialized. 
        */
        uint32_t run(){
    
            // Get time elapsed
            uint32_t elapsed_time = millis() - start_time;
            if(elapsed_time > period) elapsed_time = period; // ensure target value doesn't overshoot once the target has been reached
 
            if((start_value < target_1) && (target_value < target_1)){
                // If positive slope and current position is below peak: update position
                target_value = start_value + ((target_1 - start_value) * elapsed_time / period);  // evaluates to start_value at time 0, and to (start_value + (target_1 - start_value)) at time `period`. 
            }
            
            else if((start_value > target_1) && (target_value > target_1)){
                // If negative slope and current position is above minimum: update position
                target_value = start_value - ((start_value - target_1) * elapsed_time / period);  // evaluates to start_value at time 0, and to (start_value - (target_1 - start_value)) at time `period`.
            }
    
            // Return actuators position
            return target_value;               
        }

};
