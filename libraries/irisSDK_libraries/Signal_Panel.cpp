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


#include "Signal_Panel.h"


/**
 * @brief Function initializes all signal panel elements and adds them to a new GUI_Page object. The init_value is initialized with an internal int32_t pointer with a constant value of zero.
*/
void Signal_Panel::add(uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units){
	panel_elements = new GUI_Page();
	add(panel_elements, _y_anchor, _x_anchor, _units);
}


/**
 * @brief Function initializes all signal panel elements and adds them to the GUI_Page object specified. The init_value is initialized with an internal int32_t pointer with a constant value of zero.
*/
void Signal_Panel::add(GUI_Page * _panel_elements, uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units){
	init_value = &default_init_value;
	signal_generator.set_init_value(&default_init_value);
	panel_elements = _panel_elements;
	x_anchor = _x_anchor;
	y_anchor = _y_anchor;
	setup((char *) _units);
}



/**
 * @brief Function initializes all signal panel elements and adds them to a new GUI_Page object. The init_value is initialized with the int32_t pointer parameter.
*/
void Signal_Panel::add(int32_t* _init_value, uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units){
	panel_elements = new GUI_Page();
	add(panel_elements, _init_value, _y_anchor, _x_anchor, _units);
}


/**
 * @brief Function initializes all signal panel elements and adds them to the GUI_Page object specified. The init_value is initialized with the int32_t pointer parameter.
*/
void Signal_Panel::add(GUI_Page * _panel_elements, int32_t * _init_value, uint8_t _y_anchor, uint8_t  _x_anchor, const char * _units){
	panel_elements = _panel_elements;
	init_value = _init_value;
	signal_generator.set_init_value(_init_value);
	x_anchor = _x_anchor;
	y_anchor = _y_anchor;
	setup((char *) _units);
}


/**
 * @brief Initializes all signal input elements and signal GUI_Pages with the elements required for each signal. After first initializing, only the "Select Signal" label and the slider are displayed.
*/
void Signal_Panel::setup(char * units) {

	// Make panel labels using provided units
	make_label(target_label, (char *) "target", units);
	make_label(min_label, (char *) "min", units);
	make_label(max_label, (char *) "max", units);
	//make_label(amp_label, (char *) "amp", units);
	//make_label(offset_label, (char *) "offset", units);

	signal_generator.init();

	// Initialize GUI_Page
	panel_elements->add();

	// Initialize update/start signal button
	start_signal_btn.add(panel_elements, "Start Signal", -1, y_anchor, x_anchor + 5, 2, 4);

	// Initialize pause button
	pause_signal_btn.add(panel_elements, "Pause Signal", -1, y_anchor, x_anchor, 2, 4);
	
	// Add slider for selecting signal type
	signal_slider.add(panel_elements, "      None", y_anchor+3, x_anchor, 1, 15, 0, 4, 0, 1, FlexData::ALLOW_INPUT);
	last_slider_value = 0;

	// Initialize and hide the three input fields
	input1.add(panel_elements, "target(*mu*m)", y_anchor + 5, x_anchor, 1, 8, 0, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7);
	input2.add(panel_elements, "period(ms)", y_anchor + 7, x_anchor, 1, 8, 0, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7);
	input3.add(panel_elements, "period(ms)", y_anchor + 9, x_anchor, 1, 8, 0, 1, FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::DIGITS_7);

	update_panel_fields(None);
}


/** @brief Function updates label by concatenating the label prefix with units in brackets. (i.e. if label_prefix is "target" and units are "mN", label will be "target(mN)")*/
void Signal_Panel::make_label(char * label, char * label_prefix, char * units){

	// Label pointer index
	int p1;

	// Copy label prefix to label starting at first address in label
	for(p1 = 0; label_prefix[p1] != '\0'; p1++){
		label[p1] = label_prefix[p1];
	}

	// Append opening bracket
	label[p1++] = '(';

	// Copy units to label starting at last address in label
	for(int p2 = 0; units[p2] != '\0'; p1++, p2++){
		label[p1] = units[p2];
	}

	// Append closing bracket
	label[p1++] = ')';

	// Append null terminator char
	label[p1] = '\0';
} 


/**
 * @brief Initializes the specified signal if the "start signal" button is pressed. Continues to return values for this signal until another signal is specified.
*/
void Signal_Panel::run() {

	// If slider value changed, update displayed panel fields
	if(slider_change()) update_panel_fields(signal_slider.get());

	// If pause button was pressed, put motor into sleep mode
	if( pause_signal_btn.pressed() ){
		pause();
	}

	// If start button is pressed, initialize signal with the parameters specified in input fields
	if (start_signal_btn.pressed()) {
		signal_generator.signal_type = signal_slider.get();
		signal_generator.signal_paused = false;
		switch (signal_generator.signal_type) {
			case None:
				break;
			case Ramp:
				// Initialize ramp signal
				if(input2.get()==0){
					// use default value if 0 was entered for period
					input2.update(default_period);
				}
				signal_generator.ramp_signal.init(*init_value, input1.get(), input2.get());
				break;
			case Square:
				if(input3.get()==0){
					// use default value if 0 was entered for period
					input3.update(default_period);
				}
				// Initialize square signal
				frequency =  1000. / (float)input3.get();
				signal_generator.square_wave.init(*init_value, input1.get(), input2.get(), frequency);
				
				break;
			case Triangle:
				// Initialize triangle signal
				if(input3.get()==0){
					// use default value if 0 was entered for period
					input3.update(default_period);
				}
				frequency =  1000. / (float)input3.get(); 

				
				signal_generator.triangle_wave.init(*init_value, input1.get(), input2.get(), frequency);
				break;
			case Sine:
				// Initialize sine wave signal
				if(input3.get()==0){
					// use default value if 0 was entered for period
					input3.update(default_period);
				}
				frequency =  1000. / (float)input3.get();
				IC4_virtual->print_l("\rn frequency going into generator");
				IC4_virtual->print_l(String(frequency).c_str());
				signal_generator.sine_wave.init(*init_value, input1.get(), input2.get(), frequency);
				break;
		}
	}
}


/**
 * @brief Returns true if the panel slider has been updated sine the last time this function has been called, otherwise false.
*/
bool Signal_Panel::slider_change() {
	bool slider_change = false;		// Return value

	// Check if slider value has changed
	if (last_slider_value != signal_slider.get()) {

		// Update return value
		slider_change = true;

		// Update last slider value
		last_slider_value = signal_slider.get();
	}
	return slider_change;
}


/**
 * @brief Updates the panel's displayed input fields to suit the signal type parameter specified.
 */
void Signal_Panel::update_panel_fields(int slider_value) {

	// Clear data from input fields
	clear_inputs();
	
	// Add / remove elements from GUI_Page object based on the panel slider value
	switch (slider_value) {
	case None:
		panel_elements->hide();
		panel_elements->remove_element(&input1);
		panel_elements->remove_element(&input2);
		panel_elements->remove_element(&input3);
		panel_elements->remove_element(&start_signal_btn);
		signal_slider.rename("      None");
		panel_elements->show();
		break;
	case Ramp:
		panel_elements->hide();
		panel_elements->add_element(&input1);
		panel_elements->add_element(&input2);
		panel_elements->remove_element(&input3);
		panel_elements->add_element(&start_signal_btn);
		input1.rename((const char*)target_label);
		input2.rename("period(ms)");
		signal_slider.rename("      Ramp");
		panel_elements->show();
		break;
	case Square:
		panel_elements->hide();
		panel_elements->add_element(&input1);
		panel_elements->add_element(&input2);
		panel_elements->add_element(&input3);
		panel_elements->add_element(&start_signal_btn);
		input1.rename(max_label);
		input2.rename(min_label);
		signal_slider.rename("    Square");
		panel_elements->show();
		break;
	case Triangle:
		panel_elements->hide();
		panel_elements->add_element(&input1);
		panel_elements->add_element(&input2);
		panel_elements->add_element(&input3);
		panel_elements->add_element(&start_signal_btn);
		input1.rename(max_label);
		input2.rename(min_label);
		signal_slider.rename("  Triangle");
		panel_elements->show();
		break;
	case Sine:
		panel_elements->hide();
		panel_elements->add_element(&input1);
		panel_elements->add_element(&input2);
		panel_elements->add_element(&input3);
		panel_elements->add_element(&start_signal_btn);
		//input1.rename(amp_label);
		//input2.rename(offset_label);
		input1.rename(max_label);
		input2.rename(min_label);
		signal_slider.rename("      Sine"); 
		panel_elements->show();
		break;
	}
}


/**
 * @brief Clears data from input field elements
*/
void Signal_Panel::clear_inputs() {
	input1.update(0);
	input2.update(0);
	input3.update(0);
}


/**
 * @brief Stops the panel from generating new signal values.
*/
void Signal_Panel::pause() {
	signal_generator.signal_type = None;
	signal_generator.signal_paused = true;
}

/**
 * @brief Returns the target value.
 */
int32_t Signal_Panel::get_target_value() {
	return signal_generator.get_target_value();
}


/**
* @brief Hides all signal panel elements
*/
void Signal_Panel::hide() {
	panel_elements->hide();
}


/**
 * @brief Shows the slider and "Select signal" label elements
*/
void Signal_Panel::show() {
	panel_elements->show();
	update_panel_fields(signal_slider.get());
}