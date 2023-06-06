/**
   @file Sine_Wave_Signal_Position.h
   @class SineWave
   @brief Initializes and runs a sine wave signal using SignalParameters to save values such as the wave's amplitude, offset from zero, and period. Not recommended for position control on a motor! There are no provisions to stop sudden and unexpected movement. 
   @version 1.0
    
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

#include "Sine_Wave_Signal.h"

class SineWavePosition : public SineWave {

public:
    int32_t amplitude = 0;

    /**
     * @brief Initializes Signal_Parameters for a sine wave signal. These include the starting value(zero crossing), amplitude, period (wavelength).
     * @param start_position - starting position for the sine wave (positive zero crossing)
     * @param max - int, peak of sine wave
     * @param min - int, valley of sine wave
     * @param wavelength - wavelength (period) of wavelength
     * 
    */
    void init(int32_t start_position, int32_t max, int32_t min, int32_t wavelength){

        // Update start value
        start_value = std::abs(start_position);
        
        period = wavelength;

        // Get amplitude
        amplitude = std::ceil((max-min)/2.0);

        target_1 = max;

        target_2 = min;

    }

    
    /**
     * @brief Generates and returns the value at x = current_position in the sine wave using a lookup table. Value depends on the signal parameters and input value.
    */
    int32_t run(int32_t current_position){
        int32_t relative_position = current_position;
        while (relative_position <= start_value) {
            relative_position = relative_position + period;
        }
        while (relative_position >= (start_value + period)) {
            relative_position = relative_position - period;
        }
        int32_t distance_from_start_of_wave = relative_position - start_value;
        int32_t quadrant_length = std::ceil(period / 4.0);
        if (distance_from_start_of_wave >= 0 && distance_from_start_of_wave < quadrant_length) {
            quadrant = 1;
        } else if(distance_from_start_of_wave >= quadrant_length && distance_from_start_of_wave < (quadrant_length*2)) {
            quadrant = 2;
        } else if (distance_from_start_of_wave >= (quadrant_length * 2) && distance_from_start_of_wave < (quadrant_length * 3)) {
            quadrant = 3;
        } else if (distance_from_start_of_wave >= (quadrant_length * 3) && distance_from_start_of_wave <= (quadrant_length * 4)) {
            quadrant = 4;
        }
        int32_t position_from_start_of_quadrant = distance_from_start_of_wave % quadrant_length;

        float percent_of_quadrant = position_from_start_of_quadrant / (float)(period / 4.0);
        if (percent_of_quadrant > 1) {
            percent_of_quadrant = 1;
        }
         
        // Update index
        switch (quadrant) {
        case 1:
            // lut index runs through 0 to 16384 each quarter period
            lut_index = (16384 * percent_of_quadrant);
            break;
        case 2:
            // lut index runs through 16384 to 32767 each quarter period
            lut_index = 16384 + (16384 * percent_of_quadrant);
            break;
        case 3:
            // lut index runs through -32768 to -16384 each quarter period
            lut_index = -32768 + (16384 * percent_of_quadrant);
            break;
        case 4:
            // lut index runs through -16384 to 0 each quarter period
            lut_index = -16384 + (16384 * percent_of_quadrant);
            break;
        }

        // Get sine value from index, + 65535 to prevent negative position values
        target_value = 65535 + lut(lut_index);

        // Scale to amplitude
        target_value = (target_value * ((float)target_1 / (float)65535));

        // Add offset
        target_value += target_2;

        return target_value;
    }


};
