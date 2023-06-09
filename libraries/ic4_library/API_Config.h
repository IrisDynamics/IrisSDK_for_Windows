/**
 * @file API_Config.h
 * @author  Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.2
 * @brief Contains the enum of all IC4 serial command chars
    
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

#pragma once

#define API_VERSION "IC4_v3.0"


#define FLAG 			'\x02' 	//STX
#define TRAILER 		'\x03'	//ETX
#define ESC				'\x10'	//DLE
#define END_STRING		'\x19' 	//EM
#define SPACE			'\x20'
#define FRAMING_CHAR	'\r'   

// Style RGBa values
#define CLEAR			0,   0,   0,   0
#define WHITE			255, 255, 255, 255
#define TRANS_WHITE		255, 255, 255, 100
#define OFF_WHITE		190, 195, 199, 255

#define RED 			255, 037, 037, 200
#define TRANS_RED 		255, 037, 037, 100
#define SOFT_RED 		230, 100, 75, 200
#define BRICK_RED 		150, 040, 027, 200
#define DARK_RED 		50, 0, 0, 200

#define ORANGE			255, 125, 000, 255
#define OP_ORANGE		255, 125, 000, 100
#define LIGHT_ORANGE	255, 150, 51,  255
#define DARK_ORANGE		230, 111, 0,   255

#define YELLOW			255, 255, 69,  200
#define GOLD_YELLOW		255, 184, 28,  200
#define BUS_YELLOW		250, 220, 0,  200

#define GREEN			105, 255, 100, 200
#define TRANS_GREEN		105, 255, 100, 100
#define SOFT_GREEN		40, 80, 40, 200
#define DARK_GREEN		2, 48, 32, 200
#define PISTACHIO_GREEN		147, 197, 114,  200

#define BLUE			105, 105, 255, 255
#define TRANS_BLUE		105, 105, 255, 100
#define LIGHT_BLUE		100, 100, 255, 100
#define DARK_BLUE		1, 1, 122, 200
#define TRUE_BLUE		45, 85, 255, 200
#define SKY_BLUE		25, 181, 254, 200

#define PINK			244,  25, 244, 255
#define CORAL_PINK 		255, 91, 91, 200
#define BLUSH_PINK		190,  86, 131, 200
#define LIGHT_PINK		250,  187, 200, 200
#define DARK_PINK		170,  50, 100, 200

#define PURPLE			100, 100, 255, 100
#define TRUE_PURPLE		128, 0, 128, 200
#define PLUM_PURPLE		48, 25, 52, 200
#define LILAC_PURPLE	150, 110, 172, 200

#define GREY			105, 105, 105, 105
#define LIGHT_GREY		230, 230, 230, 255
#define OP_LIGHT_GREY	230, 230, 230,  25
#define DARK_GREY	 	 69,  69,  69, 255
#define TRANS_DARK_GREY	 69,  69,  69, 105
#define OP_DARK_GREY	 69,  69,  69,  69

#define BEIGE			210,  180, 140, 200
#define TAN				170,  125, 80, 200
#define BROWN			78, 52, 46, 200


enum IC4_SERIAL_API_COMMANDS{

	END_OF_TRANSMISSION				= 4,
	DISCONNECT						= 17,
	DEVICE_STATE_CHECK				= 18,
	LOAD_CONFIG_FILE				= 19,
	
	FLEXBUTTON_ADD					= 20,
	FLEXBUTTON_CHECK				= 22,

	FLEXSLIDER_ADD					= 30,
	SET_FLEXSLIDER_RANGE			= 31,

	FLEXLABEL_ADD					= 40,

	FLEXDATA_ADD					= 45,

	ADD_GUI_PAGE					= 50,
	REMOVE_GUI_PAGE					= 51,
	SHOW_GUI_PAGE					= 52,
	HIDE_GUI_PAGE					= 53,
	ADD_ELEMENT_TO_PAGE				= 54,
	REMOVE_ELEMENT_FROM_PAGE		= 55,
	ADD_CHILD_PAGE_TO_PAGE			= 56,
	REMOVE_CHILD_PAGE_FROM_PAGE		= 57,
		
	FLEXPLOT_ADD					= 60,
	SET_WALKING_DOMAIN				= 61,
	HIDE_DATASET					= 63,
	SHOW_DATASET					= 64,
	SET_RANGE						= 65,
	SET_DOMAIN						= 66,
	SET_MAX_PLOTTED_POINTS			= 67,
	SET_AXES_LABELS					= 68,

	DATASET_ADD						= 70,
	DATASET_REMOVE					= 71,
	DATASET_ADD_FLOAT_DATA			= 72,
	DATASET_ADD_INT_DATA			= 73,
	DATASET_ADD_TIME_DATA			= 79,
	DATASET_PURGE_DATA				= 74,
	DATASET_ASSIGN					= 75,
	DATASET_UNASSIGN				= 76,		
	DATASET_SET_COLOUR				= 77,
	DATASET_SET_MAX_SIZE			= 78,

	SET_GUI_ROWS_COLS				= 80,
	QUERY_MAX_ROWS_COLS				= 81,
	SET_MAX_ROWS_COLS				= 82,
	INJECT_DELAY					= 83,
	SET_DEFAULT_COLOUR				= 84,
	RESET_ALL_DEFAULT_COLOURS		= 85,
	RESET_COLOURS_OF_ELEMENT_TYPE	= 86, 
	RESET_SPECIFIC_INSTANCE_COLOURS = 87, 
	SET_WINDOW_TITLE				= 88,

	HIDE_ELEMENT					= 100,
	SHOW_ELEMENT					= 101,
	MOVE_ELEMENT					= 102,
	REMOVE_ELEMENT					= 103,
	RESIZE_ELEMENT					= 104,
	RENAME_ELEMENT					= 105,
	UPDATE_VALUE					= 106,
	SET_COLOUR						= 107,
	CONFIG_ELEMENT					= 108,
	DISABLE_ENABLE					= 109,
	SET_FONT_SIZE					= 110,
	ASSIGN_KEYBOARD_SHORTCUT		= 111,	


	FLEXDROPDOWN_ADD				= 115,
	FLEXDROPDOWN_ADD_OPTION			= 116,
	FLEXDROPDOWN_REMOVE_OPTION		= 117,

	CONSOLE_MESSAGE					= 120,

	LOG_ADD							= 121,
	LOG_WRITE						= 122,
	LOG_CLOSE						= 123,

};

enum HANDSHAKE_MESSAGES{
	NAME_ENQUIRY				= 5, // \x05 = ENQ 
	HANDSHAKE					= 6, // \x06 = ACK 
	BEGIN_CONNECTION			= 1, // \x01 = SOH 
	END_CONNECTION				= 7  // \x07 = BEL 
};

enum RX_MESSAGES{
	FLEXELEMENT_PRESSED			= 33,
	BUTTON_TOGGLED				= 34,
	FLEXELEMENT_UPDATED			= 35,
	CONSOLE_COMMAND				= 36,
	DATALOG_UPDATED				= 37,

	GENERIC_MESSAGE 			= 100,
	ERR_HANDLING 				= 101
};

//current incoming messages lengths including the FLAG and \r at the end i.e., FLAG + ID + payload + \r
enum RX_MESSAGE_LENGTHS {
	NO_PAYLOAD_LENGTH			= 3,
	FLEXELEMENT_PRESSED_LENGTH	= 7,
	BUTTON_TOGGLED_LENGTH		= 8,
	FLEXELEMENT_UPDATED_LENGTH	= 11,
	DATALOG_UPDATED_LENGTH		= 11,
};

enum RX_GENERIC_MESSAGES{
	GEOMETRY_QUERY				= 150
};

enum RX_ERROR_CODES {
	ERROR_COLLISION				= 1,
	ERROR_OUT_OF_BOUNDS			= 2,
	ERROR_INVALID_SIZE			= 3
};
