/**
 * @file device_config.h
 * @author Rebecca McWilliam <mcwilliamrm@irisdynamics.com>
 * @version 1.1
 * @brief Contains configuration information used to build and run objects based on the device using the library
    
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
//#define WINDOWS
#include "windows.h"
#ifdef IRIS_ZYNQ_7000
#define millis() 0
#elif defined(__MK20DX256__)
#elif defined(WINDOWS)
#define millis() GetTickCount()
#define micros() GetTicks_micro()//(millis()*1000)
#define String(x)   std::to_string(x)    
#define concat(x)   append(x)
static inline int64_t GetTicks_micro()
{
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
    {
    }
    return ticks.QuadPart/ 10;
}



#elif defined(QT_WINDOWS)
#else
#error select plaftform in this file
#endif
