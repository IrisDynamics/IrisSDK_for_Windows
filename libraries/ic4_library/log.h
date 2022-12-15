/**
 * @file log.h
 * @author  Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 *
 * @brief
 *	Class definition for IrisControls-based log files
 
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
#include "iriscontrols4.h"

class DataLog {
public:

	DataLog() {
		my_id = id_assigner++;
	}
	~DataLog(  ) {
	}

	void add (const char * name);
	void write (const char * string);

protected:
		static int id_assigner;
		int my_id;

};

