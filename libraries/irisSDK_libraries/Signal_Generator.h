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

#pragma once

#include "Ramp_Signal.h"
#include "Square_Wave_Signal.h"
#include "Triangle_Wave_Signal.h"
#include "Sine_Wave_Signal.h"


/*
 * enum specifies a signal type: 0-None, 1-Ramp, 2-Square wave, 3-Triangle wave, 4-Sine wave
*/
enum SignalType {
	None = 0,
	Ramp = 1,
	Square = 2,
	Triangle = 3,
	Sine = 4
};


/**
   @file Signal_Panel.h
   @class Signal_Panel
   @brief Interface for generating signals. A slider can be used to display input fields for either a ramp, square wave, triangle wave, sine wave, or no signal.
   A start button can be pressed to generate the specified signal's values, and a pause button can be used to stop generating these values.
*/
class Signal_Generator {
public:
	int32_t target_value;							// value to be continuously updated with generated signal values

	// Signal objects for generating signal values
	RampSignal ramp_signal;
	SquareWave square_wave;
	TriangleWave triangle_wave;
	SineWave sine_wave;

	int32_t* init_value;							// Address of value used for initializing signals
	bool signal_paused = true;								// Boolean value indicating whether the signal is running or paused.
	int signal_type = None;								// Value represents a signal type: 0-None, 1-Ramp, 2-Square Wave, 3-Triangle Wave, 4-Sine Wave

	void init();
	void set_init_value(int32_t* val);
	int32_t run();									// Generates and returns the signal value (should be called more frequently than the GUI update period to generate enough values for a smooth signal)
	int32_t get_target_value();						// Returns the generated signal value
};
