/**
 * @file log.cpp
 * @author  Kyle Hagen <khagen@irisdynamics.com>
 * @version 2.2.0
	
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

#include "log.h"

int DataLog::id_assigner = 0;

/**
 * @brief Add a new DataLog
 */
void DataLog::add (const char * name) { IC4_virtual->datalog_add(my_id, name); }

/**
 * @brief Writes to an existing DataLog
 */
void DataLog::write  (const char * string) {	IC4_virtual->datalog_write 			(my_id, string); 	}

