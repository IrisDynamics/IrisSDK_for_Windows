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

#include "../ic4_library/iriscontrols4.h"
#include <string.h>
#include "Signal_Generator.h"



/**
   @file Signal_Panel.h
   @class Signal_Panel
   @brief Interface for generating signals. A slider can be used to display input fields for either a ramp, square wave, triangle wave, sine wave, or no signal.
   A start button can be pressed to generate the specified signal's values, and a pause button can be used to stop generating these values.
*/
class Signal_Panel {
public:
	GUI_Page* panel_elements;						// GUI_Page object for showing/hiding panel elements

	FlexSlider signal_slider;						// Slider whose value determines the value of signal_type: 0-None, 1-Ramp, 2-Square Wave, 3-Triangle Wave, 4-Sine Wave
	FlexLabel signal_label;							// Signal label
	FlexData input1, input2, input3; 				// Input fields for signal parameters
	FlexButton start_signal_btn, pause_signal_btn; 	// Start and pause signal buttons

	uint8_t last_slider_value;						// Last slider value, used to determine when the slider has been updated by user

	float frequency; 								// Given the value (1 / user input period) before initializing a signal panel
	uint16_t default_period = 1000;

	uint8_t y_anchor;								// Panel y anchor
	uint8_t x_anchor;								// Panel x anchor

	// Panel input field labels that are initialized with the units passed to the add() function
	char target_label[20];
	char min_label[20];
	char max_label[20];
	char amp_label[20];
	char offset_label[20];
	int32_t* init_value;							// Address of value used for initializing signals
	int32_t default_init_value = 0;					// Address that init_value points to if an alternatvie one is not provided

	

	Signal_Generator& signal_generator;				// Manages and runs the signals


	Signal_Panel(Signal_Generator& _signal_generator) :
		signal_generator(_signal_generator)
	{}


	// Init functions
	void add(uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units="*mu*m");
	void add(GUI_Page * _panel_elements, uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units="*mu*m");
	void add(int32_t* _init_value, uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units="*mu*m");
	void add(GUI_Page * _panel_elements, int32_t* _init_value, uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units="*mu*m");

	void setup(char * units);											// Called by add() to initialize page elements
	void make_label(char * label, char * label_prefix, char * units);	// called by setup() to initialize input field labels

	void run();										// Updates panel in GUI and initializes signal when 'Start Signal' button is pressed
	bool slider_change();							// Called by run_gui() to determine if the slider value has changed since the last time this function was called
	void update_panel_fields(int slider_value);		// Called by run_gui() to update the displayed input fields to match the given slider value
	void clear_inputs();							// Called by run_gui() to clear all input field values
	void pause();									// Called by run_gui() to pauses signal generation when 'Pause' button pressed
	int32_t get_target_value();						// Returns the generated signal value
	void hide();									// Hides panel in GUI
	void show();									// Displays panel in GUI

};
