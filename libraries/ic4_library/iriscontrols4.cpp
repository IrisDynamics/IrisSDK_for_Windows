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
#include <stdio.h>	//k8 added 

uint8_t CRC::table[256];
int 	CRC::is_built = 0;

/**
* @fn int IrisControls4::check()
* @brief Starts the transmitter when it is idle and there is data in the tx_buffer.
* @param[out] int ret - The number of pending incoming messages. (Should be 0 as this function calls parse_message() until that time).
*
* @note 
* Removes all pending messages from the rx buffer by monitoring the pending_incoming_messages counter.
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

	if (receive_buffer.size()) {

		//check to see there is a full message in the circular buffer.
		u32 start = receive_buffer.start_index;	
		
		while (start != receive_buffer.end_index && check_for_full_message(start)) {
			pending_incoming_messages++;
			start++;
		}

		if (!pending_incoming_messages && is_disconnected()) start_interchar_timer();
	}

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
		print_l("connection status = timed out");
		was_timed_out	= 1;
	}

	// Parse message - empty receive buffer
	u32 ret = pending_incoming_messages;

	//parse messages while there are messages to parse 
	while (pending_incoming_messages) {		
		parse_message	(command);
	}
	return ret;
}

/**
* @fn int IrisControls4::check_for_full_message(u32& start, u32& end)()
* @brief Determines if a full messages has been received in the receive buffer.
* @param[in] u32& start	- The starting position in the circular buffer.
* @param[out] int 		- Returns 1 if a full message was found, otherwise 0.
*  
*/
int IrisControls4::check_for_full_message(u32 &start) {

	//check for handshake, name enquiry, or EOT if disconnected (these messages aren't properly byte stuffed with trailers).
	if (is_disconnected()) {

		u32 end = receive_buffer.end_index;

		int command_id = (int)receive_buffer.d[((start + 1) & receive_buffer.mask)];
		
		switch (command_id) {
			case HANDSHAKE:
			case NAME_ENQUIRY:
			case BEGIN_CONNECTION:
			case END_OF_TRANSMISSION:
				if (end - start >= NO_PAYLOAD_LENGTH) {
					start += 2;
					if (receive_buffer.d[start & receive_buffer.mask] == TRAILER) start++;
					return 1;
				}			
				break;
			
			// Check for console command when disconnected. Used by Orcabrains to command device reset.
			case CONSOLE_COMMAND:
			default:
				while (start != receive_buffer.end_index) {
					if (receive_buffer.d[start & receive_buffer.mask] == FRAMING_CHAR) return 1;
					start++;
				}
				break;
		}
	}
	else {			

		bool start_found	= false;
		bool end_found		= false;

		//go through rx buffer to find FLAG chars and then use logic to see if they are a true start or end of message
		while (start != receive_buffer.end_index) {

			//look at each char and if it's a flag, check to see if its a true FLAG or true TRAILER
			if (receive_buffer.d[start & receive_buffer.mask] == FLAG) {

				//Test if this FLAG is a true begin of message (a payload byte that happens to be FLAG will ALWAYS be preceeded by ESC and a trailer precceded with TRAILER).
				if ((start == receive_buffer.start_index)
					|| (receive_buffer.d[((start - 1) & receive_buffer.mask)] != TRAILER
					&&  receive_buffer.d[((start - 1) & receive_buffer.mask)] != ESC))
				{
					start_found = true;
				}

				//Test if this FLAG is a true trailer (a trailer FLAG will always be preceeded by TRAILER).		
				if (start != receive_buffer.start_index
					&& receive_buffer.d[((start - 1) & receive_buffer.mask)] == TRAILER)
				{
					end_found = true;
				}
			}
			//if we found both start and end, return 1
			if (start_found && end_found) return 1;

			start++;
		}
	}

	return 0;
}

/**
* @fn int IrisControls4::parse_API()
* @brief Parses IC4 messages. 
* @return int - Returns 0 when the message was not parsed by the API parser (ie the message should be passed along to subsequent parsers), and 1 when the message was parsed
*/
int IrisControls4::parse_API() {	

	// Here ret represents whether a full message was parsed
	int ret = 0;
	
	// If the FLAG isn't present, this isn't an IC4 message so return 0
	if(receive_buffer.d[receive_buffer.start_index & receive_buffer.mask] != FLAG) return 0;

	// Check the command ID
	int command_id = (int)receive_buffer.d[(receive_buffer.start_index + 1) & receive_buffer.mask];

	// If the command is a console command, return early and let another parser parse this message.
	if (command_id == CONSOLE_COMMAND){	return 0; }

	// Pop the FLAG char off the RX buffer
	receive_buffer.popchar();

	//handle the command based on the command_id
	switch (command_id) {
				
		int id;
		int value;

		case END_OF_TRANSMISSION:
			receive_buffer.popchar();			
			handle_eot();	
			ret = 1;
			break;
		case NAME_ENQUIRY: //name query
			receive_buffer.popchar();
			enquiryResponse();
			ret = 1;
			break;
		case HANDSHAKE: { //challenge
			//pop the challenge char (x06) off the recieve_buffer
			receive_buffer.popchar();
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
			ret = 1;
			break;
		}
		case BEGIN_CONNECTION:
			receive_buffer.popchar();
			connection_status = connected;
			print_l("IrisControls4: connected!!!\r");
			ret = 1;
			break;

		case END_CONNECTION:
			receive_buffer.popchar();
			set_disconnected();
			ret = 1;
			break;

		case FLEXELEMENT_PRESSED:
			receive_buffer.popchar();
			//PRINTL("Button pressed");
			id = pop_int();
			IO_registry::set_element_pressed(id);
			element_press_received = 1;
			ret = 1;
			break;

		case BUTTON_TOGGLED:
			receive_buffer.popchar();
			//PRINTL("Button toggled");
			id 			= pop_int();
			value 		= pop_bool();
			IO_registry::set(id, value);
			element_press_received = 1;
			ret = 1;
			break;

		case FLEXELEMENT_UPDATED: 
			receive_buffer.popchar();
			//PRINTL("Element updated");
			id 		= pop_int();
			value 	= pop_int();
			element_value_received = 1;
			IO_registry::set(id, value);

			ret = 1;
			break;

		case DATALOG_UPDATED:
			receive_buffer.popchar();
			//PRINTL("DataLog updated");
			id 		= pop_int();
			value 	= pop_int();
			DataLog_registry::update_status(id, value);
			ret	= 1;
			break;

		default:
			PRINTL("Error in parseAPI!");
			break;
	}

	//pop the trailer off the buffer. 
	if (ret) {
		receive_buffer.popchar();						//FRAMING_CHAR or TRAILER
		if(is_connected()) receive_buffer.popchar();	//FLAG
	}

	return ret;
}

/**
* @fn int IrisControls4::parse_int()
* @brief Parses an int from the receive buffer.
* @return int result - The parsed int.
* 
* @note
* Called by console command parsers.
*
*/
int IrisControls4::parse_int() {

	// If no data before trailer or no bytes in rx buffer, return early.
	if (!receive_buffer.bytes_to_trailer()) return -1;

	int output = 0;

	// Remove any extra whitespace before the argument starts
	while (receive_buffer.d[receive_buffer.start_index & receive_buffer.mask] == SPACE) receive_buffer.popchar();

	char c;

	// Populate argument char array
	int i = 0;
	while(receive_buffer.bytes_to_trailer() && i < MAX_ARGUMENT_LENGTH) {

		c = receive_buffer.d[receive_buffer.start_index & receive_buffer.mask];

		if (c == FRAMING_CHAR) { receive_buffer.popchar(); break; }
		if (c == SPACE || c == TRAILER) { break; }

		argument[i++] = c;

		receive_buffer.popchar();
	}

	argument[i] = '\0';

	// Convert to int from char array
	sscanf(argument, "%d", &output);

	argument[0] = '\0'; // Reset argument char array

	return output;
}
/**
* @overload int IrisControls4::parse_int(char* input, unsigned int& index)
* @brief Parses an int from the receive buffer - parameters for the old implementation - allows for backwards compatibility.
* @param[in] char* input			- The char array in the input buffer.
* @param[in] unsigned int& index	- The position in the array to start parsing.
* @return int						- The parsed int.
*/
int IrisControls4::parse_int(char* input, unsigned int& index) {
	return parse_int();
}

/**
* @fn double IrisControls4::parse_double()
* @brief Parses a double from the receive buffer.
* @return double result	- The parsed double.
*
* @note
* Called by console command parsers.
*/
double IrisControls4::parse_double() {

	// If no data before trailer or no bytes in rx buffer, return early.
	if (!receive_buffer.bytes_to_trailer()) return -1;

	double output = 0.0;

	// Remove any extra whitespace before the argument starts
	while (receive_buffer.d[receive_buffer.start_index & receive_buffer.mask] == SPACE) receive_buffer.popchar();

	char c;

	// Populate argument char array
	int i = 0;
	while(receive_buffer.bytes_to_trailer() && i < MAX_ARGUMENT_LENGTH) {

		c = receive_buffer.d[receive_buffer.start_index & receive_buffer.mask];

		if (c == FRAMING_CHAR) { receive_buffer.popchar(); break; }
		if (c == SPACE || c == TRAILER) { break; }

		argument[i++] = c;

		receive_buffer.popchar();
	}

	argument[i] = '\0';

	// Convert to double from char array
	sscanf(argument, "%lf", &output);

	argument[0] = '\0'; // Reset argument char array

	return output;
}
/**
* @fn double IrisControls4::parse_double(char* input, unsigned int& index)
* @brief Parses a double from the receive buffer - parameters for the old implementation - allows for backwards compatibility.
* @param[in] char* input			- The char array in the input buffer.
* @param[in] unsigned int& index	- The position in the array to start parsing.
* @return double					- The parsed double.
*/
double IrisControls4::parse_double(char* input, unsigned int& index) {
	return parse_double();
}

/**
* @fn int IrisControls4::pop_int()
* @brief Parses an int from the receive buffer.
* @param[out] int result - The parsed int.
* 
* @note
* Called by the IC4 serial messages.
* Pops the next 4 bytes off the buffer, and shifts them appropriatly, and casts them to an int.
*/
int IrisControls4::pop_int() {

	u8 parsed[4] = { 0 };

	for (int i = 0; i < 4; i++) {
		u8 test_char = (u8)receive_buffer.popchar();

		if (test_char == ESC) test_char = (u8)receive_buffer.popchar();

		parsed[i] = test_char;
	}

	int result =	(parsed[0] << 24)	|
					(parsed[1] << 16)	|
					(parsed[2] << 8)	|
					(parsed[3]);

	return result;
}

/**
* @fn double IrisControls4::pop_double()
* @brief parses a double from the receive buffer.
* @param[out] double result	- The parsed double.
* 
* @note
* Pops the next 4 bytes off the buffer, and shifts them appropriatly, and casts them to a double.
*/
double IrisControls4::pop_double() {

	double result;
	u8 parsed[4] = { 0 };

	for (int i = 0; i < 4; i++) {
		u8 test_char = (u8)receive_buffer.popchar();
		if (test_char == ESC) test_char = (u8)receive_buffer.popchar();
		parsed[i] = test_char;
	}

	memcpy(&result, parsed, sizeof(result));

	return result;
}

/**
* @fn bool IrisControls4::pop_bool()
* @brief Pops a char from the receive buffer, and returns that bool.
* @param[out] bool result - The parsed bool.
*/
bool IrisControls4::pop_bool() {
	int result = receive_buffer.popchar();
	return result;
}

/**
* @fn int IrisControls4::parse_message(char* cmd)
* @brief Parses a message, which contains a command string, and optionally an argument string.
* @param[in] char* cmd - The char array containing the command.
* @param[out] int ret - Returns 1 if the message was parsed and 0 if not.
*/
int IrisControls4::parse_message(char* cmd) {
	//look at the command id, pop the correct number off, and then parse the payload, which we just popped off 
	int ret = parse_API				();	//IC4 API parser

	//if parse_API did not return a 1, use the other parsers to parse the message
	if (!ret && get_message(cmd)) {
		ret |= parse_app(cmd);	//Application layer parser
		ret |= parse_app(cmd, cmd); //legacy app parser format, deprecated
		ret |= parse_console_msg(cmd);	//IC4 Console Message Parser
		ret |= parse_device_driver(cmd);	//Hardware Specific Parser		

		//Make sure the console message was fully parsed i.e., no more stray bytes on the RX buffer.
		if (is_connected() && receive_buffer.recover()) {
			PRINT("Error: Previous console message: "); PRINTL(cmd);  PRINTL("was not parsed correctly. Arguments remained on the buffer.");
		}
	}

	//if none of the parsers were able to parse the message, print help
	if (!ret) {
		print_help(cmd);
	}

	//decrement pending message count as the message is now off the recieve_buffer
	pending_incoming_messages--;
	return ret;
}




/**
* @fn void IrisControls4::receive_char(char c)
* @brief checks for an overflow, and if everything is good add a character (probably incoming coming from a UART) to the rx_buffer.
* @param[in] char c - The char to be written onto the rx buffer.
*
* @note
* This function would typically be called by a UART rx interrupt, or by a function that polls the uart for received character(s).
*/
void IrisControls4::receive_char(char c) {
	//check to see if the buffer has overflowed 
	if (receive_buffer.size() >= receive_buffer.max_size - 1) {
		print_l("\r\rIC RX buffer overflowed. New messages are discarded during this condition.\r\r");
		errors.rx_buffer_overflow++;
	}
	else {
		//buffer is good, add the char to the software buffer 
		//print_l("printchar");
		//print_c(c);
		receive_buffer.printchar(c);

	}
	
}

/**
* @fn void IrisControls4::write_tx_buffer(char c)
* @brief Writes a char to the transmit buffer checking for overflow
* @param[in] char c - The char to be written onto the tx buffer.
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
* @fn int IrisControls4::get_message(char* command)
* @brief checks to see whether a valid console commands has been recieved, and if so, saves it.
* @param[in] char* commmand - The command.
* @param[out] int ret		- 1 if the command is complete, otherwise 0.
* 
* @note
* The message's command string is saved to the command buffer. 
* The passed command array must have memory allocated to it. 
*/
int IrisControls4::get_message(char* cmd) {

	// Check to see if the message is an IC4 message by checking for the FLAG char.
	if(receive_buffer.d[receive_buffer.start_index & receive_buffer.mask] == FLAG){

		receive_buffer.popchar(); // pop the FLAG off

		// All IC4 console messages should have the CONSOLE_COMMAND command ID.
		if((int)receive_buffer.popchar() != CONSOLE_COMMAND) {
			PRINTL("[IC4] Corrupt console message received - get message");
			receive_buffer.clear();
			return 0;
		}
	}

	// Checks for message that starts with the SPACE char
	if (receive_buffer.d[receive_buffer.start_index & receive_buffer.mask] == SPACE) {
		PRINTL("Error: Console message cannot start with whitespace.");
	}

	u32 cmd_pos = 0;
	int ret = 0;

	while (receive_buffer.size()) {
		char temp = receive_buffer.d[receive_buffer.start_index & receive_buffer.mask];

		if (temp == SPACE || temp == TRAILER) {
			ret = 1;
			break;
		}
		
		if (temp == FRAMING_CHAR) { // This case should only occur if disconnected from IC4
			receive_buffer.popchar();
			ret = 1;
			break;
		}

		receive_buffer.popchar();

		cmd[cmd_pos++] = temp;		

		if (cmd_pos >= MAX_COMMAND_LENGTH - 1) {
			PRINTL("[IC4] Command overflow: discarding all received data");
			receive_buffer.clear();
			pending_incoming_messages = 0;
			cmd[0] = '\0';
			handle_eot();
			return 0;
		}
	}

	cmd[cmd_pos] = '\0';	// terminate command string

	if (!ret) {
		cmd[0] = '\0';
	}

	return ret;
}

/**
* @fn void IrisControls4::build_crc_data(const char * data)
* @brief Builds the crc data array for use in the handshake.
* @param[in] const char* data	- The string data used to create the CRC array.
* @param[in] u16 u				- The u16 data used to create the CRC array.
* @param[in] u64				- The u64 data used to create the CRC array.
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
