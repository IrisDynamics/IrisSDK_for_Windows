/**
 * @file iriscontrols.cpp
 * @author  Kyle Hagen <khagen@irisdynamics.com>
 * @version 2.2.0
 * @brief static definitions and implementations of the IrisControls object members
    
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
#pragma warning(disable : 4996).

#include "iriscontrols4.h"

	uint8_t CRC::table[256];
	int 	CRC::is_built = 0;

 /**
  * @brief 
  *
  * Starts the transmitter when it is idle and there is data in the tx_buffer
  * Removes all pending messages from the rx buffer by monitoring the pending_incoming_messages counter
  *
  * Returns the number of pending messages parsed
  */


int IrisControls4::check() {

	//poll interchar timer and clear receive buffer if timer exceeded
	if (is_disconnected() && is_interchar_timer_expired()) {
		receive_buffer.clear();
		pending_incoming_messages = 0;
	}

	//poll the hardware receiver (if implemented by platform)
	receive();

	//enter timeout condition if timer expires while connected and no pending incoming messages
	if( is_connected()
			&&
		!pending_incoming_messages
			&&
		(u32)(system_time() - time_last_message) > (timeout_timer * 1000)
			&&
		!was_timed_out)
	{
		connection_status = timed_out;
		was_timed_out	= 1;
	}

	// Parse message - empty receive buffer
	u32 ret = pending_incoming_messages;
	while (pending_incoming_messages) {
		get_message		(command, argument);
		parse_message	(command, argument);
	}

	return ret;
}

 /*
  * @brief parses IC4 messages
  * 
  * @return 0 when the message was not parsed by the API parser (ie the message should be passed along to subsequent parsers), and 1 when the message was parsed
  */
int IrisControls4::parseAPI(char* command, char* arg) {
	int command_id;

	if (command[0] == '\x02') {
		command_id = int(command[1]);
	}	
	else {
		if (command[0] == '\0')	
			return 1;	// ie we've "parsed" an empty command, or a command that was only the line return
		else 
			return 0;	// if the command character was not the IC4 command start character, this parser returns 0 to let other parsers check the message 
	}
	

	switch (command_id) {

		unsigned int index;
		int id;
		int value;

		case EOT_CHAR:			
			handle_eot();				
			return 1;

		case NAME_ENQUIRY: //name query
			enquiryResponse();
			return 1;

		case HANDSHAKE: {//challenge	
			u64 now = system_time();
	#ifdef BYTE_STUFFING_PARSING
			build_crc_data((u16)IC4_virtual->byte_stuffing);
	#endif
	#ifdef MESSAGE_LENGTH_PARSING
			build_crc_data((u16)IC4_virtual->no_byte_stuffing);
	#endif
			build_crc_data((u16)IC4_virtual->communication_protocol);
			build_crc_data(IC4_virtual->device_id);
			build_crc_data(IC4_virtual->serial_api_version);
			build_crc_data(IC4_virtual->server_name);
			build_crc_data(now);
			build_crc_data(timeout_timer);

			handshakeResponse(now, CRC::generate(crc_data, crc_index));
			crc_index = 0;
			return 1;			
		}
		case BEGIN_CONNECTION:
			IO_registry::reset_active_list();
			connection_status = connected;
			print_l("IrisControls4: connected!!!\r");
			return 1;

		case END_CONNECTION:
			set_disconnected();
			return 1;

		case FLEXELEMENT_PRESSED:
//			print_l("Button pressed \r");
			index = 0;
			id = parse_int(arg, index);
			IO_registry::set_element_pressed(id);
			element_press_received = 1;
			return 1;

		case BUTTON_TOGGLED:
//			print_l("Button toggled \r");
			index 		= 0;
			id 			= parse_int(arg, index);
			value 		= parse_int(arg, index);
			IO_registry::set(id, value);
			element_press_received = 1;
			return 1;

		case FLEXELEMENT_UPDATED: {
//			print_l("Element updated \r");
			index 	= 0;
			id 		= parse_int(arg, index);
			value 	= parse_int(arg, index);
			element_value_received = 1;
//			print_d(value); print_l("\r");
			IO_registry::set(id, value);
			return 1;
		}

		case GENERIC_MESSAGE: {
			index = 0;
			unsigned int messageCode;
			messageCode = parse_int(arg, index);

			switch (messageCode) {

				case GEOMETRY_QUERY: {
					//print_l("Max rows/cols:");
					u16 rows = parse_int(arg, index);
					u16 columns = parse_int(arg, index);
					//print_d(rows);
					//print_c(' ');
					//print_d(columns);
					//print_c('\r');

					max_rows = rows;
					max_cols = columns;

					return 1;
				}
				default:
					return 0;
			}
		}

		case ERR_HANDLING: {
			unsigned int errorCode;
			index = 0;
			errorCode = parse_int(arg, index);

			//print_l("Error!\r");

			switch (errorCode) {

				case ERROR_COLLISION: {
//					print_l("Error: Element Collision:");
//					int _id = parseInt(arg, index);
//					id 		= parseInt(arg, index);
//					IO_thing * id_thing 	= IO_registry::get(id);
//					IO_thing * _id_thing 	= IO_registry::get(_id);
//
//					print_l("\rTried to add \"");
//					if( id_thing) 	{ print_l(id_thing->name); } 	else { print_l("id: "); print_d(id); 	}
//					print_l("\" to the grid, but \"");
//					if( _id_thing) 	{ print_l(_id_thing->name); } 	else { print_l("id: "); print_d(_id); 	}
//					print_l("\" was in the way.\r");

					return 1;
				}

				case ERROR_OUT_OF_BOUNDS: {
//					print_l("Error: Element Out of Bounds:");
//					id = parseInt(arg, index);
//					IO_thing * id_thing = IO_registry::get(id);
//
//					print_l("\rTried to add \"");
//					if( id_thing) 	{ print_l(id_thing->name); } 	else { print_l("id: "); print_d(id); 	}
//					print_l("\" to the grid, but it would have been out of bounds\r");

					return 1;
				}

				case ERROR_INVALID_SIZE: {
//					print_l("Error: Invalid Size:");
//					id = parseInt(arg, index);
//					int min_h = parseInt(arg, index);
//					int min_w = parseInt(arg, index);
//					int alloted_h = parseInt(arg, index);
//					int alloted_w = parseInt(arg, index);
//
//					IO_thing * id_thing = IO_registry::get(id);
//
//					print_l("\rTried to add \"");
//					if( id_thing) 	{ print_l(id_thing->name); } 	else { print_l("id: "); print_d(id); 	}
//					print_l("\" to the grid with an invalid size.\r Minimum height/width: ");
//					print_d(min_h);
//					print_l("/");
//					print_d(min_w);
//					print_l(" Alloted height/width: ");
//					print_d(alloted_h);
//					print_l("/");
//					print_d(alloted_w);
//					print_l(" pixels.\r");

					return 1;
				}
				default:
					return 0;
			}

		default:
			return 0;
		}

	}
	return 0;
}



/**
 * Expects space-delimited integer arguments
 * Not explicit error checking
 * Looks down the input string starting at the index, and converts the
 * text to an int.
 * Increments the index reference according to how many characters were read
 * 
 * Usage:
 * 		uint index = 0;
 *		int i = parseInt ( arg, index );
 *		int j = parseInt ( arg, index );
 *		int k = parseInt ( arg, index );
 *		xil_printf( "output: %d, %d, %d\r" , i, j, k );
 *
 */
int IrisControls4::parse_int(char* input, unsigned int& index) {

	int charsread;
	int output = 0;
	while (input[index] == ' ') index++;
	if (input[index] == '\0') return 0;		// cover special case when parseInt was called on an emtpy (or all-whitespace) string. 
	if (sscanf(input + index, "%d%n", &output, &charsread))
		index += charsread;
	return output;
}


double IrisControls4::parse_double(char* input, unsigned int& index) {
	int charsread;
	double output = 0.0;
	while (input[index] == ' ') index++;
	if (input[index] == '\0') return 0;		// cover special case when parseInt was called on an empty (or all-whitespace) string.
	if (sscanf(input + index, "%lf%n", &output, &charsread))
		index += charsread;
	return output;
}


 /**
  * @brief parse a message, which contains a command string, and optionally an argument string.
  */
int IrisControls4::parse_message(char* cmd, char* arg) {
	int ret = parseAPI				( cmd, arg );	//IC4 API parser
		ret |= parse_app			( cmd, arg );	//Application layer parser
		ret |= parse_console_msg	( cmd, arg );	//IC4 Console Message Parser
		ret |= parse_device_driver	( cmd, arg );	//Hardware Specific Parser

	if (!ret) print_help(cmd, arg);

	return ret;
}


/**
 * @brief add a character (probably incoming coming from a UART) to the rx_buffer
 *
 * This function watches for the line-return character; when one is detected, a counter tracking the number of unparsed incoming messages is increased
 * This function would typically be called by a UART rx interrupt, or by a function that polls the uart for received character(s)
 * Increments the pending_incoming_messages counter
 */
void IrisControls4::receive_char(char c) {
	
	if(receive_buffer.size() >= receive_buffer.max_size - 1){
//		if(!receive_buffer.overflow){
//			print_l("\r\rIC RX buffer overflowed. New messages are discarded during this condition.\r\r");
			errors.rx_buffer_overflow++;
//		}
//		receive_buffer.overflow = true;
	}

	else {

		receive_buffer.printchar(c);

		if (c == '\r') 			 	{ pending_incoming_messages++;  }
		else if (is_disconnected()) { start_interchar_timer();		}
	}
	
}

/**
 * @brief Writes a char to the transmit buffer checking for overflow
 */
void IrisControls4::write_tx_buffer(char c){
	if(transmit_buffer.size() >= transmit_buffer.max_size - 1) {
//		transmit_buffer.overflow = true;
		errors.tx_buffer_overflow++;
//		flush();
//		print_l("IC TX buffer overflowed. Buffer flushed (blocking firmware execution). To avoid this issue, increase buffer size, decrease transmission burst sizes, or use the flush function between large bursts.\r\r");
	}
	else {
		transmit_buffer.printchar(c);
	}
//	if(transmit_buffer.overflow && c == '\r'){
//		print_l("IC TX buffer overflowed. Buffer flushed (blocking firmware execution). To avoid this issue, increase buffer size, decrease transmission burst sizes, or use the flush function between large bursts.\r\r");
//		transmit_buffer.overflow = false;
//	}
}

/**
 * @brief pops the oldest message off the rx_buffer
 *
 * The message's command string is saved to the command buffer 
 * The message's arguments string is saved to the argument buffer
 * This function assumes that the pending_incoming_messages counter is greater than zero
 * Decrements the pending_incoming_messages counter
 *
 * Note that the passed command and argument arrays must have memory allocated to them 
 */
int IrisControls4::get_message(char* command, char* argument) {

	u32 cmd_pos = 0, arg_pos = 0;

	int complete_cmd = 0;
	for (cmd_pos = 0; cmd_pos < receive_buffer.size(); cmd_pos++) {					// check for first whitespace or end of buffer
		u32 index = (cmd_pos + receive_buffer.start_index) & receive_buffer.mask;	// handle wrap-around for end of buffer memory
		if (receive_buffer.d[index] == '\r') {										// cmd has no arguments; message is fully extracted
			complete_cmd = 2;
			break;
		}
		if (receive_buffer.d[index] == ' ') {										// cmd has arguments; index points to whitespace between command and first arg
			complete_cmd = 1;
			break;
		}
		if (receive_buffer.d[index] == '\0') {										// rx buffer contains an incomplete message 
			complete_cmd = 0;
			break;
		}
		command[cmd_pos] = receive_buffer.d[index];									// write from rx_buffer to command string

		if (cmd_pos >= MAX_COMMAND_LENGTH - 1) {
			PRINTL("[IC4] Command overflow: discarding all received data");
			receive_buffer.start_index = 0;
			receive_buffer.end_index = 0;
			pending_incoming_messages = 0;
			command[0] = '\0';
			argument[0] = '\0';
			handle_eot();
			return 0;
		}
	}
	command[cmd_pos] = '\0';													// terminate command string

	if (complete_cmd) {
	    // Get args
		for (arg_pos = 0; arg_pos + cmd_pos < receive_buffer.size(); arg_pos++) {
			u32 index = (arg_pos + cmd_pos + receive_buffer.start_index) & receive_buffer.mask;		
			if (receive_buffer.d[index] == '\r') {								// end of message detected
				complete_cmd = 2;
				break;
			}
			argument[arg_pos] = receive_buffer.d[index];						// write from rx_buffer to argument string

			if (arg_pos >= MAX_COMMAND_LENGTH - 1) {
				PRINTL("[IC4] Argument overflow: discarding all received data");
				receive_buffer.start_index = 0;
				receive_buffer.end_index = 0;
				pending_incoming_messages = 0;
				command[0] = '\0';
				argument[0] = '\0';
				handle_eot();
				return 0;
			}
		}
		argument[arg_pos] = '\0';												// terminate argument string
	}

	if (complete_cmd == 2) {
		receive_buffer.start_index += arg_pos + cmd_pos + 1;
		pending_incoming_messages--;
		return 1;
	}

	command[0] = '\0';
	argument[0] = '\0';

	return 0;
}

/**
 * @brief Builds the crc data array for use in the handshake

 */
void IrisControls4::build_crc_data(const char * data){
	for (int i = 0; data[i] != '\0'; i++){
		crc_data[crc_index] = data[i];
		crc_index++;
	}
}
void IrisControls4::build_crc_data(u16 u){
	for (int i = 1; i>=0; i--){
		crc_data[crc_index] = (u8)(u >>(8*i));
		crc_index++;
	}
}
void IrisControls4::build_crc_data(u64 u){
	for (int i = 7; i>=0; i--){
		crc_data[crc_index] = (u8)(u >>(8*i));
		crc_index++;
	}
}

/*
 * @brief Checks value of all active elements (active elements are those that have had their
 * "add" commands called and therefore they exist in Iris Controls. Those values are used to
 * compute a CRC which is transmitted to Iris Controls for comparison.
 */
void IrisControls4::device_state_check(){

//
//	uint8_t data;
//	uint8_t remainder = 0;
//
//	int item_count = 0;
//
//	print_l("========================== \rDevice Elements:\r");
//
////	for (IO_thing * thing = IO_registry::get_active_list(); thing; thing = thing->next_active) {
////		item_count++;
////		print_l("\rName: \"");
////		print_l(thing->name);
////		print_l("\" || Value: ");
////		print_d(thing->get()); 
////
////		const char * value_char = get_char_ptr(thing->get()); 
////
////		//run formatted value through CRC
////		for (int byte = 0; value_char[byte] != '\0'; ++byte) {
////			data = value_char[byte] ^ remainder;
////			remainder = CRC::table[data] ^ (remainder << 8);
////		}
////	}
//	print_l("\r\rItem Count: ");
//	print_d(item_count);
//	print_l("\rDevice CRC Result: ");
//	print_d(remainder);
//	print_l("\r ========================== \r\r");
//
//
//	//send serial cmd to IC
//	tx_crc_result(remainder);

}


