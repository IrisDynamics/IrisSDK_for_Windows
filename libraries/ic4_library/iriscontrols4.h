/*!
 * @file iriscontrols4.h
 * @author  Kyle Hagen <khagen@irisdynamics.com>, Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 * @brief Header file for IrisControls object
 *
 * The IrisControls library is responsible for maintaining connection to the IrisControls windows application
 * used for configuration and control of Iris firmware. It allows reading and writing to specific variables, creating logfiles, and viewing data in realtime charts.
 *
 * This library acts as master to the windows application, controlling which GUI elements are visible with serial commands.
 *
 * The library implements the transport layer, session layer, and presentation layer of the OSI networking model.
 *
 * Where this library is used, its virtual functions specific to the platform it is built on must be implemented.
 *
 * @section message_format Message Format 
 * 
 * Messages consist of a command string, and optionally an argument string. The command and list of arguemnts are separated by a single whitespace. The command is terminated by a carriage return character.
 *
    
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

#include "config.h"
#include "circularbuffer.h"
#include <stdio.h>
	
#include "io_elements.h"
#include "API_Config.h"

#include "CRC.h"

#include <string>

#define MAX_COMMAND_LENGTH		100
#define MAX_ARGUMENT_LENGTH		100
#define MAX_VAR_STR_LENGTH		32

#define MAX_HANDSHAKE_LENGTH	200

#define CONSOLE_ELEMENT_ID		9999

// Some macros to make an application-defined parser more readable 
// See print_help for an example 
#define START_PARSING if (0) {
#define COMMAND_IS  } else if( strcmp( cmd,
#define THEN_DO     ) == 0 ) {
#define FINISH_PARSING } else return 0; return 1;

#define PRINT(__ARG__) {IC4_virtual->print_l(__ARG__);}
#define PRINTL(__ARG__) {PRINT(__ARG__); PRINT("\r");}
#define PRINTD(__ARG__) {IC4_virtual->print_d(__ARG__);}
#define PRINTDL(__ARG1__,__ARG2__) {PRINT(__ARG1__);PRINTD(__ARG2__);PRINTL(".");}

/**
 * @class IrisControls4
 * @brief A virtual object which takes care of buffering and parsing IrisControls4 traffic.
 * 
 * Typical usage: 
 *	call check() once per GUI frame 
 *  optionally, call send() once the GUI frame is done executing (send will be called by the next gui frame so this is not strictly required) 
 * 
 * Note several of the functions are purely virtual; see the function documentation for implementation details. 
 */
class IrisControls4 {

	// static storage for the command and arguement stripped from the incoming data buffer
	char command[MAX_COMMAND_LENGTH];			// storage for the command part of a received message - todo: make this an int and modify get_message to return the mesage id instead of a command string
	char argument[MAX_ARGUMENT_LENGTH];

public:
	char var_to_str_array[MAX_VAR_STR_LENGTH];	// storage used when converting variables to strings

	int disable = 0;

	//////////////////////////////////////////// CONNECTION /////////////////////////////////////////////

	/**
	* @brief Sets the device ID.
	* @param[in] const char* did_string - The ID.
	* 
	* The device ID appears in the IrisControls4 COM select menu.
	*/
	void set_device_id(const char* did_string) {
		device_id		= did_string;	
	}

	/**
	* @brief Sets the server name.
	* @param[in] const char* name_string - The name.
	*/
	void set_server_name(const char* name_string) {
		server_name	= name_string;	
	}

	/**
	 * @brief Transmits buffered data, and parses received messages.
	 *
	 * This function should be called once during execution of a GUI frame.
	 */
	int check();


	/**
	* @brief determines if a full messages has been received in the receive buffer.
	* @param[in] u32& start	- The starting position in the circular buffer. 
	* @param[in] u32& end		- The ending position in the circular buffer. 
	* @param[out] int ret		- Returns 1 if a full message was found, otherwise 0.
	* 
	* @note 
	* This function is called as the check() function is looping through the bytes in the receive buffer between the start_index and end_index.
	* This function checks if the bytes between start and end make a full message, based on the command id. If they do, then start is moved to one index past the end position and true is returned.
	*/
	//int check_for_full_message(u32& start, u32& end);
	int check_for_full_message(u32& start);


	/**
	 * @brief Platform-defined function that starts transmission which moves data from the software buffer to the hardware transmitter
	 * @note
	 * The implementation should check the size of the transmit_buffer, and when it contains data, it should start transmitting it to IC4.
	 * In the case that the platform uses a UART to communicate, it should fill the UART hardware fifo from the transmit_buffer, and enable interrupts.
	 * The interrupt routine should continue to send characters from the transmit_buffer to the UART until all characters are transmitted.
	 * In a half-duplex implementation, this function should also watch the transmit_buffer for break-characters and stop transmission accordingly.
	 */
	virtual void send() = 0;


	/**
	 * @brief connection state machine methods
	 */
	int is_connected() {
		return connection_status == connected;
	}
	int is_disconnected() {
		return connection_status == disconnected;	
	}
	void set_disconnected() {
		connection_status = disconnected;
	}
	int is_timed_out() {
		return connection_status == timed_out;
	}
	int new_connection() {
		int ret = 0;
		if (is_connected()) {
			if (!was_connected) {
				ret = 1;
				was_timed_out = 0;
				was_connected = 1;
				//print_l("new connection~~\r\r");
			}
		}
		else {
			was_connected = 0;
		}
		return ret;
	}
	int restored_connection(){
		int ret = 0;
		if(is_connected() && was_timed_out){
			ret = 1;
		}
		return ret;
	}

	/**
	* @brief Resets the timeout timer for the IC4 connection.
	* 
	* Called when enqueuing an EOT into the TX buffer.
	*/
	void refresh_timeout(){		
		time_last_message = system_time();		
	}

	/**
	 * @brief Console object created so that it can be manipulated using FlexElement methods (move, hide, resize, etc...).
	 */
	Console console;

	int 	pop_int 		();
	double	pop_double		();
	bool	pop_bool		();

	int 			parse_int 		();
	int				parse_int		(char* input , unsigned int& index  );
	virtual double 	parse_double	();
	double 			parse_double	(char * input, unsigned int & index );

	///////////////////////////////////////// General GUI Implementation //////////////////////////////////
	void gui_set_grid			(u16 num_rows, u16 num_columns);
	void gui_query_max_grid		();
	void gui_set_max_grid		();
	void inject_delay			(u16 delay_length);
	void end_of_frame();
	void set_main_window_title	(const char *);

	/**
	 * @brief Loads a configuration file via console commands.
	 * @param[in] const char* filename - The filename of the config file to load.
	 * 
	 * Note: The config file must be in the config_files directory of the IrisControls4.exe location.
	 */
	void load_config_file(const char * filename);

	/**
	 * @brief Assigns a keyboard shortcut to key 'key' which activates the element with the id 'target'.
	 * @param[in] int key		- The key which will activate the shortcut.
	 * @param[in] int target	- The element ID that will report "pressed" when the shortcut key is pressed.
	 * 
	 * For a list of keys: https://doc.qt.io/qt-5/qt.html#Key-enum
	 */
	void assign_keyboard_shortcut(int key, int target);

	/**
	 * @brief Functions that return the maximum allowable rows and columns for a GUI.
	 * @param[out] The maximum allowable rows/columns.
	 *
	 * May only be called after calling:    IC4_virtual->gui_query_max_grid();
	 *
	 */
	int get_max_rows(){ return max_rows; }
	int get_max_cols(){ return max_cols; }

	///////////////////////////////////////// Element Updates //////////////////////////////////
	/**
	* @fn int was_any_element_pressed()
	* @brief Returns 1 if any element has been pressed since this last time this was called.
	*/
	int was_any_element_pressed(){
		int r = element_press_received;
		element_press_received = 0;
		return r;
	}
	
	/**
	* @fn int was_any_value_updated()
	* @brief Returns 1 if any element has been updated (changed value) since this last time this was called.
	*/
	int was_any_value_updated(){
		int r = element_value_received;
		element_value_received = 0;
		return r;
	}

	/**
	 * @brief print a single character to the tx buffer.
	 * @param[in] char c - The char.
	 * 
	 * If the FLAG or ESC char is detected, inject an ESC char into the tx buffer before sending (byte stuffing).
	 */
	void print_c(char c) {
		#ifdef BYTE_STUFFING_PARSING
		if(c == FLAG || c == ESC) write_tx_buffer(ESC);
		#endif
		write_tx_buffer(c);
	}

	/**
	* @brief print a null-terminated string to the tx buffer.
	* @param[in] const char* s - The string.
	*
	* Will display in the console of IrisControls.
	*/
	void print_l(const char* s) {

#ifdef MESSAGE_LENGTH_PARSING
		begin_tx_frame(size_of_string(s));
#elif defined(BYTE_STUFFING_PARSING)
		begin_tx_frame();
#endif
		print_c((char)CONSOLE_MESSAGE);
		print_s(s);
		end_tx_frame();
	}
	void print_(const char * s){ print_l(s);}

	/**
	 * @brief Prints a formatted int to the tx buffer.
	 * @param[in] int d - The int to display.
	 *
	 * These will be displayed in the console of IrisControls.
	 */
	void print_d(int d) {
		print_l(val_to_str(d));
	}
	void print_d(unsigned int d) {
		print_l(val_to_str(d));
	}
	void print_d(u64 d)	{
		print_l(val_to_str(d));
	}
	
#ifndef WINDOWS
	void print_d(int32_t d) { print_d((int)d); }
	void print_d(u32 d) 	{ print_d((unsigned int)d); }
#endif

	/**
	 * @brief print a formatted float to the tx buffer.
	 * @param[in] float f - The float.
	 *
	 * Will display on the console of IrisControls.
	 */
	void print_f(float f) {
		print_l(val_to_str(f));
	}

	/**
	 * @brief Platform-defined function which returns the system time in microseconds
	 * @param[out] system time in microseconds
	 */
	virtual u64 system_time() = 0;

	/**
	* @brief Flushes the TX buffer by calling send until it is empty.
	*/
	void flush() {
		while( transmit_buffer.size()) {
			send();
		}
	}

	/**
	* @brief Flushes the TX buffer by calling send until it reaches a threshold value.
	* @param[in] u32 threshold - The threshold value in bytes.
	*/
	void flush(u32 threshold) {
		while( transmit_buffer.size() > threshold) {
			send();
		}
	}

protected:
	volatile u32 time_last_message = 0;

	friend class GUI_Page;
	friend class FlexElement;
	friend class FlexButton;
	friend class FlexSlider;
	friend class Basic_FlexSlider;
	friend class FlexLabel;
	friend class C_FlexLabel;
	friend class FlexData;
	friend class Basic_FlexData;
	friend class FlexDropdown;
	friend class MenuOption;
	friend class FlexPlot;
	friend class Dataset;
	friend class IO_thing;
	friend class Complex_IO_thing;
	friend class GUI_thing;
	friend class Console;
	friend class DataLog;

	IrisControls4() :
		pending_incoming_messages(0)
	,	serial_api_version(API_VERSION)
	,	communication_protocol(protocol_unset_by_platform)
	{
		#ifdef FULL_DUPLEX
			set_full_duplex();
		#elif defined(HALF_DUPLEX)
			set_half_duplex();
		#endif

		CRC::build_table();
	}
	virtual ~IrisControls4(){}

	// Software buffers used to temporarily hold the serial streams while they are transmitted and received
	CircularBuffer
		<TX_BUFFER_BITS>
		transmit_buffer;
	CircularBuffer
		<RX_BUFFER_BITS>
		receive_buffer;

	u32 pending_incoming_messages;		// see receive_char() (increments) and get_message() (decrements)
	u8  eot_queued = 0;

	//const char * communication_protocol = "UNSET_by_platform";
	const char * device_id 				= "UNSET_by_platform";
	const char * server_name 			= "UNSET_by_platform";
	const char * serial_api_version 	= API_VERSION;
	const u16 	 timeout_timer 			= 1000; 

	void set_full_duplex 	() { communication_protocol = full_duplex; 				}
	void set_half_duplex 	() { communication_protocol = half_duplex; 				}

	enum CONNECTION_STATUS {
		disconnected,
		connected,
		timed_out,
	} connection_status;

	enum CHANNEL_STATUS {
		idle,
		sending,
		recieving,
	} channel_status;

	enum COMMUNICATION_PROTOCOL {
		protocol_unset_by_platform	= 0,
		full_duplex					= 1,
		half_duplex					= 2,
	} communication_protocol;

	enum MESSAGE_FORMAT {
		no_byte_stuffing 	= 1,
		byte_stuffing		= 2
	};


	/**
	 * @brief Platform-defined function which polls hardware and moves data from hardware to the software buffer. 
	 * 
	 * This is called by at the start of check().  
	 * Some platforms may not implement this function (ie leave it as this empty default), if the hardware is not polled. 
	 * For example, a platform which receives UART interrupts would instead directly call receive_char() from the interrupt routine instead.
	 * 
	 * Note, in half-duplex implementions, platforms should inject a clear-to-send, followed by break character into the transmit_buffer.
	 */
	virtual void receive() {};

	/**
	 * @brief Device implemented function that determines the behaviour after receiving an EOT from IrisControls.
	 */
	virtual void handle_eot() = 0;	

	/**
	 * @brief Writes a char to the transmit buffer checking for overflow.
	 * @param[in] char c - The char.
	 */
	void write_tx_buffer(char c);

	/**
	 * @brief platform must implement a response to an unidentified command.
	 *
	 * @param[in] char * cmd - The command string in the message that caused the help to be called.
	 * @param[in] char * arg - The argument string in the message that caused the help to be called.
	 */
	virtual void print_help(char* cmd) {
		print_l("Unknown Command: "); print_l(cmd); print_l("\r");
		print_l("Use command \"help\" for a list of allowable commands.\r");
	}

	/**

* @brief print a formatted number to the tx buffer
*/
	virtual const char* val_to_str(int d) = 0;
	virtual const char* val_to_str(unsigned int) = 0;
	virtual const char* val_to_str(u64) = 0;
#ifndef WINDOWS
virtual const char* val_to_str(int32_t d) { return val_to_str((int)d); }
	virtual const char* val_to_str(u32 d) { return val_to_str((unsigned int)d); }
#endif

#ifdef  WINDOWS
public:
	bool setup_sucess = false;
	virtual void setup(int comport) = 0;
#endif //  windows

	virtual const char* val_to_str(float f) = 0;


	/**
	 * @brief Initiates a disconnect from Iris Controls
	 */
	void disconnect();

	// Data inflow sequence: 
	
	// 1
	void receive_char(char c);
	
	// 2
	int get_message(char* command);
	
	// 3 
	int parse_message(char* cmd);
	
	// 4
	int parse_API();
	
	// 5
	/**
	 * @brief an optional parser that can be implemented by the application layer
	 */
	virtual int parse_app(char* cmd) { return 0; }
	virtual int parse_app(char* cmd, char* args) { return 0; } //deprecated function written here to prevent older firmware from malfunctioning

	// 6
	int parse_console_msg( char * cmd);
		
	// 7
	/**
	 * @brief an optional hardware specific parser that can be implemented by the device driver object
	 */
	virtual int parse_device_driver(char* cmd) { return 0; }


	///////////////////////////////////////// Responses to incoming requests //////////////////////////////
	void enquiryResponse();
	void handshakeResponse(u64, uint8_t);

	///////////////////////////////////////// GUI Thing Implementation //////////////////////////////////
	void gui_thing_config		(u32 index, u32 config);

	///////////////////////////////////////// GUI Page Implementation //////////////////////////////////////
	void gui_page_add			(u32 index, u32 parent_index);
	void gui_page_remove		(u32 index);
	void gui_page_show			(u32 index);
	void gui_page_hide			(u32 index);
	void gui_page_add_element	(u32 index, u32 element_index);
	void gui_page_remove_element(u32 index, u32 element_index);
	void gui_page_add_page		(u32 index, u32 child_page_index);
	void gui_page_remove_page	(u32 index, u32 child_page_index);

	///////////////////////////////////////// Flex Element Implementation //////////////////////////////////
	void flexElement_hide			(u32 index);
	void flexElement_show			(u32 index);
	void flexElement_move			(u32 index, u16 row, u16 column);
	void flexElement_remove			(u32 index);
	void flexElement_resize			(u32 index, u16 rowSpan, u16 columnSpan);
	void flexElement_rename			(u32 index, const char* name);
	void flexElement_update 		(u32 index, int value);
	void flexElement_setColour		(u32 index, u16 config, u16 r, u16 g, u16 b, u16 a);
	void flexElement_config			(u32 index, u16 config);
	void flexElement_disable		(u32 index, bool status);
	void flexElement_set_font_size	(u32 index, u16 sub_element, u16 font_size);

	///////////////////////////////////////// Flex Button Implementation //////////////////////////////////
	void flexButton_add			(u32 parent_id, u32 index, const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan);
	void flexButton_set_checked	(u32 index, int t);

	////////////////////////////////////////////flex Slider Implementation///////////////////////
	void flexSlider_add						(u32 parent_id, u32 index, int total_factor, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, const char* units, u32 config);
	void flexSlider_update					(u32 index, float value);
	void flexSlider_set_range				(u32 index, int min, int max);

	////////////////////////////////////////////flex Label Implementation///////////////////////
	void flexLabel_add		(u32 parent_id, u32 index, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config);

	////////////////////////////////////////////flex Data Implementation///////////////////////
	void flexData_add		(u32 parent_id, u32 index, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char * units, int total_factor, u32 config);

	////////////////////////////////////////////flex Dropdown Implementation///////////////////
	void flexDropdown_add				(u32 parent_id, u32 index, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config);
	void flexDropdown_add_option		(u32 index, u32 option_id, const char* label);
	void flexDropdown_remove_option		(u32 index, u32 option_id);

	////////////////////////////////////////////Flex Plot Implementation///////////////////////
	void flexPlot_add						(u32 parent_id, u32 index, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config);
	void flexPlot_set_range					(u32 index, int config, float min, float max);
	void flexPlot_set_domain				(u32 index, float min, float max);
	void flexPlot_set_domain				(u32 index, int domain);
	void flexPlot_set_visible_datapoints	(u32 index, u16 datapoints);
	void flexPlot_set_axes_labels			(u32 flexplot_index, u32 dataset_index);

	////////////////////////////////////////////Dataset Implementation///////////////////////
	void dataset_add					(u32 dataset_id, u32 plot_id, const char* name, const char* _x_label, const char* _y_label, u32 config);
	void dataset_set_max_data_points	(u32 index, u32 number_of_data_points);
	void dataset_remove					(u32 index);
	void dataset_add_float_data			(u32 index, u16 dataPairs, float xData[], float yData[]);
	void dataset_add_int_data			(u32 index, int xData, int yData);
#ifndef WINDOWS
	void dataset_add_int_data			(u32 index, s32 xData, s32 yData);
#endif
	void dataset_add_int_data			(u32 index, u64 xData, int yData);
	void dataset_hide					(u32 index);
	void dataset_show					(u32 index);
	void dataset_purge					(u32 index);
	void dataset_assign					(u32 dataset_index, u32 flexplot_index);
	void dataset_unassign				(u32 index);
	void dataset_set_colour				(u32 index, u16 r, u16 g, u16 b, u16 a);

	////////////////////////////////////////////Dataset Implementation///////////////////////
	void datalog_add 					(u32 index, const char * name);
	void datalog_write					(u32 index, const char * string);
	void datalog_close					(u32 index);

	///////////////////////////////////////// Style Implementation //////////////////////////////////
	void reset_all_default_colours		();
	void reset_element_default_colours	(u16 element_type);
	void reset_specific_element_colours	(u32 index);
	void set_default_colour				(u16 element_type, u16 config, u16 r, u16 g, u16 b, u16 a);

	struct Errors {
		u32 tx_buffer_overflow			= 0;
		u32 rx_buffer_overflow			= 0;
		u32 dropped_gui_frames			= 0;
	};

	Errors errors;

private :

	u16 tx_buffer_bits;
	u16 rx_buffer_bits;

	int was_connected 	= 0;
	int was_timed_out 	= 0;

	u16	element_press_received	= 0;
	u16 element_value_received	= 0;

	u16 crc_index 		= 0;
	uint8_t crc_data[MAX_HANDSHAKE_LENGTH];

	u16 max_rows = 0;
	u16 max_cols = 0;

	u64 interchar_start_time;
	u64 interchar_timer_duration = 1000000; // 1 second
	volatile bool interchar_enabled = 0;

	void start_interchar_timer() {
		interchar_start_time = system_time();
		interchar_enabled = 1;
	}

	bool is_interchar_timer_expired() {
		if( interchar_enabled
				&& ( (u64)(system_time() - interchar_start_time) > interchar_timer_duration) )
		{
			interchar_enabled = 0;
			return true;
		}
		else return false;
	}

	void build_crc_data(const char *);
	void build_crc_data(u16);
	void build_crc_data(u64);

	void tx_crc_result(int);

	/**
	* @brief Prints a formatted string to the tx buffer.
	* @param[in] const char* s - The char.
	* 
	* Used by the methods in serialAPI.h to transmit strings terminated by the END_OF_STRING char.
	*/
	void print_s(const char* s) {
			
		u8 c = s[0];
		u16 i = 0;

		while (c != '\0') {
			print_c(s[i]);
			i++;
			c = s[i];
		}
		write_tx_buffer(END_STRING);
	}

	/**
	 * @brief Prints a formatted int to the tx buffer.
	 * @param[in] int d - The int.
	 * 
	 * Used by the methods in serialAPI.h to transmit integers broken up into four 1 byte words.
	 */
	void print_i(int d) {
		u8 the_int[4];

		the_int[0] = (u8)(d>>24);
		the_int[1] = (u8)(d>>16);
		the_int[2] = (u8)(d>> 8);
		the_int[3] = (u8)(d>> 0);

		for(int i=0; i<4; i++){
			print_c(the_int[i]);
		}
	}
#ifndef WINDOWS
	void print_i(s32 d) { print_i((int)d); }
#endif 	

	/**
	 * @brief Prints a formatted unsigned int to the tx buffer.
	 * @param[in] unsigned int d - The unsigned int.
	 * 
	 * Used by the methods in serialAPI.h to transmit unsigned ints broken up into four 1 byte words.
	 */
	void print_i(unsigned int d) {
		u8 the_int[4];

		the_int[0] = (u8)(d>>24);
		the_int[1] = (u8)(d>>16);
		the_int[2] = (u8)(d>> 8);
		the_int[3] = (u8)(d>> 0);

		for(int i=0; i<4; i++){
			print_c(the_int[i]);
		}
	}
#ifndef WINDOWS
	void print_i(u32 d) { print_i((unsigned int)d); }
#endif // !WINDOWS

	
	/**
	 * @brief Prints a formatted u16 to the tx buffer.
	 * @param[in] u16 d - The u16.
	 * 
	 * Used by the methods in serialAPI.h to transmit u16 integers broken up into two 1 byte words.
	 */
	void print_i(u16 d) {
		u8 the_u16[2];

		the_u16[0] = (u8)(d>>8);
		the_u16[1] = (u8)(d>>0);

		for(int i=0; i<2; i++){
			print_c(the_u16[i]);
		}
	}
	void print_i(u8  d) { print_i((u16)d); 			}

	/**
	 * @brief Prints a formatted u64 to the tx buffer.
	 * @param[in] u64 d - The u64.
	 * 
	 * Used by the methods in serialAPI.h to transmit u64 integers broken up into eight 1 byte words.
	 */
	void print_i(u64 d)  {

		u8 the_u64[8];

		the_u64[0] = (u8)(d>>56);
		the_u64[1] = (u8)(d>>48);
		the_u64[2] = (u8)(d>>40);
		the_u64[3] = (u8)(d>>32);
		the_u64[4] = (u8)(d>>24);
		the_u64[5] = (u8)(d>>16);
		the_u64[6] = (u8)(d>> 8);
		the_u64[7] = (u8)(d>> 0);

		for(int i=0; i<8; i++){
			print_c(the_u64[i]);
		}
	}

	/**
	 * @brief Prints a formatted float to the tx buffer.
	 * @param[in] float f - The float.
	 * 
	 * Used by the methods in serialAPI.h to transmit floats broken up into 1 byte words.
	 */
	void print_i(float f) {
		const u8 size = 4;	//number of bytes the float occupies	
		u8 the_float[size];
		memcpy(the_float,&f,size);
		
		for(int i=0; i<size; i++){
			print_c(the_float[i]);
		}
	}



	/**
	 * @brief Prepends an outgoing serial message with the message frame
	 */
	void begin_tx_frame(u16 message_size = 0){
		write_tx_buffer(FLAG);
		#ifdef MESSAGE_LENGTH_PARSING
		print_i(message_size);
		#endif
	}

	/**
	 * @brief Appends the message frame to the end of an outgoing serial message
	 */
	void end_tx_frame(){
		#ifdef BYTE_STUFFING_PARSING
		write_tx_buffer(TRAILER);
		write_tx_buffer(FLAG);
		#endif
	}

#ifdef MESSAGE_LENGTH_PARSING
	/**
	 * @brief Calculates the size of the string.
	 * @param[in] const char* msg - The string.
	 */
	u16 size_of_string(const char* msg){
		u16 the_size = 0;
		while(msg[the_size] != '\0') the_size++;
		return the_size;
	}
#endif

};

extern IrisControls4* IC4_virtual;
