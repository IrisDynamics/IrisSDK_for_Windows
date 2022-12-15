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

int IrisControls4::parse_console_msg(char * cmd, char * args){
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

//	COMMAND_IS "element_size" THEN_DO
//
//		print_l("Element Sizes:");
//		print_l("\rFlexButton: ");
//		print_d(sizeof(FlexButton));
//		print_l(" bytes\rFlexSlider:    	");
//		print_d(sizeof(FlexSlider));
//		print_l(" bytes\rBasic FlexSlider:	");
//		print_d(sizeof(Basic_FlexSlider));
//		print_l(" bytes\rFlexLabel:     	");
//		print_d(sizeof(FlexLabel));
//		print_l(" bytes\rC_FlexLabel:   	");
//		print_d(sizeof(C_FlexLabel));
//		print_l(" bytes\rFlexData:     		");
//		print_d(sizeof(FlexData));
//		print_l(" bytes\rBasic FlexData:    ");
//		print_d(sizeof(Basic_FlexData));
//		print_l(" bytes\rFlexPlot:      	");
//		print_d(sizeof(FlexPlot));
//		print_l(" bytes\rDataset:       	");
//		print_d(sizeof(Dataset));
//		print_l(" bytes\r");

	COMMAND_IS "help" THEN_DO
		IC4_virtual->print_l("\rIC4 commands:\r");
		IC4_virtual->print_l(command_list.c_str());
		return 0;

	FINISH_PARSING

}


