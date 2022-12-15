/**
 *  @file mb_crc.cpp
 *
 *  Created on: Aug. 3, 2021
 *  @author: Sean
    
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

//#ifdef __MK20DX256__
//#else
#include "mb_crc.h"
constexpr uint8_t ModbusCRC::crc_hi_table[256];
constexpr uint8_t ModbusCRC::crc_lo_table[256];

ModbusCRC mod_crc;
//#endif