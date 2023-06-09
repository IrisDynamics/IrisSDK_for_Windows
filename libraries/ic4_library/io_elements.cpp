/**
 * @file io_elements.cpp
 * @author Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 * @brief Contains method definitions of GUI object classes.
    
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

#include "io_elements.h"
#include "iriscontrols4.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// GUI Thing Implementation/////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

u32 GUI_thing::index_assigner = 0;

/**
* @fn void GUI_thing::config(u32 config)
* @brief Virtual method. Sets/alters the config of the GUI thing.
* @param[in] u32 config - The config value to be used.
*/
void GUI_thing::config(u32 config) {
	IC4_virtual->gui_thing_config(index, config);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// IO Thing Implementation//////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

IO_thing * 	IO_registry::list 					= 0;
u32 		IO_registry::errors					= 0;

IO_thing::IO_thing() : FlexElement() {}
IO_thing::~IO_thing () {}

/**
* @fn void IO_thing::update_received()
* @brief Raises a flag when an update to an IO_thing has been received.
*/
void IO_thing::update_received(){
	feedback_register &= ~ELEMENT_VALUE_MASK;
}

/**
* @fn int IO_thing::pressed()
* @brief Returns true if the element was pressed since the last time this function was called.
* @param[out] int ret - 1 if the element was pressed since last check, 0 otherwise.
*/
int IO_thing::pressed() {
	int ret = (feedback_register & ELEMENT_PRESSED_MASK) >> 1;
	feedback_register &= ~ELEMENT_PRESSED_MASK;
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Complex IO Thing Implementation//////////////
//////////////////////////////////////////////////////////////////////////////////////////
Complex_IO_thing::Complex_IO_thing() : IO_thing () {}
Complex_IO_thing::~Complex_IO_thing(){}

/**
* @fn void Complex_IO_thing::config(u16 config)
* @brief Sets/alters the config of the Complex IO thing.
* @param[in] u16 config - The config value to be used.
*/
void Complex_IO_thing::config(u32 config) {
	set_factor(config);
	IC4_virtual->gui_thing_config(index, config);
}

/**
* @fn int	Complex_IO_thing::get
* @fn float Complex_IO_thing::get_f
* @brief Returns the value of the element correcting for the factor.
* @param[out] int	- The value of the element.
* @param[out] float - The value of the element.
*/
int 	Complex_IO_thing::get 		() { update_received(); 	if(my_factor == 1) return value; return value / my_factor;	}
float 	Complex_IO_thing::get_f 	() { update_received(); 	return (float)value / (float)my_factor;						}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Console Implementation///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Console::Console(){};
Console::~Console(){};

/**
* @fn void Console::hide()
* @brief Hides the console.
*/
void Console::hide(){ IC4_virtual->flexElement_hide(CONSOLE_ELEMENT_ID); }

/**
* @fn void Console::show()
* @brief Shows the console.
*/
void Console::show(){ IC4_virtual->flexElement_show(CONSOLE_ELEMENT_ID); }

/**
* @fn void Console::move(u16 row, u16 column)
* @brief Moves the console to a new row/column.
* @param[in] u16 row	- The row to move to.
* @param[in] u16 column - The column to move to.
*/
void Console::move(u16 row, u16 column){ IC4_virtual->flexElement_move(CONSOLE_ELEMENT_ID, row, column); }

/**
* @fn void Console::resize(u16 rowSpan, u16 columnSpan)
* @brief Resizes the console.
* @param[in] u16 rowSpan	- The new row span.
* @param[in] u16 columnSpan	- The new column span.
*/
void Console::resize(u16 rowSpan, u16 columnSpan){ IC4_virtual->flexElement_resize(CONSOLE_ELEMENT_ID, rowSpan, columnSpan); }

/**
* @fn void Console::set_colour(colour_set_flag flag,u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour of the selected aspect of the console.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the console to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void Console::set_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(CONSOLE_ELEMENT_ID, (u16)flag, r, g, b, a); }

/**
* @fn void Console::reset_default_colours()
* @brief Resets the console's colours to their defaults.
*/
void Console::reset_default_colours() { IC4_virtual->reset_element_default_colours(CONSOLE); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// GUI Page Implementation///////////////////
//////////////////////////////////////////////////////////////////////////////////////////
GUI_Page::GUI_Page() : GUI_thing() {}
GUI_Page::~GUI_Page() {}

/**
* @fn		void GUI_Page::add()
* @overload void GUI_Page::add(GUI_Page* parent_gui_page)
* @brief Adds a new GUI_Page to the GUI.
* @param[in] GUI_Page* parent_gui_page - Pointer to the page's parent page.
*/
void GUI_Page::add		()							{ IC4_virtual->gui_page_add(index, -1);						}
void GUI_Page::add		(GUI_Page* parent_gui_page) { IC4_virtual->gui_page_add(index, parent_gui_page->id());	}

/**
* @fn void GUI_Page::remove()
* @brief Removes the GUI_Page from the GUI.
*/
void GUI_Page::remove	() { IC4_virtual->gui_page_remove(index); }

/**
* @fn void GUI_Page::add_element(FlexElement* flex_element)
* @brief Adds an element to the GUI Page.
* @param[in] FlexElement* flex_element - Pointer to the element to be added.
*/
void GUI_Page::add_element		(FlexElement* flex_element) { IC4_virtual->gui_page_add_element		(index, flex_element->id()); }

/**
* @fn void GUI_Page::remove_element(FlexElement* flex_element)
* @brief Removed an element from the GUI Page.
* @param[in] FlexElement* flex_element - Pointer to the element to be removed.
*/
void GUI_Page::remove_element	(FlexElement* flex_element) { IC4_virtual->gui_page_remove_element	(index, flex_element->id()); }

/**
* @fn void GUI_Page::add_page(GUI_Page* page)
* @brief Adds a child page to the GUI Page.
* @param[in] GUI_Page* page - Pointer to the page to be added.
*/
void GUI_Page::add_page(GUI_Page* page) { IC4_virtual->gui_page_add_page(index, page->id()); }

/**
* @fn void GUI_Page::remove_page(GUI_Page* page)
* @brief Removed a child page from the GUI Page.
* @param[in] GUI_Page* page - Pointer to the page to be removed.
*/
void GUI_Page::remove_page(GUI_Page* page) { IC4_virtual->gui_page_remove_page(index, page->id()); }


/**
* @fn void GUI_Page::show()
* @brief Reveals all elements in the GUI Page.
*/
void GUI_Page::show() { IC4_virtual->gui_page_show(index); }

/**
* @fn void GUI_Page::hide()
* @brief Hides all elements in the GUI Page.
*/
void GUI_Page::hide() { IC4_virtual->gui_page_hide(index); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexElement Implementation///////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexElement::FlexElement() : GUI_thing(){}
FlexElement::~FlexElement(){}

/**
* @fn void FlexElement::hide()
* @brief Hides the FlexElement with id index.
*/
void FlexElement::hide() { IC4_virtual->flexElement_hide(index); }

/**
* @fn void FlexElement::show()
* @brief Shows the FlexElement with id index.
*/
void FlexElement::show() { IC4_virtual->flexElement_show(index); }

/**
* @fn void FlexElement::move(u16 row, u16 column)
* @brief Moves the FlexElement with id index to the new row and column.
* @param[in] u16 row	- The row to move to.
* @param[in] u16 column	- The column to move to.
*/
void FlexElement::move(u16 row, u16 column) { IC4_virtual->flexElement_move(index, row, column); }

/**
* @fn oid FlexElement::remove()
* @brief Deletes the FlexElement.
*/
void FlexElement::remove() { IC4_virtual->flexElement_remove(index); }

/**
* @fn void FlexElement::resize(u16 rowSpan, u16 columnSpan)
* @brief Resizes the FlexElement with id index to the new row and column spans.
* @param[in] u16 rowSpan	- The new row  span.
* @param[in] u16 columnSpan	- The new column span.
*/
void FlexElement::resize(u16 rowSpan, u16 columnSpan){ IC4_virtual->flexElement_resize(index, rowSpan, columnSpan);}

/**
* @fn void FlexElement::rename(const char * name)
* @brief Renames the FlexElement with id index. This will change the visible label for things like FlexButtons and FlexSlider.
* @param[in] const char * name - The new name.
*/
void FlexElement::rename(const char * name){
	IC4_virtual->flexElement_rename(index, name);
}

/**
* @fn void FlexElement::set_font_size(FlexElement::subelement_type sub_element, u16 font_size)
* @brief Sets the font size of the element
* @param[in] FlexElement::subelement_type sub_element	- The sub-element of the FlexElement to change.
* @param[in] u16 font_size								- The new font size.
*/
void FlexElement::set_font_size(FlexElement::subelement_type sub_element, u16 font_size){
	IC4_virtual->flexElement_set_font_size(index, (u16)sub_element, font_size);
}

/**
* @fn void FlexElement::disable(bool status)
* @brief Disables/Enables the FlexElement.
* @param[in] bool status - True enables, False disables.
* @note
* While disabled: FlexButtons cannot be clicked or toggled, FlexSliders and FlexData are output only.
*/
void FlexElement::disable(bool status) { IC4_virtual->flexElement_disable(index, status); }

/**
* @fn void FlexElement::reset_all_default_colours()
* @brief Resets all elements' default colours
*/
void FlexElement::reset_all_default_colours() { IC4_virtual->reset_all_default_colours(); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexButton Implementation////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexButton::FlexButton() : IO_thing(){ IO_registry::add(this); }
FlexButton::~FlexButton() {	IO_registry::remove(this); }

/**
* @fn		void FlexButton::add(GUI_Page* parent, const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan)
* @overload void FlexButton::add(const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan)
* @brief Adds a new FlexButton to the GUI.
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexButton belongs to.
* @param[in] const char* name	- Label for the button.
* @param[in] u16 initValue		- 0 unchecked, 1 checked, -1 uncheckable
* @param[in] u16 row			- The row of the button.
* @param[in] u16 column			- The column of the button.
* @param[in] u16 rowSpan		- The row span of the button.
* @param[in] u16 columnSpan		- The column span of the button.
* 
* @note
* If the FlexButton had already been added (ie. the index is already assigned) this function will instead make any changes to that FlexButton and then make it visible if hidden.
*/
void FlexButton::add(const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan) { add(NULL, name, initValue, row, column, rowSpan, columnSpan); }
void FlexButton::add(GUI_Page* parent, const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan){
	int parent_id = -1; if (parent) parent_id = parent->id();
	
	value = initValue;
	IC4_virtual->flexButton_add(parent_id, index, name, value, row, column, rowSpan, columnSpan);
}

/**
* @fn int FlexButton::toggled()
* @brief Queries the button to determine if it was toggled since the last time it was queried.
* @param[out] int ret - True if the button was toggled since last time this function was called.
* 
* @note
* Returned value does not reflect the checked status of the button.
* The checked status of the button is stored in the feedback_register (bit 2).
*/
int FlexButton::toggled() {
	int ret = 0;

	if ((feedback_register & ELEMENT_TOGGLED_MASK) >> 2 != value) {

		if (value)  feedback_register |= ELEMENT_TOGGLED_MASK;
		else		feedback_register &= ~ELEMENT_TOGGLED_MASK;

		ret = 1;
	}

	return ret;
}

/**
* @fn void FlexButton::set_toggle (int t)
* @brief Sets the toggle/checked state of the button.
* @param[in] int t - The toggle state to set. 1 toggled, 0 untoggled.
*/
void FlexButton::set_toggle (int t) { if (set(t)) IC4_virtual->flexButton_set_checked(index, t); }

/**
* @fn void FlexButton::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the button to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexButton::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void FlexButton::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the button to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexButton::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXBUTTON, (u16)flag, r, g, b, a);
}

/**
* @fn void FlexButton::reset_this_flexbutton_default_colours()
* @brief Resets the specific instance of the FlexButton to default colours.
*/
void FlexButton::reset_this_flexbutton_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void FlexButton::reset_all_flexbutton_default_colours()
* @brief Resets all FlexButton colours to their defaults.
*/
void FlexButton::reset_all_flexbutton_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXBUTTON); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexSlider Implementation////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexSlider::FlexSlider() : Complex_IO_thing(){ IO_registry::add(this); }
FlexSlider::~FlexSlider() {	IO_registry::remove(this); }

/**
* @fn		void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
* @overload void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config)
* @overload void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config) 
* @overload void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
* @brief Adds a new Flex Slider (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexSlider belongs to.
* @param[in] const char* name	- Label for the slider.
* @param[in] u16 row			- The row of the slider.
* @param[in] u16 column			- The column of the slider.
* @param[in] u16 rowSpan		- The row span of the slider.
* @param[in] u16 columnSpan		- The column span of the slider.
* @param[in] int min			- The min value of the slider.
* @param[in] int max			- The max value of the slider.
* @param[in] int initValue		- The initial value of the slider.
* @param[in] u16 denominator	- Factor used for unit conversion.
* @param[in] const char* units	- The units of the slider.
* @param[in] u32 config			- Config value. 
*/
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config)				{ add(parent, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config) 								{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config) 				{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator,  units, config);	}
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
{	
	int parent_id = -1; if (parent) parent_id = parent->id();

	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexSlider_add(parent_id, index, (int)my_denominator, name, row, column,rowSpan, columnSpan, min * my_factor, max * my_factor, value, units, config);
}

/**
* @fn		void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
* @overload void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config)
* @overload void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config)
* @overload void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
* @brief Adds a new Flex Slider (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexSlider belongs to.
* @param[in] const char* name	- Label for the slider.
* @param[in] u16 row			- The row of the slider.
* @param[in] u16 column			- The column of the slider.
* @param[in] u16 rowSpan		- The row span of the slider.
* @param[in] u16 columnSpan		- The column span of the slider.
* @param[in] double min			- The min value of the slider.
* @param[in] double max			- The max value of the slider.
* @param[in] double initValue	- The initial value of the slider.
* @param[in] u16 denominator	- Factor used for unit conversion.
* @param[in] const char* units	- The units of the slider.
* @param[in] u32 config			- Config value.
*/
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, u32 config)		{ add(parent, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, u32 config)						{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, const char * units, u32 config)	{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, units, config);	}
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, const char* units, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexSlider_add(parent_id, index, (int)my_denominator, name, row, column,rowSpan, columnSpan, (int)(min * my_factor), (int)(max * my_factor), value, units, config);
}



/**
* @fn int   FlexSlider::update(int new_value)
* @fn float FlexSlider::update(int new_value)
* @brief Updates the value of the Flex Slider.
* @param[in] int   new_value - The new value.
* @param[in] float new_value - The new value.
*/
int   FlexSlider::update(int new_value)		{ if (set(new_value * my_factor)) 		 IC4_virtual->flexElement_update(index, value); return value;																		}
float FlexSlider::update(float new_value)	{ if (set((int)(new_value * my_factor))) IC4_virtual->flexElement_update(index, value); if (my_factor ==1 ) return (float)value; return (float)value / (float)my_factor;	}

/**
* @fn void FlexSlider::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the slider to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexSlider::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void FlexSlider::reset_this_flexslider_default_colours()
* @brief Resets the specific instance of the FlexSlider to default colours.
*/
void FlexSlider::reset_this_flexslider_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void FlexSlider::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the slider to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value. 
*/
void FlexSlider::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXSLIDER, (u16)flag, r, g, b, a);
}

/**
* @fn void FlexSlider::reset_all_flexslider_default_colours()
* @brief Resets all FlexSlider colours to their defaults
*/
void FlexSlider::reset_all_flexslider_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXSLIDER); }

/**
* @fn void FlexSlider::set_range(int min, int max)
* @brief Sets the slider's minimum and maximum values
* @param[in] int min - The new minimum value. 
* @param[in] int max - The new maximum value.
*/
void FlexSlider::set_range(int min, int max) { IC4_virtual->flexSlider_set_range(index, (min * my_factor), (max * my_factor)); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Basic FlexSlider Implementation//////////////
//////////////////////////////////////////////////////////////////////////////////////////
Basic_FlexSlider::Basic_FlexSlider() : IO_thing(){ IO_registry::add(this); }
Basic_FlexSlider::~Basic_FlexSlider() {	IO_registry::remove(this); }

/**
* @fn		void Basic_FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
* @overload void Basic_FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config)
* @overload void Basic_FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config) 
* @overload void Basic_FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char * units, u32 config)
* @brief Adds a new Flex Slider (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the Basic FlexSlider belongs to.
* @param[in] const char* name	- Label for the slider.
* @param[in] u16 row			- The row of the slider.
* @param[in] u16 column			- The column of the slider.
* @param[in] u16 rowSpan		- The row span of the slider.
* @param[in] u16 columnSpan		- The column span of the slider.
* @param[in] int min			- The min value of the slider.
* @param[in] int max			- The max value of the slider.
* @param[in] int initValue		- The initial value of the slider.
* @param[in] u16 denominator	- Factor used for unit conversion.
* @param[in] const char* units	- The units of the slider.
* @param[in] u32 config			- Config value. 
*/
void Basic_FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config)			{ add(parent, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void Basic_FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u32 config) 							{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void Basic_FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char * units, u32 config)		{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator,  units, config);	}
void Basic_FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();
	set(initValue);
	IC4_virtual->flexSlider_add(parent_id, index, 1, name, row, column,rowSpan, columnSpan, min, max, value, units, config);
}

/**
* @fn int Basic_FlexSlider::update(int new_value)
* @brief Updates the value of the Flex Slider.
* @param[in] int new value	- The new value of the slider.
* @param[out] int			- The value of the slider after attempting to set it. 
*/
int Basic_FlexSlider::update(int new_value)	{ if (set(new_value)) 		 IC4_virtual->flexElement_update(index, value); return value; }

/**
* @fn void Basic_FlexSlider::set_colour(FlexSlider::colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] FlexSlider::colour_set_flag flag	- A flag corresponding to which aspect of the slider to recolour.
* @param[in] u16 r								- The new red value.
* @param[in] u16 g								- The new green value.
* @param[in] u16 b								- The new blue value.
* @param[in] u16 a								- The new alpha value. 
*/
void Basic_FlexSlider::set_colour(FlexSlider::colour_set_flag config, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void Basic_FlexSlider::reset_this_flexslider_default_colours()
* @brief Resets the specific instance of the Basic_FlexSlider to default colours.
*/
void Basic_FlexSlider::reset_this_flexslider_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void Basic_FlexSlider::set_default_colour(FlexSlider::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element.
* @param[in] FlexSlider::colour_set_flag flag	- A flag corresponding to which aspect of the slider to recolour.
* @param[in] u16 r								- The new red value.
* @param[in] u16 g								- The new green value.
* @param[in] u16 b								- The new blue value.
* @param[in] u16 a								- The new alpha value.
*/
void Basic_FlexSlider::set_default_colour(FlexSlider::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXSLIDER, (u16)flag, r, g, b, a);
}

/**
* @fn void Basic_FlexSlider::set_range(int min, int max)
* @brief Sets the slider's minimum and maximum values.
* @param[in] int min - The minimum value of the slider.
* @param[in] int max - The maximum value of the slider.
*/
void Basic_FlexSlider::set_range(int min, int max) { IC4_virtual->flexSlider_set_range(index, min, max); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexLabel Implementation/////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexLabel::FlexLabel() : FlexElement(){}
FlexLabel::~FlexLabel() {}

/**
* @fn		void FlexLabel::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
* @overload void FlexLabel::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
* @brief Adds a new Flex Label (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexLabel belongs to.
* @param[in] const char* name	- Label for the label.
* @param[in] u16 row			- The row of the slider.
* @param[in] u16 column			- The column of the slider.
* @param[in] u16 rowSpan		- The row span of the slider.
* @param[in] u16 columnSpan		- The column span of the slider.
* @param[in] u32 config			- Config value.  
*/
void FlexLabel::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config) { add(NULL, name, row, column, rowSpan, columnSpan, config); }
void FlexLabel::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();
	value = 0;
	IC4_virtual->flexLabel_add(parent_id, index, name, row, column, rowSpan, columnSpan, config);
}

/**
* @fn void FlexLabel::font_size(u16 font_size)
* @brief Sets the font size of the FlexLabel's text.
* @param[in] u16 font_size - The new font size.
*/
void FlexLabel::font_size(u16 font_size) {
	IC4_virtual->flexElement_set_font_size(index, (u16)FlexElement::subelement_type::LABEL, font_size);
}

/**
* @fn void FlexLabel::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the label to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexLabel::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void FlexLabel::reset_this_flexlabel_default_colours()
* @brief Resets the specific instance of the FlexLabel to default colours.
*/
void FlexLabel::reset_this_flexlabel_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void FlexLabel::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element. 
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the label to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexLabel::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a) {
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXLABEL, (u16)flag, r, g, b, a);
}

/**
* @fn void FlexLabel::reset_all_flexlabel_default_colours()
* @brief Resets all FlexLabel colours to their defaults.
*/
void FlexLabel::reset_all_flexlabel_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXLABEL); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Clickable FlexLabel Implementation///////////
//////////////////////////////////////////////////////////////////////////////////////////
C_FlexLabel::C_FlexLabel() : IO_thing(){ IO_registry::add(this); }
C_FlexLabel::~C_FlexLabel() { IO_registry::remove(this); }

/**
* @fn		void C_FlexLabel::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
* @overload void C_FlexLabel::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
* @brief Adds a new Clickable Flex Label (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexLabel belongs to.
* @param[in] const char* name	- Label for the label.
* @param[in] u16 row			- The row of the slider.
* @param[in] u16 column			- The column of the slider.
* @param[in] u16 rowSpan		- The row span of the slider.
* @param[in] u16 columnSpan		- The column span of the slider.
* @param[in] u32 config			- Config value. 
*/
void C_FlexLabel::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config) { add(NULL, name, row, column, rowSpan, columnSpan, config); }
void C_FlexLabel::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	value = 0;
	IC4_virtual->flexLabel_add(parent_id, index, name, row, column, rowSpan, columnSpan, config);
}

/**
* @fn void C_FlexLabel::font_size(u16 font_size)
* @brief Sets the font size of the FlexLabel's text.
* @param[in] u16 font_size - The new font size.
*/
void C_FlexLabel::font_size(u16 font_size) {
	IC4_virtual->flexElement_set_font_size(index, (u16)FlexElement::subelement_type::LABEL, font_size);
}

/**
* @fn void C_FlexLabel::set_colour(FlexLabel::colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] FlexLabel::colour_set_flag flag	- A flag corresponding to which aspect of the label to recolour.
* @param[in] u16 r								- The new red value.
* @param[in] u16 g								- The new green value.
* @param[in] u16 b								- The new blue value.
* @param[in] u16 a								- The new alpha value.
*/
void C_FlexLabel::set_colour(FlexLabel::colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void C_FlexLabel::reset_this_flexlabel_default_colours()
* @brief Resets the specific instance of the FlexLabel to default colours.
*/
void C_FlexLabel::reset_this_flexlabel_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexData Implementation//////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexData::FlexData() : Complex_IO_thing(){ IO_registry::add(this); }
FlexData::~FlexData() {	IO_registry::remove(this); }

/**
* @fn		void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char* units, u32 config)
* @overload void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u32 config)
* @overload void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u32 config)
* @overload void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char * units, u32 config)
* @brief Adds a new Flex Data (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexData belongs to.
* @param[in] const char* name	- Label for the element.
* @param[in] u16 row			- The row of the element.
* @param[in] u16 column			- The column of the element.
* @param[in] u16 rowSpan		- The row span of the element.
* @param[in] u16 columnSpan		- The column span of the element.
* @param[in] int initValue		- The initial value of the element.
* @param[in] u16 denominator	- Factor used for unit conversion.
* @param[in] const char* units	- The units of the element.
* @param[in] u32 config			- Config value. 
*/
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u32 config)		{ add(parent, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u32 config) 							{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char * units, u32 config) 		{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator,  units, config);	}
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char* units, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexData_add(parent_id, index, name, row, column, rowSpan, columnSpan, value, units, (int)(my_denominator), config);
}

/**
* @fn		void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char* units, u32 config)
* @overload void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u32 config)
* @overload void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u32 config)
* @overload void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char * units, u32 config)
* @brief Adds a new Flex Data (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexData belongs to.
* @param[in] const char* name	- Label for the element.
* @param[in] u16 row			- The row of the element.
* @param[in] u16 column			- The column of the element.
* @param[in] u16 rowSpan		- The row span of the element.
* @param[in] u16 columnSpan		- The column span of the element.
* @param[in] double initValue	- The initial value of the element.
* @param[in] u16 denominator	- Factor used for unit conversion.
* @param[in] const char* units	- The units of the element.
* @param[in] u32 config			- Config value.
*/
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u32 config)		{ add(parent, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u32 config)						{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}	
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char * units, u32 config) 	{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator,  units, config);	}
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char* units, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexData_add(parent_id, index, name, row, column, rowSpan, columnSpan, value, units, (int)(my_denominator), config);
}

/**
* @fn int   FlexData::update(int new_value)
* @fn float FlexData::update(float new_value)
* @brief Updates the value of the Flex Data
* @param[in] int   new_value - The new value.
* @param[in] float new_value - The new value.
*/
int   FlexData::update(int new_value) 	{ if (set(new_value * my_factor)) 		 IC4_virtual->flexElement_update(index, value); return value;																	}
float FlexData::update(float new_value)	{ if (set((int)(new_value * my_factor))) IC4_virtual->flexElement_update(index, value);	if(my_factor == 1) return (float)value; return (float)value / (float)my_factor;	}


/**
* @fn void FlexData::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the element to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexData::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void FlexData::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the element to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexData::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->set_default_colour((u16)FlexElement::FLEXDATA, (u16)flag, r, g, b, a); }

/**
* @fn void FlexData::reset_this_flexdata_default_colours() 
* @brief Resets the specific instance of the FlexLabel to default colours.
*/
void FlexData::reset_this_flexdata_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void FlexData::reset_all_flexdata_default_colours()
* @brief Resets all FlexData colours to their defaults
*/
void FlexData::reset_all_flexdata_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXDATA); }

////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Basic FlexData Implementation//////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
Basic_FlexData::Basic_FlexData() : IO_thing(){ IO_registry::add(this); }
Basic_FlexData::~Basic_FlexData() {	IO_registry::remove(this); }

/**
* @fn		void Basic_FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char* units, u32 config)
* @overload void Basic_FlexData::add(GUI_Page *parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u32 config)
* @overload void Basic_FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u32 config)
* @overload void Basic_FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char * units, u32 config)
* @brief Adds a new Flex Data (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexData belongs to.
* @param[in] const char* name	- Label for the element.
* @param[in] u16 row			- The row of the element.
* @param[in] u16 column			- The column of the element.
* @param[in] u16 rowSpan		- The row span of the element.
* @param[in] u16 columnSpan		- The column span of the element.
* @param[in] int initValue		- The initial value of the element.
* @param[in] const char* units	- The units of the element.
* @param[in] u32 config			- Config value. 
*/
void Basic_FlexData::add(GUI_Page *parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u32 config)		{ add(parent, name, row, column, rowSpan, columnSpan, initValue, ""		, config);	}
void Basic_FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u32 config) 						{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, ""		, config);	}
void Basic_FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char * units, u32 config) 	{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, units	, config);	}
void Basic_FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char* units, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	set(initValue);
	IC4_virtual->flexData_add(parent_id, index, name, row, column, rowSpan, columnSpan, value, units, 1, config);
}

/**
* @fn int Basic_FlexData::update(int new_value)
* @brief Updates the value of the Flex Data.
* @param[in] int new_value - The new value.
*/
int Basic_FlexData::update(int new_value) 		{ if (set(new_value)) 		 IC4_virtual->flexElement_update(index, value);	return value;	}

/**
* @fn void Basic_FlexData::set_colour(FlexData::colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element
* @param[in] FlexData::colour_set_flag flag	- A flag corresponding to which aspect of the label to recolour.
* @param[in] u16 r							- The new red value.
* @param[in] u16 g							- The new green value.
* @param[in] u16 b							- The new blue value.
* @param[in] u16 a							- The new alpha value.
*/
void Basic_FlexData::set_colour(FlexData::colour_set_flag config, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void Basic_FlexData::reset_this_flexdata_default_colours()
* @brief Resets the specific instance of the Basic_FlexData to default colours.
*/
void Basic_FlexData::reset_this_flexdata_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexDropdown Implementation//////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexDropdown::FlexDropdown() : IO_thing() { IO_registry::add(this); };
FlexDropdown::~FlexDropdown() { IO_registry::remove(this); };

/**
* @fn		void FlexDropdown::add(GUI_Page* parent, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
* @overload void FlexDropdown::add(u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config)
* @brief Adds a new Flex Dropdown (or alters and then shows an existing one).
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexDropdown belongs to.
* @param[in] u16 row			- The row of the element.
* @param[in] u16 column			- The column of the element.
* @param[in] u16 rowSpan		- The row span of the element.
* @param[in] u16 columnSpan		- The column span of the element.
* @param[in] u32 config			- Config value.
*/
void FlexDropdown::add(u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config) { add(NULL, row, column, rowSpan, columnSpan, config); }
void FlexDropdown::add(GUI_Page* parent, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config) {
	int parent_id = -1; if (parent) parent_id = parent->id();
		
	IC4_virtual->flexDropdown_add(parent_id, index, row, column, rowSpan, columnSpan, config);
}

/**
* @fn void FlexDropdown::set_menu_item(MenuOption* option)
* @brief Sets the menu to the specified item.
* @param[in] MenuOption* option - Pointer to the option to set the menu to.
*/
void FlexDropdown::set_menu_item(MenuOption* option) { 
	IO_registry::set(index, (int)option->id());
	IC4_virtual->flexElement_update(index, option->id()); 
}

/**
* @fn void FlexDropdown::add_option(MenuOption* option)
* @brief Adds an option to the FlexDropdown.
* @param[in] MenuOption* option - The option to add.
*/
void FlexDropdown::add_option(MenuOption* option, const char* label) { IC4_virtual->flexDropdown_add_option(id(), option->id(), label); }

/**
* @fn void FlexDropdown::remove_option(MenuOption* option)
* @brief Removes an option from the FlexDropdown.
* @param[in] MenuOption* option - The option to remove.
*/
void FlexDropdown::remove_option(MenuOption* option) { IC4_virtual->flexDropdown_remove_option(id(), option->id()); }

/**
* @fn void FlexDropdown::font_size(u16 font_size)
* @brief Sets the font size of the FlexDropdown's text.
* @param[in] u16 font_size - The font size.
*/
void FlexDropdown::font_size(u16 font_size) { IC4_virtual->flexElement_set_font_size(index, (u16)FlexElement::subelement_type::VALUE, font_size); }

/**
* @fn void FlexDropdown::set_colour(FlexData::colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element
* @param[in] FlexDropdown::colour_set_flag flag	- A flag corresponding to which aspect of the label to recolour.
* @param[in] u16 r								- The new red value.
* @param[in] u16 g								- The new green value.
* @param[in] u16 b								- The new blue value.
* @param[in] u16 a								- The new alpha value.
*/
void FlexDropdown::set_colour(FlexDropdown::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->flexElement_setColour(index, (u16)flag, r, g, b, a); }

/**
* @fn void FlexDropdown::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the element to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexDropdown::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->set_default_colour((u16)FlexElement::FLEXDROPDOWN, (u16)flag, r, g, b, a); }


/**
* @fn void FlexDropdown::reset_this_flexdropdown_default_colours()
* @brief Resets the specific instance of the FlexDropdown to default colours.
*/
void FlexDropdown::reset_this_flexdropdown_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void FlexDropdown::reset_all_flexdata_default_colours()
* @brief Resets all FlexData colours to their defaults
*/
void FlexDropdown::reset_all_flexdata_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXDROPDOWN); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexPlot Implementation//////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexPlot::FlexPlot() : FlexElement(){}
FlexPlot::~FlexPlot(){}

/**
* @fn		void FlexPlot::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config)
* @overload void FlexPlot::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config)
* @brief Adds a new Flex Plot (or alters and then shows an existing one)
* @param[in] GUI_Page* parent	- Pointer to the GUI Page the FlexPlot belongs to.
* @param[in] const char* name	- Label for the plot.
* @param[in] u16 row			- The row of the plot.
* @param[in] u16 column			- The column of the plot.
* @param[in] u16 rowSpan		- The row span of the plot.
* @param[in] u16 columnSpan		- The column span of the plot.
* @param[in] float min			- The minimum value of the y-axis.
* @param[in] float max			- The maximum value of the y-axis.
* @param[in] const char* units	- The units of the element.
* @param[in] u32 config			- Config value. 
*/
void FlexPlot::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config) { add(NULL, name, row, column, rowSpan, columnSpan, min, max, config); }
void FlexPlot::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	value = 0;
	IC4_virtual->flexPlot_add(parent_id, index, name, row, column, rowSpan, columnSpan, min, max, config);
}

/**
* @fn void FlexPlot::set_range (float min, float max)
* @brief Sets/alters the range of the Flex Plot (min and max y-value shown).
* @param[in] float min - The new minimum value of the y-axis. 
* @param[in] float max - The new maximum value of the y-axis. 
*/
void FlexPlot::set_range			(float min, float max) { IC4_virtual->flexPlot_set_range(index, 0, min, max); }

/**
* @fn void FlexPlot::set_secondary_range (float min, float max)
* @brief Sets/alters the secondary range (right side) of the Flex Plot (min and max y-value shown).
* @param[in] float min - The new minimum value of the y-axis.
* @param[in] float max - The new maximum value of the y-axis.
*/
void FlexPlot::set_secondary_range	(float min, float max) { IC4_virtual->flexPlot_set_range(index, 1, min, max); }

/**
* @fn		void FlexPlot::set_domain			(float min, float max)
* @overload void FlexPlot::set_domain(int domain) 
* @brief Sets/alters the domain of the Flex Plot (min and max x-value shown).
* @param[in] float min	- The new minimum value of the x-axis.
* @param[in] float max	- The new maximum value of the x-axis.
* @param[in] int domain	- The new size of the domain.
*/
void FlexPlot::set_domain(float min, float max) { IC4_virtual->flexPlot_set_domain(index, min, max	); }
void FlexPlot::set_domain(int domain) 			{ IC4_virtual->flexPlot_set_domain(index, domain	); }

/**
* @fn void FlexPlot::set_visible_datapoints(u16 datapoints)
* @brief Sets/alters the number of visible data points of the Flex Plot.
* @param[in] u16 datapoints - The maximum number of datapoints to keep visible. 
*/
void FlexPlot::set_visible_datapoints(u16 datapoints) { IC4_virtual->flexPlot_set_visible_datapoints(index, datapoints); }

/**
* @fn void FlexPlot::set_axes_labels(Dataset* dataset)
* @brief Sets/alters from which Dataset the axes labels of the Flex Plot are taken.
* @param[in] Dataset* dataset - Pointer to the Dataset from which to use its labels.
*/
void FlexPlot::set_axes_labels(Dataset* dataset) { IC4_virtual->flexPlot_set_axes_labels(index, dataset->id()); }

/**
* @fn void FlexPlot::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour property of the Flex Element.
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the plot to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexPlot::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a) { IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @fn void FlexPlot::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a)
* @brief Sets the default colour property of the element
* @param[in] colour_set_flag flag	- A flag corresponding to which aspect of the plot to recolour.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void FlexPlot::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a) {
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXPLOT, (u16)flag, r, g, b, a);
}

/**
* @fn void FlexPlot::reset_this_flexplot_default_colours()
* @brief Resets the specific instance of the FlexPlot to default colours.
*/
void FlexPlot::reset_this_flexplot_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @fn void FlexPlot::reset_all_flexplot_default_colours()
* @brief Resets all FlexPlot colours to their defaults.
*/
void FlexPlot::reset_all_flexplot_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXPLOT); }
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Dataset Implementation///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Dataset::Dataset() : GUI_thing() {}
Dataset::~Dataset() {}

/**
* @fn void Dataset::add(FlexPlot * plot, const char * name, const char * x_label, const char * y_label, u32 config)
* @brief Adds a new Dataset (or alters and then shows an existing one).
* @param[in] FlexPlot* plot			- A pointer to the FlexPlot the Dataset is to be assgined to.
* @param[in] const char* name		- The name of the Dataset.
* @param[in] const char* x_label	- The label of the x-axis.
* @param[in] const char* y_label	- The label of the y-axis.
* @param[in] u32 config				- The config value of the Dataset.
*/
void Dataset::add(FlexPlot * plot, const char * name, const char * x_label, const char * y_label, u32 config) {
	IC4_virtual->dataset_add(index, plot->id(), name, x_label, y_label, config);
}

/**
* @fn void Dataset::remove()
* @brief Removes/deletes a Dataset and all its data.
* 
* @note
* Irreversible.
*/
void Dataset::remove() {  IC4_virtual->dataset_remove(index); }

/**
* @fn void Dataset::add_data(int dataPairs, int xData[], int yData[])
* @overload void Dataset::add_data(int xData, int yData)
* @overload void Dataset::add_data(s32 xData, s32 yData)
* @brief Adds data points to a Dataset.
* @param[in] int xData		- The x value at the datapoint.
* @param[in] int yData		- The y value at the datapoint.
* @param[in] s32 xData		- The x value at the datapoint.
* @param[in] s32 yData		- The y value at the datapoint.
* @param[in] u64 xData		- The x value at the datapoint.
* @param[in] int dataPairs	- The number of datapoints.
* @param[in] int[] xData	- Array of x-values.
* @param[in] int[] yData	- Array of y-values.
*/
void Dataset::add_data(int xData, int yData) {	IC4_virtual->dataset_add_int_data(index, xData, yData); }
#ifndef WINDOWS
void Dataset::add_data(s32 xData, s32 yData) {	IC4_virtual->dataset_add_int_data(index, xData, yData); }
#endif
void Dataset::add_data(int dataPairs, int xData[], int yData[]) {
	for(int i=0; i< dataPairs; i++){
		IC4_virtual->dataset_add_int_data(index, xData[i], yData[i]);
	}
}
void Dataset::add_data(u64 xData, int yData) { IC4_virtual->dataset_add_int_data(index, xData, yData); }

/**
* @fn void Dataset::add_data(int dataPairs, float xData[], float yData[])
* @overload void Dataset::add_data(float xData, float yData)
* @brief Adds data points to a Dataset.
* @param[in] float xData	- The x value at the datapoint.
* @param[in] float yData	- The y value at the datapoint.
* @param[in] int dataPairs	- The number of datapoints.
* @param[in] float[] xData	- Array of x-values.
* @param[in] float[] yData	- Array of y-values.
*/
void Dataset::add_data(int dataPairs, float xData[], float yData[]) { IC4_virtual->dataset_add_float_data(index, dataPairs, xData, yData); }
void Dataset::add_data(float xData, float yData) {
	float x[1] = {xData};
	float y[1] = {yData};
	IC4_virtual->dataset_add_float_data(index, 1, x, y);
}

/**
* @fn void Dataset::set_max_data_points(u32 number_of_data_points)
* @brief Sets the maximum number of datapoints retained by Iris Controls before doing FIFO deletion.
* @param[in] u32 number_of_data_points - The maximum size of the Dataset.
*/
void Dataset::set_max_data_points(u32 number_of_data_points) { IC4_virtual->dataset_set_max_data_points(index, number_of_data_points); }

/**
* @fn void Dataset::hide()
* @brief Hides the Dataset - i.e., makes it invisible on its Flex Plot.
*/
void Dataset::hide() { IC4_virtual->dataset_hide(index); }

/**
* @fn void Dataset::show()
* @brief Shows the Dataset - i.e., makes it visible on its Flex Plot.
*/
void Dataset::show() { IC4_virtual->dataset_show(index); }

/**
* @fn void Dataset::purge()
* @brief Purges the Dataset of all data.
* 
* @note
* Permanently deletes all the data. Irreversible.
*/
void Dataset::purge() { IC4_virtual->dataset_purge(index); }

/**
* @fn void Dataset::assign(FlexPlot* plot)
* @brief Assigns/re-assigns a dataset to a FlexPlot.
* @param[in] FlexPlot* - A pointer to the FlexPlot to which to assign the Dataset.
*/
void Dataset::assign(FlexPlot* plot) { IC4_virtual->dataset_assign(index, plot->id()); }

/**
* @fn void Dataset::unassign()
* @brief Un-assigns the dataset from its FlexPlot.
*/
void Dataset::unassign() {  IC4_virtual->dataset_unassign(index); }

/**
* @fn void Dataset::set_colour(u16 r, u16 g, u16 b, u16 a)
* @brief Sets the colour of the Dataset.
* @param[in] u16 r					- The new red value.
* @param[in] u16 g					- The new green value.
* @param[in] u16 b					- The new blue value.
* @param[in] u16 a					- The new alpha value.
*/
void Dataset::set_colour(u16 r, u16 g, u16 b, u16 a) { IC4_virtual->dataset_set_colour(index,r, g, b, a); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// DataLog Implementation //////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

int DataLog::id_assigner = 1;

/**
* @fn DataLog::add(const char * name) { IC4_virtual->datalog_add(my_id, name)
* @brief Adds a new DataLog.
* @param[in] const char * name - The filename for the DataLog.
*/
void DataLog::add(const char* name) { IC4_virtual->datalog_add(my_id, name); }

/**
* @fn DataLog::write(const char * string)
* @brief Writes to an existing DataLog.
* param[in] const char * string - The string to write to the DataLog.
*/
void DataLog::write(const char* string) { IC4_virtual->datalog_write(my_id, string); }

/**
* @fn DataLog::close()
* @brief Closes the DataLog.
*/
void DataLog::close() { IC4_virtual->datalog_close(my_id); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// DataLog Registry Implementation /////////////
//////////////////////////////////////////////////////////////////////////////////////////

DataLog* DataLog_registry::list = 0;

/**
* @fn void DataLog_registry::add(IO_thing * new_io)
* @brief Adds a DataLog to the linked list of all DataLogs.
* @param[in] DataLog* new_datalog - A pointer to the DataLog to add.
*/
void DataLog_registry::add(DataLog* new_datalog) {
	if (!list) { list = new_datalog; }
	else {
		new_datalog->next = list;
		list = new_datalog;
	}
}

/**
* @fn int DataLog_registry::remove(IO_thing * to_remove)
* @brief Removes a DataLog from the linked list of all DataLogs.
* @param[in]  DataLog* to_remove	- A pointer to the DataLog to remove.
* @param[out] int					- 1 if successful, 0 if not.
*/
int DataLog_registry::remove(DataLog* to_remove) {
	if (!to_remove) 			return 0;
	if (!list) 					return 0;

	DataLog* p = list;
	if (list->id() == to_remove->id()) {
		list = list->next;
		return 1;
	}
	while (p) {
		if (p->next && p->next->id() == to_remove->id()) {
			p->next = p->next->next;
			return 1;
		}
		p = p->next;
	}
	return 0;
}

/**
* @fn int DataLog_registry::update_status(int id, int value)
* @brief Finds a DataLog and sets its value.
* @param[in]  int id	- The ID of the DataLog to update.
* @param[in]  int value	- The new value.
* @param[out] int 		- 1 if successful, 0 if not.
*
* @note
* Called when parsing a serial message from the IrisControls application.
*/
int DataLog_registry::update_status(u32 id, int value) {
	if (!list) 			return 0;
	DataLog* p = list;
	while (p) {
		if (p->id() == id) {
			p->update_status(value);
			return 1;
		}
		p = p->next;
	}
	return 0;
}

/**
* @fn DataLog* DataLog::get(int id)
* @brief Finds a DataLog from the linked list of all DataLogs.
* @param[in]  int id	- The ID of the DataLog to find.
* @param[out] DataLog*	- A pointer to the DataLog if successful, 0 if not.
*/
DataLog* DataLog_registry::get(u32 id) {
	for (DataLog* p = list; p; p = p->next) {
		if (p->id() == id) return p;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// IO Registry Implementation //////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/**
* @fn IO_thing* IO_registry::get (int id)
* @brief Finds an IO_thing from the linked list of all elements.
* @param[in]  int id	- The ID of the element to find.
* @param[out] IO_thing* - A pointer to the element if successful, 0 if not.
*/
IO_thing* IO_registry::get (u32 id) {
	for (IO_thing * p = list; p; p = p->next) {
		if (p->id() == id) return p;
	}
	return 0;
}

/**
* @fn void IO_registry::add (IO_thing * new_io)
* @brief Adds an element to the linked list of all created IO elements.
* @param[in] IO_thing * new_io - A pointer to the element to add.
*/
void IO_registry::add (IO_thing * new_io) {
	if (!list) { list = new_io;	}
	else {
		new_io->next = list;
		list = new_io;
	}
}

/**
* @fn int IO_registry::remove (IO_thing * to_remove)
* @brief Removes an element from the linked list of all created IO elements.
* @param[in]  IO_thing * to_remove - A pointer to the element to remove.
* @param[out] int				   - 1 if successful, 0 if not.
*/
int IO_registry::remove (IO_thing * to_remove) {
	if (!to_remove) 			return 0;
	if (!list) 					return 0;

	IO_thing * p = list;
	if (list->id() == to_remove->id()) {
		list = list->next;
		return 1;
	}
	while (p) {
		if (p->next && p->next->id() == to_remove->id()) {
			p->next=p->next->next;
			return 1;
		}
		p = p->next;
	}
	errors |= (1<<remove_failed);
	return 0;
}

/**
* @fn int IO_registry::set (int id, int value)
* @brief Finds an element and sets its value.
* @param[in]  int id	- The ID of the element to set.
* @param[in]  int value	- The new value.	
* @param[out] int 		- 1 if successful, 0 if not.
* 
* @note
* Called when parsing a serial message from the IrisControls application.
* Raises an errors flag if unsuccessful.
*/
int IO_registry::set (u32 id, int value) {
	if (!list) 			return 0;
	IO_thing* p = list;
	while (p) {
		if (p->id() == id) {
			p->set(value);
			p->raise_value_updated_flag();
			return 1;

		}
		p = p->next;
	}
	errors |= (1 << set_failed);
	return 0;
}

/**
* @fn int IO_registry::set_element_pressed
* @brief Finds an element and raises a flag that indicates the user has clicked it.
* @param[in]  int id - The ID of the element
* @param[out] int	 - 1 if successful, 0 if not.
* 
* @note
* Called when parsing a serial message from the IrisControls application.
* Raises an errors flag if unsuccessful.
*/
int IO_registry::set_element_pressed (u32 id) {

	if (!list) 			return 0;
	IO_thing * p = list;
	while (p) {
		if (p->id() == id ) {			
			p->raise_element_pressed_flag();
			return 1;
		}
		p = p->next;
	}
	errors |= (1<<set_failed);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
