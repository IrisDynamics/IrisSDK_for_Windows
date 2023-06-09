/**
 * @file console_message_parser.cpp
 * @author Dan Beddoes
 * @version 2.2.0
 * created May 24, 2021
 *  This function parses console messages that are not App or hardware specific.
 *  Commands are parsed through the IC4 API first, then this parser, and finally to any App parser.
    
    Copyright 2022 Iris Dynamics Ltd 
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

#include "iriscontrols4.h"

int IrisControls4::parse_console_msg(char * cmd) {

	std::string command_list;
#define SERIAL_COMMAND(_ARG_)  									\
	COMMAND_IS (  (command_list += (std::string)_ARG_ + (std::string)"\r\n").capacity()  ? _ARG_: _ARG_    ) THEN_DO 	\

	START_PARSING

//	COMMAND_IS "crc" THEN_DO
//
//		device_state_check();

	COMMAND_IS "IC4_error" THEN_DO

		print_l("tx_buffer_overflow: "); print_d(errors.tx_buffer_overflow); print_l("\r");
		print_l("rx_buffer_overflow: "); print_d(errors.rx_buffer_overflow); print_l("\r");
		print_l("dropped_gui_frames: "); print_d(errors.dropped_gui_frames); print_l("\r");

	SERIAL_COMMAND ("version")

		print_l("\rSerial API Version: ");
		print_l(serial_api_version);
		print_c('\r');

	SERIAL_COMMAND ("disconnect")
		disconnect();

	COMMAND_IS "element_size" THEN_DO

		PRINTL ("Element Sizes (bytes):");
		PRINTDL("FlexButton:		"	, sizeof(FlexButton));
		PRINTDL("FlexSlider:    	"	, sizeof(FlexSlider));
		PRINTDL("Basic FlexSlider:	"	, sizeof(Basic_FlexSlider));
		PRINTDL("FlexLabel:     	"	, sizeof(FlexLabel));
		PRINTDL("C_FlexLabel:   	"	, sizeof(C_FlexLabel));
		PRINTDL("FlexData:     		"	, sizeof(FlexData));
		PRINTDL("Basic FlexData:    "	, sizeof(Basic_FlexData));
		PRINTDL("FlexPlot:      	"	, sizeof(FlexPlot));
		PRINTDL("FlexDropdown:     	"	, sizeof(FlexDropdown));
		PRINTDL("MenuOption:     	"	, sizeof(MenuOption));
		PRINTDL("Dataset:       	"	, sizeof(Dataset));

	COMMAND_IS "help" THEN_DO
		IC4_virtual->print_l("\rIC4 commands:\r");
		IC4_virtual->print_l(command_list.c_str());
		return 0;

	FINISH_PARSING

}


