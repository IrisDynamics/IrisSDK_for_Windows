/**
   @file Square_Wave_Signal.h
   @class SquareWave
   @brief Initializes and runs a square wave signal using SignalParameters to save values such as the wave's minimum and maximum values and period
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


class SquareWave : public Signal_Parameters {

    public:

        /**
         * @brief Initializes Signal_Parameters for a square wave signal. These include the starting value, maximum target position, minimum target position, start time, and period.
        */
        void init(int32_t init_value, uint32_t max_target, uint32_t min_target, float frequency){
    
            // Set start position = current position
            start_value = init_value;
    
            // Set start time
            start_time = millis();

            period = 1000. / frequency;

            // Set max target position and min target position
            target_1 = max_target;
            target_2 = min_target;
    
            // Set current target position to be the min value of the square wave
            target_value = target_2;


        }


        /**
         * @brief Updates the target value with the next value in the square wave signal. Value depends on the signal parameters specified and the time elapsed since the signal was initialized. 
        */
        uint32_t run(){

            // get elapsed time since start of quadrant
            unsigned long long elapsed_time = millis() - start_time;
    
            if(elapsed_time >= period){
                
                // Switch between max and min position
                if(target_value == target_1){
                    // Current position is max, update to min
                    target_value = target_2; 
                }
                else{
                    // Current position is min, switch to max
                    target_value = target_1;
                }
    
                // Reset start time
                start_time = millis();
            }
    
            // Return target position
           return target_value;

        }


};
