/**
 * @file actuator_config.h
 * @author Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 * @brief Contains configuration information used to construct the Actuator objects
    
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

#ifdef IRIS_ZYNQ_7000
#include "../device_drivers/zynq7000/zynq7000_modbus_client.h"
#elif defined(__MK20DX256__)
#include "../device_drivers/k20/modbus_client_k20.h"
#elif defined(WINDOWS)
#include "../device_drivers/windows/windows_modbus_client.h"
#elif defined(QT_WINDOWS)
#include "qt_modbus_client.h"
#endif


//#include "orca600_memory_map.h"
#include "../../orca600_api/orca600.h"


#define POS_CTRL                  true


#define POS_REG_OFFSET            SHAFT_POS_UM
#define POS_REG_H_OFFSET          SHAFT_POSITION_H

#define FORCE_REG_OFFSET          FORCE
#define FORCE_REG_H_OFFSET        FORCE_H

#define POWER_REG_OFFSET          POWER

#define TEMP_REG_OFFSET           STATOR_TEMP


#define VOLTAGE_REG_OFFSET        VDD_FINAL

#define ERROR_REG_OFFSET          ERROR_0

#define ZERO_POS_REG_OFFSET       CTRL_REG_0

#define CLEAR_ERROR_REG_OFFSET    CTRL_REG_0

#define ZERO_POS_MASK             1<<2

#define CLEAR_ERROR_MASK          1<<1


