/**
   @file Signal_Parameters.h
   @class Signal_Parameters
   @brief Contains signal parameter variables used for initializing each of the signal classes.
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

#include "device_config.h"
#pragma once

class Signal_Parameters {
    protected:
    int32_t start_value;    // Start value
    int32_t target_value;   // Current value
    
    int32_t target_1;       // Target value, or peak of two target values
    int32_t target_2;       // Min target value for oscillating signals 

    uint64_t start_time;    // Time the signal is initialized
    uint32_t period;        // Period of signal
};
