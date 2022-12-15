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

#define API_VERSION	"IC4_v2.4"

#define FLAG 		'\x02' 	//STX
#define TRAILER 	'\x03'	//ETX
#define ESC			'\x10'	//DLE
#define END_STRING	'\x19' 	//EM

// Style RGBa values
#define CLEAR			0,   0,   0,   0
#define WHITE			255, 255, 255, 255
#define OFF_WHITE		190, 195, 199, 255

#define RED 			255, 037, 037, 200

#define ORANGE			255, 125, 000, 255
#define OP_ORANGE		255, 125, 000, 100
#define LIGHT_ORANGE	255, 150, 51,  255
#define DARK_ORANGE		230, 111, 0,   255

#define YELLOW			255, 255, 69,  200

#define GREEN			105, 255, 100, 200

#define BLUE			105, 105, 255, 255
#define LIGHT_BLUE		100, 100, 255, 100

#define PINK			244,  25, 244, 255
#define PURPLE			100, 100, 255, 100

#define GREY			105, 105, 105, 105
#define LIGHT_GREY		230, 230, 230, 255
#define OP_LIGHT_GREY	230, 230, 230,  25
#define DARK_GREY	 	 69,  69,  69, 255
#define TRANS_DARK_GREY	 69,  69,  69, 105
#define OP_DARK_GREY	 69,  69,  69,  69



enum IC4_SERIAL_API_COMMANDS{
	HIDE_ELEMENT						= 100,
	SHOW_ELEMENT						= 101,
	MOVE_ELEMENT						= 102,
	DELETE_ELEMENT						= 103,
	RESIZE_ELEMENT						= 104,
	RENAME_ELEMENT						= 105,
	UPDATE_VALUE						= 106,
	SET_COLOUR							= 107,
	CONFIG_ELEMENT						= 108,
	DISABLE_ENABLE						= 109,
	SET_FONT_SIZE						= 112,
	ASSIGN_KEYBOARD_SHORTCUT			= 113,

	CONSOLE_MESSAGE						= 114,

	ADD_GUI_PAGE						= 44,
	REMOVE_GUI_PAGE						= 45,
	SHOW_GUI_PAGE						= 46,
	HIDE_GUI_PAGE						= 47,
	ADD_ELEMENT_TO_PAGE					= 48,
	REMOVE_ELEMENT_FROM_PAGE			= 49,

	ADD_FLEXBUTTON						= 20,
	DISABLE_ENABLE_FLEXBUTTON			= 21,
	CHECK_FLEXBUTTON					= 22,

	ADD_FLEXSLIDER						= 30,
	SET_FLEXSLIDER_RANGE				= 31,

	ADD_FLEXLABEL						= 40,

	ADD_FLEXDATA						= 70,

	ADD_FLEXPLOT						= 50,
	SET_WALKING_DOMAIN					= 51,
	PLOT_DATASET						= 52,
	HIDE_DATASET						= 53,
	SHOW_DATASET						= 54,
	SET_RANGE							= 55,
	SET_DOMAIN							= 56,
	SET_MAX_PLOTTED_POINTS				= 57,
	SET_AXES_LABELS						= 58,
	FLEXPLOT_SET_COLOUR					= 59,

	ADD_DATASET							= 60,
	REMOVE_DATASET						= 61,
	ADD_FLOAT_DATA_TO_DATASET			= 62,
	ADD_INT_DATA_TO_DATASET				= 63,
	ADD_TIME_DATA_TO_DATASET			= 69,
	PURGE_DATASET						= 64,
	ASSIGN_DATASET						= 65,
	UNASSIGN_DATASET					= 66,
	DATASET_SET_COLOUR					= 67,
	DATASET_SET_MAX_SIZE				= 68,

	SET_GUI_ROWS_COLS					= 80,
	QUERY_MAX_GRID						= 81,
	SET_MAX_ROWS_COLS					= 82,
	INJECT_DELAY						= 83,
	SET_DEFAULT_COLOUR					= 84,
	RESET_ALL_DEFAULT_COLOURS			= 85,
	RESET_COLOURS_OF_ELEMENT_TYPE		= 86,
	RESET_SPECIFIC_INSTANCE_COLOURS		= 87,
	SET_WINDOW_TITLE					= 88,

	ADD_LOGGER							= 110,
	WRITE_LOGGER						= 111,
};

enum HANDSHAKE_MESSAGES{
	NAME_ENQUIRY				= 5, // \x05 = ENQ 
	HANDSHAKE					= 6, // \x06 = ACK 
	BEGIN_CONNECTION			= 1, // \x01 = SOH 
	END_CONNECTION				= 7  // \x07 = BEL 
};

enum TX_MESSAGE{
};

enum RX_MESSAGES{
	EOT_CHAR					= 4,
	DISCONNECT					= 17,
	DEVICE_STATE_CHECK			= 18,
	LOAD_CONFIG_FILE			= 19,

	FLEXELEMENT_PRESSED			= 33,
	BUTTON_TOGGLED				= 34,
	FLEXELEMENT_UPDATED			= 35,

	GENERIC_MESSAGE 			= 100,
	ERR_HANDLING 				= 101
};
enum RX_GENERIC_MESSAGES{
	GEOMETRY_QUERY				= 150
};

enum RX_ERROR_CODES {
	ERROR_COLLISION				= 1,
	ERROR_OUT_OF_BOUNDS			= 2,
	ERROR_INVALID_SIZE			= 3
};
