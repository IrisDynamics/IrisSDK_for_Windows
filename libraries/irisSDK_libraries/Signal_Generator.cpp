/**
   @file Signal_Panel.h
   @brief A GUI interface for the Signals library
   @author Michelle Aleman <maleman@irisdynamics.com>
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


#include "Signal_Generator.h"


void Signal_Generator::init() {

	// Init values
	target_value = 0;
	signal_paused = false;
	signal_type = None;
}
void Signal_Generator::set_init_value(int32_t* val ) {
	init_value = val;
}
/**
 * @brief Generates and returns the next target value in the signal. If none, the initial value is returned.
 */
int32_t Signal_Generator::run() {
	if (signal_paused) return target_value;
	else {
		// Get next target_position value by calling the signal run function
		switch (signal_type) {
		case None:
			target_value = *init_value;
			break;
		case Ramp:
			target_value = ramp_signal.run();
			break;
		case Square:
			target_value = square_wave.run();
			break;
		case Triangle:
			target_value = triangle_wave.run();
			break;
		case Sine:
			target_value = sine_wave.run();
			break;
		}
	}

	return target_value;
}

/**
 * @brief Returns the target value.
 */
int32_t Signal_Generator::get_target_value() {
	return target_value;
}


