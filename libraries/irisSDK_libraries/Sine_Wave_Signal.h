/**
   @file Sine_Wave_Signal.h
   @class SineWave
   @brief Initializes and runs a sine wave signal using SignalParameters to save values such as the wave's amplitude, offset from zero, and period
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



/*!
* sine function lookup table contains 1025 values for one quadrant, ie 1025 values for sin(x) where x = [0 - 90]. Values are converted to unsigned 16-bit integers.
*/
static constexpr uint16_t sine_table[1025] =
{
    0, 101, 201, 302, 402, 503, 603, 704, 804, 905, 1005, 1106, 1206, 1307, 1407, 1508, 1608, 1709, 1809, 1910, 2010, 2111, 2211, 2312, 2412, 2513, 2613, 2714, 2814, 2914, 3015, 3115,
    3216, 3316, 3416, 3517, 3617, 3718, 3818, 3918, 4019, 4119, 4219, 4320, 4420, 4520, 4621, 4721, 4821, 4921, 5022, 5122, 5222, 5322, 5422, 5523, 5623, 5723, 5823, 5923, 6023, 6123, 6223, 6324,
    6424, 6524, 6624, 6724, 6824, 6924, 7024, 7123, 7223, 7323, 7423, 7523, 7623, 7723, 7823, 7922, 8022, 8122, 8222, 8321, 8421, 8521, 8620, 8720, 8820, 8919, 9019, 9118, 9218, 9318, 9417, 9517,
    9616, 9715, 9815, 9914, 10014, 10113, 10212, 10312, 10411, 10510, 10609, 10708, 10808, 10907, 11006, 11105, 11204, 11303, 11402, 11501, 11600, 11699, 11798, 11897, 11996, 12094, 12193, 12292, 12391, 12489, 12588, 12687,
    12785, 12884, 12982, 13081, 13179, 13278, 13376, 13475, 13573, 13671, 13770, 13868, 13966, 14064, 14163, 14261, 14359, 14457, 14555, 14653, 14751, 14849, 14947, 15045, 15142, 15240, 15338, 15436, 15533, 15631, 15729, 15826,
    15924, 16021, 16119, 16216, 16313, 16411, 16508, 16605, 16703, 16800, 16897, 16994, 17091, 17188, 17285, 17382, 17479, 17576, 17673, 17769, 17866, 17963, 18060, 18156, 18253, 18349, 18446, 18542, 18639, 18735, 18831, 18928,
    19024, 19120, 19216, 19312, 19408, 19504, 19600, 19696, 19792, 19888, 19984, 20079, 20175, 20271, 20366, 20462, 20557, 20653, 20748, 20843, 20939, 21034, 21129, 21224, 21319, 21414, 21509, 21604, 21699, 21794, 21889, 21983,
    22078, 22173, 22267, 22362, 22456, 22551, 22645, 22739, 22834, 22928, 23022, 23116, 23210, 23304, 23398, 23492, 23586, 23679, 23773, 23867, 23960, 24054, 24147, 24241, 24334, 24428, 24521, 24614, 24707, 24800, 24893, 24986,
    25079, 25172, 25265, 25358, 25450, 25543, 25635, 25728, 25820, 25913, 26005, 26097, 26189, 26281, 26374, 26466, 26557, 26649, 26741, 26833, 26925, 27016, 27108, 27199, 27291, 27382, 27473, 27565, 27656, 27747, 27838, 27929,
    28020, 28111, 28201, 28292, 28383, 28473, 28564, 28654, 28745, 28835, 28925, 29015, 29106, 29196, 29286, 29375, 29465, 29555, 29645, 29734, 29824, 29913, 30003, 30092, 30181, 30271, 30360, 30449, 30538, 30627, 30716, 30804,
    30893, 30982, 31070, 31159, 31247, 31335, 31424, 31512, 31600, 31688, 31776, 31864, 31952, 32039, 32127, 32215, 32302, 32390, 32477, 32564, 32651, 32738, 32826, 32912, 32999, 33086, 33173, 33260, 33346, 33433, 33519, 33605,
    33692, 33778, 33864, 33950, 34036, 34122, 34208, 34293, 34379, 34465, 34550, 34635, 34721, 34806, 34891, 34976, 35061, 35146, 35231, 35315, 35400, 35485, 35569, 35654, 35738, 35822, 35906, 35990, 36074, 36158, 36242, 36326,
    36409, 36493, 36576, 36660, 36743, 36826, 36909, 36992, 37075, 37158, 37241, 37324, 37406, 37489, 37571, 37653, 37736, 37818, 37900, 37982, 38064, 38145, 38227, 38309, 38390, 38472, 38553, 38634, 38715, 38797, 38877, 38958,
    39039, 39120, 39200, 39281, 39361, 39442, 39522, 39602, 39682, 39762, 39842, 39922, 40001, 40081, 40161, 40240, 40319, 40398, 40478, 40557, 40635, 40714, 40793, 40872, 40950, 41029, 41107, 41185, 41263, 41341, 41419, 41497,
    41575, 41653, 41730, 41808, 41885, 41962, 42039, 42117, 42194, 42270, 42347, 42424, 42500, 42577, 42653, 42730, 42806, 42882, 42958, 43034, 43109, 43185, 43261, 43336, 43411, 43487, 43562, 43637, 43712, 43787, 43861, 43936,
    44011, 44085, 44159, 44234, 44308, 44382, 44456, 44529, 44603, 44677, 44750, 44824, 44897, 44970, 45043, 45116, 45189, 45262, 45334, 45407, 45479, 45552, 45624, 45696, 45768, 45840, 45912, 45983, 46055, 46127, 46198, 46269,
    46340, 46411, 46482, 46553, 46624, 46694, 46765, 46835, 46905, 46976, 47046, 47116, 47185, 47255, 47325, 47394, 47464, 47533, 47602, 47671, 47740, 47809, 47877, 47946, 48014, 48083, 48151, 48219, 48287, 48355, 48423, 48491,
    48558, 48626, 48693, 48760, 48827, 48894, 48961, 49028, 49095, 49161, 49228, 49294, 49360, 49426, 49492, 49558, 49624, 49689, 49755, 49820, 49885, 49951, 50016, 50080, 50145, 50210, 50274, 50339, 50403, 50467, 50531, 50595,
    50659, 50723, 50787, 50850, 50913, 50977, 51040, 51103, 51166, 51228, 51291, 51354, 51416, 51478, 51540, 51602, 51664, 51726, 51788, 51849, 51911, 51972, 52033, 52094, 52155, 52216, 52277, 52337, 52398, 52458, 52518, 52578,
    52638, 52698, 52758, 52817, 52877, 52936, 52995, 53054, 53113, 53172, 53231, 53289, 53348, 53406, 53464, 53522, 53580, 53638, 53696, 53754, 53811, 53868, 53925, 53983, 54039, 54096, 54153, 54210, 54266, 54322, 54378, 54434,
    54490, 54546, 54602, 54657, 54713, 54768, 54823, 54878, 54933, 54988, 55042, 55097, 55151, 55206, 55260, 55314, 55367, 55421, 55475, 55528, 55582, 55635, 55688, 55741, 55794, 55846, 55899, 55951, 56003, 56056, 56108, 56159,
    56211, 56263, 56314, 56366, 56417, 56468, 56519, 56570, 56620, 56671, 56721, 56772, 56822, 56872, 56922, 56972, 57021, 57071, 57120, 57169, 57218, 57267, 57316, 57365, 57413, 57462, 57510, 57558, 57606, 57654, 57702, 57749,
    57797, 57844, 57891, 57938, 57985, 58032, 58079, 58125, 58171, 58218, 58264, 58310, 58356, 58401, 58447, 58492, 58537, 58583, 58628, 58672, 58717, 58762, 58806, 58850, 58895, 58939, 58983, 59026, 59070, 59113, 59157, 59200,
    59243, 59286, 59329, 59371, 59414, 59456, 59498, 59540, 59582, 59624, 59666, 59707, 59749, 59790, 59831, 59872, 59913, 59953, 59994, 60034, 60075, 60115, 60155, 60194, 60234, 60274, 60313, 60352, 60391, 60430, 60469, 60508,
    60546, 60585, 60623, 60661, 60699, 60737, 60775, 60812, 60850, 60887, 60924, 60961, 60998, 61034, 61071, 61107, 61144, 61180, 61216, 61252, 61287, 61323, 61358, 61393, 61429, 61463, 61498, 61533, 61567, 61602, 61636, 61670,
    61704, 61738, 61772, 61805, 61838, 61872, 61905, 61938, 61970, 62003, 62035, 62068, 62100, 62132, 62164, 62196, 62227, 62259, 62290, 62321, 62352, 62383, 62414, 62444, 62475, 62505, 62535, 62565, 62595, 62625, 62654, 62684,
    62713, 62742, 62771, 62800, 62829, 62857, 62886, 62914, 62942, 62970, 62998, 63025, 63053, 63080, 63107, 63134, 63161, 63188, 63214, 63241, 63267, 63293, 63319, 63345, 63371, 63396, 63422, 63447, 63472, 63497, 63522, 63546,
    63571, 63595, 63620, 63644, 63668, 63691, 63715, 63738, 63762, 63785, 63808, 63831, 63853, 63876, 63898, 63921, 63943, 63965, 63986, 64008, 64030, 64051, 64072, 64093, 64114, 64135, 64155, 64176, 64196, 64216, 64236, 64256,
    64276, 64295, 64315, 64334, 64353, 64372, 64391, 64409, 64428, 64446, 64464, 64482, 64500, 64518, 64535, 64553, 64570, 64587, 64604, 64621, 64638, 64654, 64671, 64687, 64703, 64719, 64734, 64750, 64765, 64781, 64796, 64811,
    64826, 64840, 64855, 64869, 64883, 64898, 64911, 64925, 64939, 64952, 64966, 64979, 64992, 65005, 65017, 65030, 65042, 65054, 65066, 65078, 65090, 65102, 65113, 65125, 65136, 65147, 65158, 65168, 65179, 65189, 65199, 65210,
    65219, 65229, 65239, 65248, 65258, 65267, 65276, 65285, 65293, 65302, 65310, 65319, 65327, 65335, 65342, 65350, 65357, 65365, 65372, 65379, 65386, 65392, 65399, 65405, 65412, 65418, 65424, 65429, 65435, 65441, 65446, 65451,
    65456, 65461, 65466, 65470, 65475, 65479, 65483, 65487, 65491, 65494, 65498, 65501, 65504, 65507, 65510, 65513, 65515, 65518, 65520, 65522, 65524, 65526, 65527, 65529, 65530, 65531, 65532, 65533, 65534, 65534, 65535, 65535, 65535
};



class SineWave : public Signal_Parameters {

public:
    int32_t amplitude = 0;

    short lut_index = -16384; 

    uint8_t quadrant = 0;       // 1: [0 - 90], 2: [90 - 180], 3: [0 - (-90)], 4: [(-90) - (-180)]
 
    uint8_t move_to_sine = 0;   /* Has value:
                                - 2 when the position needs to be moved into the sine wave and no quadrants have been complete
                                - 1 when the position needs to be moved into the sine wave and one quadrant has been complete
                                - 0 when the position no longer needs to be moved into the sine wave (the 2 quadrants have been complete and the current position is now at either the peak or min)
                                The purpose of this is to smoothly move the starting value into the sine wave. A temporary amplitude is used for the first half-cycle, after which the target
                                value will be either the peak or minimum of the sine wave and the amplitude is set to the value in the amp input field.*/

  
    uint64_t elapsed_time = 0;


    /**
     * @brief Initializes Signal_Parameters for a sine wave signal. These include the starting value, amplitude, offset from zero, period, and start time.
     *        A temporary amplitude is calculated for the first half-cycle so that the starting value can smoothly transition from its current position to the sine wave's max or min.
    */
    void init(uint32_t init_value, uint32_t max, int32_t min, float _frequency){

        // Update current value
        target_value = init_value;

        // Update start value
        start_value = init_value;

        // Set start time
        start_time = millis() * 1000;

        // Set the # of microseconds it should take to complete one wave cycle (user enters value in milliseconds)
        period = 1000. / _frequency;
        //IC4_virtual->print_l("\n\r period calculated");
        //IC4_virtual->print_l(String(period).c_str());
        // Get amplitude
        amplitude = (max-min)/2;
        target_1 = amplitude;

        // Get offset
        target_2 = min;


        // Update target_1 with temporary amplitude to update current target_value until it has moved to either the sine wave's max or min, after which the amplitude gets swapped with the user requested one

        // Init move_to_sine: so run() completes the half-cycle required to smoothly move the target from its starting value to the sine waves max or min value. 
        move_to_sine = 2; 

        // Check if target value is at or below middle of sine wave
        if(start_value <= (target_1 + target_2)){

            // Set a temporary amplitude that is 1/2 the distance between the sine wave's peak and the target value (this will be used for 2 quadrants, until the current position is at the peak)
            lut_index = -16384;
            quadrant = 4;

            // Set temporary amplitude (target_1) to get from current position to wave peak
            if(target_2 < start_value){
                // Current position is above wave minimum
                target_1 = target_1 - ((start_value - target_2)/2);
            }
            else{
                // Current position is below sine wave minimum
                target_1 = target_1 + ((target_2 - start_value)/2);
            }
            
        }
        // Check if current position is above middle of sine wave
        else if(start_value > (target_1 + target_2)){

            // Set a temporary amplitude that is 1/2 the distance between the the current position and the minimum of the sine wave (this will be used for 2 quadrants, until the current position is at the minimum)
            lut_index = 16384;
            quadrant = 2;
            target_1 = (target_value - target_2) / 2;         
        }

    }



    
    
    /**
     * @brief Generates and returns the next value in the sine wave using a lookup table. Value depends on the signal parameters and the time elapsed since signal initialization.
    */
    int32_t run(){

        // Get sine value from index, + 65535 to prevent negative position values
        target_value = 65535 + lut(lut_index); 

        // Scale to amplitude
        target_value = (target_value * ((float)target_1 / (float)65535));

        // Add offset
        if(move_to_sine && (quadrant==1 || quadrant==4)){
            // if the current target value needs to move upwards towards the sine wave range over the first half-period, the offset must temporarily be equal to the starting value 
            target_value += start_value;
        }
        else{
            // Regular offset
            target_value += target_2;
        }
        
        // get elapsed time since start of quadrant
        //uint32_t 
        elapsed_time = (uint64_t)(millis()*1000 - start_time);


        // Update index
        switch (quadrant) {
        case 1: 
            // lut index runs through 0 to 16384 each quarter period
            lut_index = (16384 * elapsed_time / (250 * period));
            //lut_index = (16384 * elapsed_time * 4/period);
            break;
        case 2:
            // lut index runs through 16384 to 32767 each quarter period
            lut_index = 16384 + (16384 * elapsed_time / (250 * period));
            //lut_index = 16384 + (16384 * elapsed_time * 4 / period);
            break;
        case 3:
            // lut index runs through -32768 to -16384 each quarter period
            lut_index = -32768 + (16384 * elapsed_time / (250 * period));
            //lut_index = -32768 + (16384 * elapsed_time * 4 / period);
            break;
        case 4:
            // lut index runs through -16384 to 0 each quarter period
            lut_index = -16384 + (16384 * elapsed_time / (250 * period));
            //lut_index = -16384 + (16384 * elapsed_time * 4 / period);
            break;
        }

        // Update quadrant each quarter period
        if(elapsed_time >= (250 * period)){
       // if(elapsed_time >= (period/4)){
            // Reset start time for new quadrant
            start_time = millis() * 1000;
            // Check if target position needed to move towards sine wave and if that position has been reached
            if(move_to_sine==1){
                // Position reached, reset amplitude to user requested amplitude for wave
                target_1 = amplitude;
            }

            if(move_to_sine){
                // Decrement `move_to_sine` for each of the two first quadrants that are complete (value will be 0 once the position target has finished moving towards sine wave)
                move_to_sine--;
            }

            // Update quadrant
            if(quadrant == 4){
                quadrant = 1;
            }
            else {
                quadrant++;
            }
        }

        // Return target value
        return target_value;

    }

    int32_t get_value() {
        return target_value;
    }


    

  protected:

    
    /**
     * @brief Takes a signed 16-bit value arg as an argument, and returns the value sin(arg) using the lookup table sine_table, which stores 1025 values for one quadrant.
     * @param[in] arg: some value in [-32768, 32767]
     * @return sin(arg) represented as a signed 32-bit value in the range [-65535, 65535]
    */
    int32_t lut(short arg){

        uint16_t index1 = 0;
        uint16_t index2 = 0;

        uint8_t i_bits = 4;
        const int i_mask = ((1 << i_bits) - 1);
        
        int32_t value1 = 0;
        int32_t value2 = 0;
        
        int32_t returnValue = 0;

        // Determine which quadrant arg is in
        switch(arg & 0xC000){
            
            case 0x8000:
                // quadrant 4: (-180 <= arg <= -90), index = arg + 180 and returnValue is negative
                index1 = ((1<<15) + arg);
                index1 = index1 >> i_bits;
                index2 = index1 + 1;
                value1 = sine_table[index1];
                value2 = sine_table[index2];
                returnValue = -(value1 + (value2 - value1) * (arg & i_mask) / (1<<i_bits));
                
                break;
                
            case 0xC000:
                // quadrant 3: (-90 <= arg <= 0), index = -arg and returnValue is negative
                index1 = (- arg);
                index1 = index1 >> i_bits;
                index2 = index1 + 1;
                value1 = sine_table[index1]; 
                value2 = sine_table[index2];
                returnValue = -(value1 + (value2 - value1) * (arg & i_mask) / (1<<i_bits));
                break;
                
            case 0x0000:
                // quadrant 1: (0 <= arg <= 90), index = arg
                index1 = arg;
                index1 = index1 >> i_bits;
                index2 = index1 + 1;
                value1 = sine_table[index1]; 
                value2 = sine_table[index2];
                returnValue = (value1 + (value2 - value1) * (arg & i_mask) / (1<<i_bits));
                break;
                
            case 0x4000:
                // quadrant 2: (90 <= arg <= 180), index = 180 - arg
                index1 = ((1<<15) - arg);
                index1 = index1 >> i_bits;
                index2 = index1 + 1;
                value1 = sine_table[index1];
                value2 = sine_table[index2];
                returnValue = (value1 + (value2 - value1) * (arg & i_mask) / (1<<i_bits));
                break;
        }
        
        return returnValue;
    }


};
