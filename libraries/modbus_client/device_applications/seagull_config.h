/**
 * @file seagull_config.h
 * @author Sean Jeffery <sjeffery@irisdynamics.com>
 * @brief Contains configuration information used in Seagull objects
    
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
#include "../mb_config.h"

#include "../device_drivers/atmega328/modbus_client_328.h"

//#include "orca600_memory_map.h"
#include "../../orca600_api/orca600.h"

// Local storage registers
#define ILOOP_REG_SIZE            4

#define ILOOP_CH1_OFFSET          0
#define ILOOP_CH2_OFFSET          1
#define ILOOP_IN_OFFSET           2
#define ERROR_REG_OFFSET          3

// For writing to Orca
#define CLEAR_ERROR_REG_OFFSET    CTRL_REG_0
#define CLEAR_ERROR_MASK          1<<1


