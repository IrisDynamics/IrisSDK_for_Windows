/**
 * @file io_elements.h
 * @author Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 * @brief Contains class definitions of GUI objects.
    
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

#include <cstring>
#include "config.h"

#define MAX_DELAY_TIME 100 //the max delay that IC4 can handle without connection issues

class FlexElement;
class FlexButton;
class FlexSlider;
class FlexLabel;
class FlexPlot;
class FlexDropdown;
class MenuOption;
class Dataset;
class DataLog;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// ADMIN CLASSES /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
* @class Console
* @brief The iris Controls console. 
* 
* @note
* Displays messages to the user and accepts text commands.
* Includes the COM channel selector for connecting to devices.
*/
class Console {
public:

	enum ID {
		CONSOLE = 5,
	};

	typedef enum COLOUR_SET_FLAG {
			TITLE_TEXT									= 1,
			TITLE_BACKGROUND							= 2,

			OUTPUT_TEXT									= 3,
			OUTPUT_BACKGROUND							= 4,

			INPUT_TEXT									= 5,
			INPUT_BACKGROUND							= 6,

			CONTROL_BUTTON_TEXT							= 7,
			CONTROL_BUTTON_BACKGROUND					= 8,
			CONTROL_BUTTON_BORDER						= 9,
			CONTROL_BUTTON_PRESSED_TEXT					= 10,
			CONTROL_BUTTON_PRESSED_BACKGROUND			= 11,
			CONTROL_BUTTON_PRESSED_BORDER				= 12,
			CONTROL_BUTTON_HOVER_TEXT					= 13,
			CONTROL_BUTTON_HOVER_BACKGROUND				= 14,
			CONTROL_BUTTON_HOVER_BORDER					= 15,

			CONNECT_BUTTON_TEXT							= 16,
			CONNECT_BUTTON_BACKGROUND					= 17,
			CONNECT_BUTTON_BORDER						= 18,
			CONNECT_BUTTON_CHECKED_TEXT					= 19,
			CONNECT_BUTTON_CHECKED_BACKGROUND			= 20,
			CONNECT_BUTTON_CHECKED_BORDER				= 21,
			CONNECT_BUTTON_PRESSED_TEXT					= 22,
			CONNECT_BUTTON_PRESSED_BACKGROUND			= 23,
			CONNECT_BUTTON_PRESSED_BORDER				= 24,
			CONNECT_BUTTON_HOVER_TEXT					= 25,
			CONNECT_BUTTON_HOVER_BACKGROUND_NORMAL		= 26,
			CONNECT_BUTTON_HOVER_BACKGROUND_CHECKED		= 27,
			CONNECT_BUTTON_HOVER_BORDER					= 28,

			COM_SELECT_TEXT_COLOUR						= 29,
	//		COM_SELECT_BACKGROUND_COLOUR				= 30,
	//		COM_SELECT_BORDER_COLOUR					= 31,
	//		COM_SELECT_CHECKED_BACKGROUND				= 32,
	//		COM_SELECT_HOVER_BORDER						= 33,

			TRAFFIC_PLAIN_TEXT							= 34,
			TRAFFIC_UP									= 35,
			TRAFFIC_DOWN								= 36,
			TRAFFIC_FPS									= 37,
			TRAFFIC_BACKGROUND							= 38
			} colour_set_flag;

	Console();
	~Console();
	void hide();
	void show();
	void move(u16 row, u16 column);
	void resize(u16 rowSpan, u16 columnSpan);
	void set_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	void reset_default_colours();
};

/**
* @class GUI_thing
* @brief Abstract class that contains data and functions that are common across all GUI elements.
*/
class GUI_thing {

public:

	friend class IO_registry;

	GUI_thing() : index(index_assigner++){}

	virtual ~GUI_thing(){};
	virtual void show(){};
	virtual void hide(){};

	virtual void config(u32 config);

	u32 id() { return index; } 

	static int get_index_assigner(){
		return index_assigner;
	}

protected:

	/**
	 * @brief update the element value and return one if the updated value differs from the old value
	 */
	int set	( int v ) 	{

		if (value != v) {
			value = v;
			return 1;
		}
		else {
			return 0;
		}
	}

	const u32 index;
	int value;

private:

	static u32 index_assigner;
};


/**
* @class FlexElement
* @brief New type of GUI element on a grid.
*/
class FlexElement: public GUI_thing{
public:

	enum FLEXELEMENT_TYPE {
		FLEXBUTTON		= 0,
		FLEXSLIDER		= 1,
		FLEXLABEL		= 2,
		FLEXDATA		= 3,
		FLEXPLOT		= 4,
		FLEXDROPDOWN	= 5,
		CONSOLE			= 6,
	};

	typedef enum SUBELEMENT_TYPE {
		LABEL			= 0,
		VALUE			= 1,
		UNITS			= 2,
	} subelement_type;

	FlexElement();
	virtual ~FlexElement();
	void show();
	void hide();
	void move(u16 row, u16 column);
	void resize(u16 rowSpan, u16 columnSpan);
	void rename(const char * name);
	void remove();
	void set_font_size(subelement_type sub_element, u16 font_size);
	void disable(bool status);
	static void reset_all_default_colours();
};

	#define ELEMENT_VALUE_MASK		(1 << 0)
	#define ELEMENT_PRESSED_MASK	(1 << 1)
	#define ELEMENT_TOGGLED_MASK	(1 << 2)

/**
* @class IO_thing
* @brief Abstract class that further extends a FlexElement. These things have input and/or output functionality.
* 
* @note 
* They need to be registered in the IO_register so that serial parser 
* can correctly assign any user changes coming from the GUI.
*/
class IO_thing : public FlexElement {
	friend class IO_registry;

public:
	IO_thing ();
	virtual ~IO_thing ();

	/**
	 * @brief Returns the value of the element correcting for the factor
	 */
	virtual int 	get 	() { update_received(); 	return value;			}
	virtual float 	get_f 	() { update_received(); 	return ((float)value); 	}
		
	int pressed();			// true if the button was pressed by GUI since last check, false otherwise
	
	/**
	* @brief Returns true if the element has received an update since the last time get()/get_f() was called.
	*/
	int new_value_received() { return feedback_register & ELEMENT_VALUE_MASK; }

protected:

	/**
	* @brief Raises the element pressed flag in the feedback register
	*/
	void raise_element_pressed_flag() { feedback_register |= ELEMENT_PRESSED_MASK;	}
	
	/**
	* @brief Raises the element value updated flag
	*/
	void raise_value_updated_flag() { feedback_register |= ELEMENT_VALUE_MASK; }
	
	/**
	* @brief Lowers the value updated flag.
	*/
	void update_received(); // Called when an element update has been received by the user to reset the flag

	IO_thing * next			= 0; // used by the IO_registry to maintain a linked list

	/** 
	* @var u8 feedback_register
	* 8 bit register that tracks feedback from the IrisControls application
	* bit 0 - element value updated
	* bit 1 - element pressed
	* bit 2 - element last toggled state
	*/
	u8 feedback_register = 0;
};

/**
* @class Complex_IO_thing
* @brief Abstract class that further extends a Basic_IO_thing.
* 
* @note
* These things can have denominators and unit conversion factors.
*/
class Complex_IO_thing : public IO_thing {
public:

	u16 my_denominator 		= 1;

	int my_factor			= 1;
	//float inv_factor		= 1; 

	Complex_IO_thing ();
	virtual ~Complex_IO_thing ();

	void config(u32 config);

	int 	get 	();
	float 	get_f 	();

	/**
	 * @brief Sets the denominator which is used to convert units
	 */
	void set_denominator(u16 new_denominator){

		if(my_denominator != new_denominator){
			my_denominator = new_denominator;
//			inv_denominator = 1.0f/my_denominator;
		}
	}

	/**
	 * @brief Sets the factor which is used for conversion between float and an int.
	 * e.g. A 2 decimal place value will have a factor of 100
	 * The config flag uses bits 4, 5, and 6 to assign its precision
	 */
	void set_factor(u16 config){
		if (config & 1 << 4) {
			if (config & 1 << 5) {
				if (config & 1 << 6) my_factor = 10000000;
				else 				 my_factor = 1000;
			}
			else {
				if (config & 1 << 6) my_factor = 100000;
				else 				 my_factor = 10;
			}
		}
		else {
			if (config & 1 << 5) {
				if (config & 1 << 6) my_factor = 1000000;
				else				 my_factor = 100;
			}
			else {
				if (config & 1 << 6) my_factor = 10000;
				else 				 my_factor = 1;
			}
		}
	}
};

/**
* @class IO_registry
* @brief Static linked list of all IO_things. 
* 
* @note
* This is used by the USB parser to deliver IO updates from the GUI to the elements.
*/
class IO_registry {
public:

	enum {
		remove_failed,
		set_failed
	};
	static u32 errors;

    static void add 		(IO_thing * new_io);
    static int remove 		(IO_thing * to_remove);
    static int set 			(u32 id, int value);
    static int set 			(u32 id, float value);
    static int set_element_pressed 	(u32 id);	// indicate a user has pressed this element
    static IO_thing * get	(u32 id);

private:
	static IO_thing * list;
};

/**
* @class GUI_Page
* @brief Class that organises collections of FlexElements into pages
* 
* @note
* The application maintains a list of all FlexElements added to the GUI_page. 
* Calling show or hide methods will result in a single serial command being issued that replaces calling
* show or hide for every element the GUI_page contains.
*/
class GUI_Page : public GUI_thing {

public:

	GUI_Page ();
	~GUI_Page();

	void add	();
	void add	(GUI_Page* parent_gui_page);
	void remove	();
	
	void add_element	(FlexElement* flex_element);
	void remove_element	(FlexElement* flex_element);

	void add_page		(GUI_Page* page);
	void remove_page	(GUI_Page* page);
	
	void show();
	void hide();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// FLEX ELEMENTS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
* @class FlexButton
* @brief An IO element Push Button.
* 
* @note
* Can be configured to be disabled, pressable, or pressable and toggleable.
*/
class FlexButton: public IO_thing{
public:

	typedef enum COLOUR_SET_FLAG {
		TEXT							= 1,
		BACKGROUND						= 2,
		BORDER							= 3,
		CHECKED_TEXT					= 4,
		CHECKED_BACKGROUND				= 5,
		CHECKED_BORDER					= 6,
		PRESSED_TEXT					= 7,
		PRESSED_BACKGROUND				= 8,
		PRESSED_BORDER					= 9,
		HOVER_TEXT						= 10,
		HOVER_BACKGROUND_NORMAL			= 11,
		HOVER_BACKGROUND_CHECKED		= 12,
		HOVER_BORDER					= 13,
		DISABLED_TEXT					= 14,
		DISABLED_BACKGROUND_NORMAL		= 15,
		DISABLED_BACKGROUND_CHECKED		= 16,
		DISABLED_BORDER					= 17,
	} colour_set_flag;

	FlexButton();
	~FlexButton();

	void add(GUI_Page* parent, const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan);
	void add(const char * name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan);
	
	void set_toggle(int t);
	int toggled();		// true if the toggle value is different than last time, false otherwise
	void set_colour(colour_set_flag, u16 r, u16 g, u16 b, u16 a);
	void reset_colours();
	void reset_this_flexbutton_default_colours();
	static void set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	static void reset_all_flexbutton_default_colours();
};

/**
* @class FlexSlider
* @brief A Slider IO element with digital value display.
* 
* @note
* Can be configured to be output only, or act as an input element where users may drag the slider or
* type values into the value field.
* Can be configured to display units.
* Can be configured to handle unit conversions automatically and display up to 7 decimal places.
* Can be configured to be mirrored horizontally.
*/
class FlexSlider: public Complex_IO_thing{
public:

	typedef enum COLOUR_SET_FLAG {
		LABEL_TEXT			= 1,
		LABEL_BACKGROUND	= 2,

		SLIDER_BACKGROUND	= 3,
		SLIDER_SUBPAGE		= 4,
		SLIDER_ADDPAGE		= 5,
		HANDLE_COLOUR		= 6,
		HANDLE_HOVER		= 7,
		HANDLE_PRESSED		= 8,
		HANDLE_DISABLED		= 9,

		VALUE_TEXT			= 10,
		VALUE_BACKGROUND	= 11,

		UNITS_TEXT			= 12,
		UNITS_BACKGROUND	= 13,
	} colour_set_flag;

	FlexSlider();
	~FlexSlider();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config = 0);
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config = 0);

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, u32 config = 0);
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, const char* units, u32 config = 0);

	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config = 0);
	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char * units, u32 config = 0);

	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, u32 config = 0);
	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, const char * units, u32 config = 0);

	int   update(int value);
	float update(float value);
	int update(u16 value) { return update((int)value); }
	int update(u32 value) { return update((int)value); }
#ifndef WINDOWS
	int update(s32 value) { return update((int)value); }
#endif // 

	void set_colour(colour_set_flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexslider_default_colours();
	static void set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	static void reset_all_flexslider_default_colours();

	void set_range(int min, int max);

	enum CONFIG_FLAG {
		TRACKING			= 0b0,
		ALLOW_INPUT			= 0b1 << 0,
		NOT_TRACKING		= 0b1 << 1,
		UNITS				= 0b1 << 2,
		FRAME				= 0b1 << 3,		

		PRECISION_1			= 0b001 << 4,
		PRECISION_2			= 0b010 << 4,
		PRECISION_3			= 0b011 << 4,
		PRECISION_4			= 0b100 << 4,
		PRECISION_5			= 0b101 << 4,
		PRECISION_6			= 0b110 << 4,
		PRECISION_7			= 0b111 << 4,

		MIRRORED			= 0b1 << 7,

		DIGITS_7			= 0 << 13 | 0b000 << 8,		
		DIGITS_1			= 0 << 13 | 0b001 << 8,
		DIGITS_2			= 0 << 13 | 0b010 << 8,
		DIGITS_3			= 0 << 13 | 0b011 << 8,
		DIGITS_4			= 0 << 13 | 0b100 << 8,
		DIGITS_5			= 0 << 13 | 0b101 << 8, 
		DIGITS_6			= 0 << 13 | 0b110 << 8, 
		DIGITS_8			= 0 << 13 | 0b111 << 8,
		DIGITS_10			= 1 << 13 | 0b000 << 8,		
		DIGITS_12			= 1 << 13 | 0b001 << 8,
		DIGITS_14			= 1 << 13 | 0b010 << 8,
		DIGITS_18			= 1 << 13 | 0b011 << 8,
		DIGITS_22			= 1 << 13 | 0b100 << 8,
		DIGITS_26			= 1 << 13 | 0b101 << 8, 
		DIGITS_30			= 1 << 13 | 0b110 << 8, 
		DIGITS_34			= 1 << 13 | 0b111 << 8,	

		BINARY				= 0b01 << 11,
		HEXADECIMAL			= 0b10 << 11,
		UNSIGNED_DECIMAL	= 0b11 << 11,

		//LABEL_ALIGN_LEFT	= 0b01 << 14,
		//LABEL_ALIGN_CENTER= 0b10 << 14,
		//LABEL_ALIGN_RIGHT	= 0b11 << 14,
		
		VALUE_ALIGN_LEFT	= 0b01 << 16,
		VALUE_ALIGN_CENTER	= 0b10 << 16,
		VALUE_ALIGN_RIGHT	= 0b11 << 16,

		//UNITS_ALIGN_LEFT	= 0b01 << 18,
		//UNITS_ALIGN_CENTER= 0b10 << 18,
		//UNITS_ALIGN_RIGHT	= 0b11 << 18,
	};
};

/**
* @class Basic_FlexSlider
* @brief A Basic FlexSlider without support for decimals or unit conversion denominators.
*/
class Basic_FlexSlider: public IO_thing{
public:

	typedef enum COLOUR_SET_FLAG {
		LABEL_TEXT			= 1,
		LABEL_BACKGROUND	= 2,

		SLIDER_BACKGROUND	= 3,
		SLIDER_SUBPAGE		= 4,
		SLIDER_ADDPAGE		= 5,
		HANDLE_COLOUR		= 6,
		HANDLE_HOVER		= 7,
		HANDLE_PRESSED		= 8,
		HANDLE_DISABLED		= 9,

		VALUE_TEXT			= 10,
		VALUE_BACKGROUND	= 11,

		UNITS_TEXT			= 12,
		UNITS_BACKGROUND	= 13,
	} colour_set_flag;

	Basic_FlexSlider();
	~Basic_FlexSlider();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config = 0);
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config = 0);

	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config = 0);
	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char * units, u32 config = 0);

	int update(int value);
	int update(u16 value) { return update((int)value); }
	int update(u32 value) { return update((int)value); }
#ifndef WINDOWS
	int update(s32 value) { return update((int)value); }
#endif 	

	void set_colour(FlexSlider::colour_set_flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexslider_default_colours();
	static void set_default_colour(FlexSlider::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);

	void set_range(int min, int max);

	enum CONFIG_FLAG {
		TRACKING			= 0b0,
		ALLOW_INPUT			= 0b1 << 0,
		NOT_TRACKING		= 0b1 << 1,
		UNITS				= 0b1 << 2,
		FRAME				= 0b1 << 3,		

		MIRRORED			= 0b1 << 7,

		DIGITS_7			= 0 << 13 | 0b000 << 8,		
		DIGITS_1			= 0 << 13 | 0b001 << 8,
		DIGITS_2			= 0 << 13 | 0b010 << 8,
		DIGITS_3			= 0 << 13 | 0b011 << 8,
		DIGITS_4			= 0 << 13 | 0b100 << 8,
		DIGITS_5			= 0 << 13 | 0b101 << 8, 
		DIGITS_6			= 0 << 13 | 0b110 << 8, 
		DIGITS_8			= 0 << 13 | 0b111 << 8,
		DIGITS_10			= 1 << 13 | 0b000 << 8,		
		DIGITS_12			= 1 << 13 | 0b001 << 8,
		DIGITS_14			= 1 << 13 | 0b010 << 8,
		DIGITS_18			= 1 << 13 | 0b011 << 8,
		DIGITS_22			= 1 << 13 | 0b100 << 8,
		DIGITS_26			= 1 << 13 | 0b101 << 8, 
		DIGITS_30			= 1 << 13 | 0b110 << 8, 
		DIGITS_34			= 1 << 13 | 0b111 << 8,	

		BINARY				= 0b01 << 11,
		HEXADECIMAL			= 0b10 << 11,
		UNSIGNED_DECIMAL	= 0b11 << 11,
				
		//LABEL_ALIGN_LEFT	= 0b01 << 14,
		//LABEL_ALIGN_CENTER= 0b10 << 14,
		//LABEL_ALIGN_RIGHT	= 0b11 << 14,
		
		VALUE_ALIGN_LEFT	= 0b01 << 16,
		VALUE_ALIGN_CENTER	= 0b10 << 16,
		VALUE_ALIGN_RIGHT	= 0b11 << 16,

		//UNITS_ALIGN_LEFT	= 0b01 << 18,
		//UNITS_ALIGN_CENTER= 0b10 << 18,
		//UNITS_ALIGN_RIGHT	= 0b11 << 18,
	};
};

/**
* @class FlexLabel
* @brief A Basic text label.
*/
class FlexLabel: public FlexElement{
public:

	typedef enum COLOUR_SET_FLAG {
		TEXT				= 1,
		BACKGROUND			= 2,
		} colour_set_flag;

	FlexLabel();
	~FlexLabel();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config = 0);
	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config = 0);

	void font_size(u16 font_size);
	void set_colour(colour_set_flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexlabel_default_colours();
	static void set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	static void reset_all_flexlabel_default_colours();

	enum CONFIG_FLAG {
		ALIGN_CENTER		= 1,
		ALIGN_RIGHT			= 2,
	};
};

/**
* @class C_FlexLabel
* @brief A FlexLabel that reports user clicks to the device.
*/
class C_FlexLabel: public IO_thing {
public:

	C_FlexLabel();
	~C_FlexLabel();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config = 0);
	void add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config = 0);

	void font_size(u16 font_size);
	void set_colour(FlexLabel::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexlabel_default_colours();
};


/**
* @class FlexData
* @brief An IO element with digital data display.
* 
* @note
* Can be configured to be output only, or act as an input element where users may type values into the value field.
* Can be configured to display units.
* Can be configured to handle unit conversions automatically and display up to 7 decimal places.
* Can be configured to be mirrored horizontally.
*/
class FlexData : public Complex_IO_thing{
public:

	typedef enum COLOUR_SET_FLAG {
			LABEL_TEXT			= 1,
			LABEL_BACKGROUND	= 2,
			VALUE_TEXT			= 3,
			VALUE_BACKGROUND	= 4,
			UNITS_TEXT			= 5,
			UNITS_BACKGROUND	= 6,
		} colour_set_flag;

	FlexData();
	~FlexData();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u32 config = 0);
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char* units, u32 config = 0);
			 
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u32 config = 0);
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char* units, u32 config = 0);

	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u32 config = 0);
	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char * units, u32 config = 0);

	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u32 config = 0);
	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char * units, u32 config = 0);

	int   update(int value);
	float update(float value);
	float update(double value) { return update((float)value); }
	int   update(u16 value) { return update((int)value); }
	int   update(u32 value) { return update((int)value); }
#ifndef WINDOWS
	int   update(s32 value) { return update((int)value); }
#endif 

	void set_colour(colour_set_flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexdata_default_colours();
	static void set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	static void reset_all_flexdata_default_colours();

	enum  CONFIG_FLAG {
		ALLOW_INPUT			= 1 << 0,
		FRAME				= 1 << 1,
		UNITS				= 1 << 2,
		//RESERVED			= 1 << 3,	

		PRECISION_1			= 0b001 << 4,
		PRECISION_2			= 0b010 << 4,
		PRECISION_3			= 0b011 << 4,
		PRECISION_4			= 0b100 << 4,
		PRECISION_5			= 0b101 << 4,
		PRECISION_6			= 0b110 << 4,
		PRECISION_7			= 0b111 << 4,

		MIRRORED			= 1 << 7,

		DIGITS_7			= 0 << 15 | 0b000 << 8,		
		DIGITS_1			= 0 << 15 | 0b001 << 8,
		DIGITS_2			= 0 << 15 | 0b010 << 8,
		DIGITS_3			= 0 << 15 | 0b011 << 8,
		DIGITS_4			= 0 << 15 | 0b100 << 8,
		DIGITS_5			= 0 << 15 | 0b101 << 8, 
		DIGITS_6			= 0 << 15 | 0b110 << 8, 
		DIGITS_8			= 0 << 15 | 0b111 << 8,
		DIGITS_10			= 1 << 15 | 0b000 << 8,		
		DIGITS_12			= 1 << 15 | 0b001 << 8,
		DIGITS_14			= 1 << 15 | 0b010 << 8,
		DIGITS_18			= 1 << 15 | 0b011 << 8,
		DIGITS_22			= 1 << 15 | 0b100 << 8,
		DIGITS_26			= 1 << 15 | 0b101 << 8, 
		DIGITS_30			= 1 << 15 | 0b110 << 8, 
		DIGITS_34			= 1 << 15 | 0b111 << 8,		

		BINARY				= 0b01 << 11,
		HEXADECIMAL			= 0b10 << 11,
		UNSIGNED_DECIMAL	= 0b11 << 11,

		VALUE_ALIGN_LEFT	= 0b01 << 13,
		VALUE_ALIGN_CENTER	= 0b10 << 13,
		VALUE_ALIGN_RIGHT	= 0b11 << 13,
		
		//WIDE VALUE		= 1 << 15

		LABEL_ALIGN_LEFT	= 0b01 << 16,
		LABEL_ALIGN_CENTER	= 0b10 << 16,
		LABEL_ALIGN_RIGHT	= 0b11 << 16,

		UNITS_ALIGN_LEFT	= 0b01 << 18,
		UNITS_ALIGN_CENTER	= 0b10 << 18,
		UNITS_ALIGN_RIGHT	= 0b11 << 18,
	};
};


/**
* @class Basic_FlexData
* @brief Lightweight FlexData without support for decimal places or denominators.
* 
* @note
* Can only accept whole integer values.
*/
class Basic_FlexData : public IO_thing{
public:

	Basic_FlexData();
	~Basic_FlexData();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u32 config = 0);
	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char* units, u32 config = 0);

	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u32 config = 0);
	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char * units, u32 config = 0);

	int update(int value);
	int update(u16 value) { return update((int)value); }
	int update(u32 value) { return update((int)value); }
#ifndef WINDOWS
	int update(s32 value) { return update((int)value); }
#endif // !WINDOWS
	
	void set_colour(FlexData::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexdata_default_colours();

	enum  CONFIG_FLAG {
		ALLOW_INPUT			= 1 << 0,
		FRAME				= 1 << 1,
		UNITS				= 1 << 2,
		//RESERVED			= 1 << 3,		

		MIRRORED			= 1 << 7,

		DIGITS_7			= 0 << 15 | 0b000 << 8,		
		DIGITS_1			= 0 << 15 | 0b001 << 8,
		DIGITS_2			= 0 << 15 | 0b010 << 8,
		DIGITS_3			= 0 << 15 | 0b011 << 8,
		DIGITS_4			= 0 << 15 | 0b100 << 8,
		DIGITS_5			= 0 << 15 | 0b101 << 8, 
		DIGITS_6			= 0 << 15 | 0b110 << 8, 
		DIGITS_8			= 0 << 15 | 0b111 << 8,
		DIGITS_10			= 1 << 15 | 0b000 << 8,		
		DIGITS_12			= 1 << 15 | 0b001 << 8,
		DIGITS_14			= 1 << 15 | 0b010 << 8,
		DIGITS_18			= 1 << 15 | 0b011 << 8,
		DIGITS_22			= 1 << 15 | 0b100 << 8,
		DIGITS_26			= 1 << 15 | 0b101 << 8, 
		DIGITS_30			= 1 << 15 | 0b110 << 8, 
		DIGITS_34			= 1 << 15 | 0b111 << 8,		

		BINARY				= 0b01 << 11,
		HEXADECIMAL			= 0b10 << 11,
		UNSIGNED_DECIMAL	= 0b11 << 11,

		VALUE_ALIGN_LEFT	= 0b01 << 13,
		VALUE_ALIGN_CENTER	= 0b10 << 13,
		VALUE_ALIGN_RIGHT	= 0b11 << 13,
		
		//WIDE VALUE		= 1 << 15

		LABEL_ALIGN_LEFT	= 0b01 << 16,
		LABEL_ALIGN_CENTER	= 0b10 << 16,
		LABEL_ALIGN_RIGHT	= 0b11 << 16,

		UNITS_ALIGN_LEFT	= 0b01 << 18,
		UNITS_ALIGN_CENTER	= 0b10 << 18,
		UNITS_ALIGN_RIGHT	= 0b11 << 18,
	};
};

/**
* @class FlexDropdown
* @brief A dropdown menu GUI element.
* 
* @note
* Is filled with MenuOption objects
*/
class FlexDropdown : public IO_thing {

public:

	typedef enum COLOUR_SET_FLAG {
		DROPDOWN_MENU_TEXT		= 1,
		ACTIVE_OPTION_TEXT		= 2,
	} colour_set_flag;

	FlexDropdown();
	~FlexDropdown();

	void add(GUI_Page* parent, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config = 0);
	void add(u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config = 0);

	void set_menu_item(MenuOption* option);

	void add_option(MenuOption* option, const char* label);
	void remove_option(MenuOption* option);

	void font_size(u16 font_size);

	void set_colour(FlexDropdown::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexdropdown_default_colours();

	static void set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	static void reset_all_flexdata_default_colours();

	enum  CONFIG_FLAG {
		SORT_BY_OPTION_ID	= 1 << 0,	
	};
};

/**
* @class MenuOption
* @brief A FlexDropdown option element.
*
*/
class MenuOption : public GUI_thing {

public:
	MenuOption	() {}
	~MenuOption() {} 
};


/**
* @class FlexPlot
* @brief A data plotting element used to display data from a Dataset.
* 
* @note
* Implements QCustomPlot plots.
* Can contain multiple datasets.
* Can be configured to contain walking data or static data.
* Can be configured to use time as x-axis values.
* Can be configured to have any of a selection of IO elements
*  - Legend
*  - Legend toggle button
*  - Mouse control button (allows users to drag and zoom)
*  - Save data button (saves the Datasets shown to a text file)
*  - Dataset select drop down menu
*  - Axes Label select drop down menu
*  - Graph name label
*
*/
class FlexPlot : public FlexElement{
public:

	typedef enum COLOUR_SET_FLAG {
			TEXT							= 1,
			BACKGROUND						= 2,
			FRAME							= 3,

			BUTTON_TEXT						= 4,
			BUTTON_BACKGROUND				= 5,
			BUTTON_BORDER					= 6,
			BUTTON_CHECKED_TEXT				= 7,
			BUTTON_CHECKED_BACKGROUND		= 8,
			BUTTON_CHECKED_BORDER			= 9,
			BUTTON_PRESSED_TEXT				= 10,
			BUTTON_PRESSED_BACKGROUND		= 11,
			BUTTON_PRESSED_BORDER			= 12,
			BUTTON_HOVER_TEXT				= 13,
			BUTTON_HOVER_BACKGROUND_NORMAL	= 14,
			BUTTON_HOVER_BACKGROUND_CHECKED = 15,
			BUTTON_HOVER_BORDER				= 16,

			COMBOBOX_TEXT_COLOUR			= 17,
			COMBOBOX_BACKGROUND_COLOUR		= 18,
			COMBOBOX_BORDER_COLOUR			= 19,
			COMBOBOX_CHECKED_BACKGROUND		= 20,
			COMBOBOX_HOVER_BORDER			= 21,

			GRID							= 22,
		} colour_set_flag;

	FlexPlot();
	~FlexPlot();

	void add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config = 0);
	void add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config = 0);

	void set_range(float min, float max);
	void set_secondary_range(float min, float max);
	void set_domain(int domain);
	void set_domain(float min, float max);
	void set_visible_datapoints(u16 datapoints);
	void set_axes_labels(Dataset* dataset);

	void set_colour(colour_set_flag, u16 r, u16 g, u16 b, u16 a);
	void reset_this_flexplot_default_colours();
	static void set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a);
	static void reset_all_flexplot_default_colours();

	enum CONFIG_FLAG {
		LEGEND				= 1,
		LEGEND_BUTTON		= 2,
		MOUSE_BUTTON		= 4,
		SAVE_DATA_BUTTON	= 8,
		DATASET_SELECT_MENU	= 16,
		AXES_LABEL_MENU		= 32,
		TIMEPLOT			= 64,
		WALKING				= 128,
		NAME_LABEL			= 256
	};
};

/**
* @class Dataset
* @brief Data container for plotting in FlexPlots
* 
* @note
* Can be configured to appear on the main (left) y-axis or the secondary (right) y-axis
* Can be configured to have time values in the x-axis.
* Can be configured to connect data points with a line or not.
* Can be configured to draw a variety of shapes at each data point.
*/
class Dataset: public GUI_thing{
public:

	Dataset();
	~Dataset();

	void add(FlexPlot * plot, const char * name, const char * _x_label, const char * _y_label, u32 config = 0);
	void set_max_data_points(u32 number_of_data_points);
	void remove();
	void hide();
	void show();
	void plot() { show(); }
	void add_data(int xData, int yData);
#ifndef WINDOWS
	void add_data(s32 xData, s32 yData);
#endif
	void add_data(u64 xData, int yData);
	void add_data(u16 xData, u16 yData) { add_data((int)xData, (int)yData); }
	void add_data(u32 xData, u32 yData) { add_data((int)xData, (int)yData); }
	//void add_data(s32 xData, s32 yData) { add_data((int)xData, (int)yData); }
	void add_data(float xData, float yData);
	void add_data(int dataPairs, int xData[], int yData[]);
	void add_data(int dataPairs, float xData[], float yData[]);
	void purge();
	void assign(FlexPlot * plot);
	void unassign();
	void set_colour(u16 r, u16 g, u16 b, u16 a);

	enum {
		TIMEPLOT				= 1,
		SECONDARY_Y_AXIS		= 2,
		SCATTER_PLOT			= 4,
		CIRCLE					= 8,
		DIAMOND					= 16,
		NONE					= 24
	};
};

/**
* @class DataLog
* @brief Object used to write data to datafiles on the application's host machine.
*/
class DataLog {
	friend class DataLog_registry;

public:

	DataLog() :	my_id(id_assigner++) {}
	~DataLog() {
		close();
	}

	int is_open() { return status; }	

	void add(const char* name);
	void write(const char* string);
	void close();

	u32 id() { return my_id; }

protected:	
		
	u8 status = 0;
	const u32 my_id;

	DataLog* next = 0; // used by the DataLog_registry to maintain a linked list

	int update_status(int new_status) {
		u8 input = (u8)new_status;
		
		if (input == status) return 0;
		status = input;
		return 1;
	}

private:
	static int id_assigner;
};

/**
* @class DataLog_registry
* @brief Static linked list of all DataLogs.
*/
class DataLog_registry {
public:
	
    static void add 			(DataLog* new_datalog);
    static int remove 			(DataLog* to_remove);
	static int update_status	(u32 id, int value);
    static DataLog* get			(u32 id);

private:
	static DataLog* list;
};


