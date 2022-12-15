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

u32 GUI_thing::index_assigner = 0;

IO_thing * 	IO_registry::list 					= 0;
IO_thing *	IO_registry::active_element_list 	= 0;
u32 		IO_registry::errors					= 0;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// GUI Thing Implementation/////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief Virtual method. Sets/alters the config of the GUI thing
*/
void GUI_thing::config(u16 config) {
	IC4_virtual->gui_thing_config(index, config);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// IO Thing Implementation//////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
IO_thing::IO_thing() : FlexElement() {}
IO_thing::~IO_thing () {}

void IO_thing::update_received(){
	feedback_register &= ~ELEMENT_VALUE_MASK;
}

/**
* @brief Returns true if the element was pressed since the last time this function was called
* @param[out] int ret - 1 if the element was pressed since last check, 0 otherwise
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
* @brief Sets/alters the config of the Complex IO thing
*/
void Complex_IO_thing::config(u16 config) {
	set_factor(config);
	IC4_virtual->gui_thing_config(index, config);
}

/**
 * @brief Returns the value of the element correcting for the factor
 */
int 	Complex_IO_thing::get 		() { update_received(); 	if(my_factor == 1) return value; return value / my_factor;	}
float 	Complex_IO_thing::get_f 	() { update_received(); 	return (float)value / (float)my_factor;						}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Console Implementation///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Console::Console(){};
Console::~Console(){};

/**
* @brief Hides the console
*/
void Console::hide(){ IC4_virtual->flexElement_hide(CONSOLE_ELEMENT_ID); }

/**
* @brief Shows the console
*/
void Console::show(){ IC4_virtual->flexElement_show(CONSOLE_ELEMENT_ID); }

/**
* @brief Moves the console to a new row/column
*/
void Console::move(u16 row, u16 column){ IC4_virtual->flexElement_move(CONSOLE_ELEMENT_ID, row, column); }

/**
* @brief Resizes the console
*/
void Console::resize(u16 rowSpan, u16 columnSpan){ IC4_virtual->flexElement_resize(CONSOLE_ELEMENT_ID, rowSpan, columnSpan); }

/**
 * @brief Sets the colour of the selected aspect of the console
 */
void Console::set_colour(colour_set_flag flag,u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(CONSOLE_ELEMENT_ID, (u16)flag, r, g, b, a); }

/**
 * @brief Resets the console's colours to their defaults
 */
void Console::reset_default_colours() { IC4_virtual->reset_element_default_colours(CONSOLE); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// GUI Page Implementation///////////////////
//////////////////////////////////////////////////////////////////////////////////////////
GUI_Page::GUI_Page() : GUI_thing() {}
GUI_Page::~GUI_Page() {}

void GUI_Page::add		() { IC4_virtual->gui_page_add(index); }
void GUI_Page::remove	() { IC4_virtual->gui_page_remove(index); }
		
void GUI_Page::add_element		(FlexElement* flex_element) { IC4_virtual->gui_page_add_element		(index, flex_element->id()); }
void GUI_Page::remove_element	(FlexElement* flex_element) { IC4_virtual->gui_page_remove_element	(index, flex_element->id()); }
		 
void GUI_Page::show() { IC4_virtual->gui_page_show(index); }
void GUI_Page::hide() { IC4_virtual->gui_page_hide(index); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexElement Implementation///////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexElement::FlexElement() : GUI_thing(){}
FlexElement::~FlexElement(){}

/**
* @brief Hides the FlexElement with id index
*/
void FlexElement::hide() { IC4_virtual->flexElement_hide(index); }

/**
* @brief Shows the FlexElement with id index
*/
void FlexElement::show() { IC4_virtual->flexElement_show(index); }

/**
* @brief Moves the FlexElement with id index to the new row and column
*/
void FlexElement::move(u16 row, u16 column) { IC4_virtual->flexElement_move(index, row, column); }

/**
* @brief Deletes the FlexElement with id index.
*/
void FlexElement::remove(){ IC4_virtual->flexElement_remove(index); }

/**
* @brief Resizes the FlexElement with id index to the new row and column spans.
*/
void FlexElement::resize(u16 rowSpan, u16 columnSpan){ IC4_virtual->flexElement_resize(index, rowSpan, columnSpan);}

/**
* @brief Renames the FlexElement with id index. This will change the visible label for things like
* FlexButtons and FlexSlider.
*/
void FlexElement::rename(const char * name){
	IC4_virtual->flexElement_rename(index, name);
}

/**
 * @brief Sets the font size of the element
 */
void FlexElement::set_font_size(FlexElement::subelement_type sub_element, u16 font_size){
	IC4_virtual->flexElement_set_font_size(index, (u16)sub_element, font_size);
}

/**
 * @brief Disables/Enables the FlexElement
 * While disabled: FlexButtons cannot be clicked or toggled, FlexSliders and FlexData are output only.
 */
void FlexElement::disable(bool status) { IC4_virtual->flexElement_disable(index, status); }

/**
 * @brief Resets all elements' default colours
 */
void FlexElement::reset_all_default_colours() { IC4_virtual->reset_all_default_colours(); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexButton Implementation////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexButton::FlexButton() : IO_thing(){ IO_registry::add(this); }
FlexButton::~FlexButton()
{
	IO_registry::remove(this);
}

/**
* @brief Adds a new FlexButton to the GUI. If the FlexButton had already been added (ie. the index is already
* assigned) this function will instead make any changes to that FlexButton and then make it visible if hidden.
*/
void FlexButton::add(const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan) { add(NULL, name, initValue, row, column, rowSpan, columnSpan); }
void FlexButton::add(GUI_Page* parent, const char* name, int initValue, u16 row, u16 column, u16 rowSpan, u16 columnSpan){
	int parent_id = -1; if (parent) parent_id = parent->id();
	
	IO_registry::activate(this);
	value = initValue;
	IC4_virtual->flexButton_add(parent_id, index, name, value, row, column, rowSpan, columnSpan);
}

/**
* @brief Queries the button to determine if it was toggled since the last time it was queried.
* @param[out] int ret - True if the button as toggled since last time this function was called.
* 
* Note: Returned value does not reflect the checked status of the button.
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
* @brief Sets the toggle/checked state of the button.
*/
void FlexButton::set_toggle (int t) { if (set(t)) IC4_virtual->flexButton_set_checked(index, t); }

/**
* @brief Sets the colour property of the Flex Element
*/
void FlexButton::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @brief Sets the default colour property of the element
*/
void FlexButton::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXBUTTON, (u16)flag, r, g, b, a);
}

/**
 * @brief Resets the specific instance of the FlexButton to default colours
 */
void FlexButton::reset_this_flexbutton_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
 * @brief Resets all FlexButton colours to their defaults
 */
void FlexButton::reset_all_flexbutton_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXBUTTON); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexSlider Implementation////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexSlider::FlexSlider() : Complex_IO_thing(){ IO_registry::add(this); }
FlexSlider::~FlexSlider()
{
	IO_registry::remove(this);
}

/**
* @brief Adds a new Flex Slider (or alters and then shows an existing one)
*/
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u16 config)				{ add(parent, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u16 config) 								{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u16 config) 				{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator,  units, config);	}
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u16 config)
{	
	int parent_id = -1; if (parent) parent_id = parent->id();

	IO_registry::activate(this);
	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexSlider_add(parent_id, index, (int)my_denominator, name, row, column,rowSpan, columnSpan, min * my_factor, max * my_factor, value, units, config);
}

void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, u16 config)		{ add(parent, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, u16 config)						{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, const char * units, u16 config)	{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, units, config);	}
void FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double min, double max, double initValue, u16 denominator, const char* units, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	IO_registry::activate(this);
	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexSlider_add(parent_id, index, (int)my_denominator, name, row, column,rowSpan, columnSpan, (int)(min * my_factor), (int)(max * my_factor), value, units, config);
}



/**
* @brief Updates the value of the Flex Slider
*/
int   FlexSlider::update(int new_value)		{ if (set(new_value * my_factor)) 		 IC4_virtual->flexElement_update(index, value); return value;																		}
float FlexSlider::update(float new_value)	{ if (set((int)(new_value * my_factor))) IC4_virtual->flexElement_update(index, value); if (my_factor ==1 ) return (float)value; return (float)value / (float)my_factor;	}

/**
* @brief Sets the colour property of the Flex Element
*/
void FlexSlider::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
 * @brief Resets the specific instance of the FlexSlider to default colours
 */
void FlexSlider::reset_this_flexslider_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @brief Sets the default colour property of the element
*/
void FlexSlider::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXSLIDER, (u16)flag, r, g, b, a);
}

/**
 * @brief Resets all FlexSlider colours to their defaults
 */
void FlexSlider::reset_all_flexslider_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXSLIDER); }

/**
 * @brief Sets the slider's minimum and maximum values
 */
void FlexSlider::set_range(int min, int max) { IC4_virtual->flexSlider_set_range(index, min, max); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Basic FlexSlider Implementation//////////////
//////////////////////////////////////////////////////////////////////////////////////////
Basic_FlexSlider::Basic_FlexSlider() : IO_thing(){ IO_registry::add(this); }
Basic_FlexSlider::~Basic_FlexSlider()
{
	IO_registry::remove(this);
}

/**
* @brief Adds a new Flex Slider (or alters and then shows an existing one)
*/
void Basic_FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u16 config)			{ add(parent, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void Basic_FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, u16 config) 							{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator, ""	  , config);	}
void Basic_FlexSlider::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char * units, u16 config)		{ add(NULL	, name, row, column, rowSpan, columnSpan, min, max, initValue, denominator,  units, config);	}
void Basic_FlexSlider::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, u16 denominator, const char* units, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();
	IO_registry::activate(this);
	set(initValue);
	IC4_virtual->flexSlider_add(parent_id, index, 1, name, row, column,rowSpan, columnSpan, min, max, value, units, config);
}

/**
* @brief Updates the value of the Flex Slider
*/
int Basic_FlexSlider::update(int new_value)	{ if (set(new_value)) 		 IC4_virtual->flexElement_update(index, value); return value; }

/**
* @brief Sets the colour property of the Flex Element
*/
void Basic_FlexSlider::set_colour(FlexSlider::colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
 * @brief Resets the specific instance of the Basic_FlexSlider to default colours
 */
void Basic_FlexSlider::reset_this_flexslider_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @brief Sets the default colour property of the element
*/
void Basic_FlexSlider::set_default_colour(FlexSlider::colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXSLIDER, (u16)flag, r, g, b, a);
}

/**
 * @brief Sets the slider's minimum and maximum values
 */
void Basic_FlexSlider::set_range(int min, int max) { IC4_virtual->flexSlider_set_range(index, min, max); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexLabel Implementation/////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexLabel::FlexLabel() : FlexElement(){}
FlexLabel::~FlexLabel() {}

/**
* @brief Adds a new Flex Label (or alters and then shows an existing one)
*/
void FlexLabel::add(const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u16 config) { add(NULL, name, row, column, rowSpan, columnSpan, config); }
void FlexLabel::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();
	value = 0;
	IC4_virtual->flexLabel_add(parent_id, index, name, row, column, rowSpan, columnSpan, config);
}

/**
 * @brief Sets the font size of the FlexLabel's text
 */
void FlexLabel::font_size(u16 font_size){
	IC4_virtual->flexElement_set_font_size(index, (u16)FlexElement::subelement_type::LABEL, font_size);
}

/**
* @brief Sets the colour property of the Flex Element
*/
void FlexLabel::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
 * @brief Resets the specific instance of the FlexLabel to default colours
 */
void FlexLabel::reset_this_flexlabel_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
* @brief Sets the default colour property of the element
*/
void FlexLabel::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXLABEL, (u16)flag, r, g, b, a);
}

/**
 * @brief Resets all FlexLabel colours to their defaults
 */
void FlexLabel::reset_all_flexlabel_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXLABEL); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Clickable FlexLabel Implementation///////////
//////////////////////////////////////////////////////////////////////////////////////////
C_FlexLabel::C_FlexLabel() : IO_thing(){ IO_registry::add(this); }
C_FlexLabel::~C_FlexLabel() { IO_registry::remove(this); }

/**
* @brief Adds a new Clickable Flex Label (or alters and then shows an existing one)
*/
void C_FlexLabel::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u16 config) { add(NULL, name, row, column, rowSpan, columnSpan, config); }
void C_FlexLabel::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u16 config) 
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	IO_registry::activate(this);
	value = 0;
	IC4_virtual->flexLabel_add(parent_id, index, name, row, column, rowSpan, columnSpan, config);
}

/**
 * @brief Sets the font size of the FlexLabel's text
 */
void C_FlexLabel::font_size(u16 font_size){
	IC4_virtual->flexElement_set_font_size(index, (u16)FlexElement::subelement_type::LABEL, font_size);
}

/**
* @brief Sets the colour property of the Flex Element
*/
void C_FlexLabel::set_colour(FlexLabel::colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
 * @brief Resets the specific instance of the FlexLabel to default colours
 */
void C_FlexLabel::reset_this_flexlabel_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexData Implementation//////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexData::FlexData() : Complex_IO_thing(){ IO_registry::add(this); }
FlexData::~FlexData() {	IO_registry::remove(this); }

/**
* @brief Adds a new Flex Data (or alters and then shows an existing one)
*/
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u16 config)		{ add(parent, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, u16 config) 							{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char * units, u16 config) 		{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator,  units, config);	}
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 denominator, const char* units, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	IO_registry::activate(this);
	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexData_add(parent_id, index, name, row, column, rowSpan, columnSpan, value, units, (int)(my_denominator), config);
}

void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u16 config)		{ add(parent, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, u16 config)						{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator, ""	, config);	}	
void FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char * units, u16 config) 	{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, denominator,  units, config);	}
void FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, double initValue, u16 denominator, const char* units, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	IO_registry::activate(this);
	set_denominator(denominator);
	set_factor(config);
	set(initValue * my_factor);
	IC4_virtual->flexData_add(parent_id, index, name, row, column, rowSpan, columnSpan, value, units, (int)(my_denominator), config);
}

/**
* @brief Updates the value of the Flex Data
*/
int   FlexData::update(int new_value) 	{ if (set(new_value * my_factor)) 		 IC4_virtual->flexElement_update(index, value); return value;																	}
float FlexData::update(float new_value)	{ if (set((int)(new_value * my_factor))) IC4_virtual->flexElement_update(index, value);	if(my_factor == 1) return (float)value; return (float)value / (float)my_factor;	}


/**
* @brief Sets the colour property of the Flex Element
*/
void FlexData::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @brief Sets the default colour property of the element
*/
void FlexData::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXDATA, (u16)flag, r, g, b, a);
}

/**
 * @brief Resets the specific instance of the FlexLabel to default colours
 */
void FlexData::reset_this_flexdata_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
 * @brief Resets all FlexData colours to their defaults
 */
void FlexData::reset_all_flexdata_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXDATA); }

////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Basic FlexData Implementation//////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
Basic_FlexData::Basic_FlexData() : IO_thing(){ IO_registry::add(this); }
Basic_FlexData::~Basic_FlexData() {	IO_registry::remove(this); }

/**
* @brief Adds a new Flex Data (or alters and then shows an existing one)
*/
void Basic_FlexData::add(GUI_Page *parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 config)		{ add(parent, name, row, column, rowSpan, columnSpan, initValue, ""		, config);	}
void Basic_FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, u16 config) 						{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, ""		, config);	}
void Basic_FlexData::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char * units, u16 config) 	{ add(NULL	, name, row, column, rowSpan, columnSpan, initValue, units	, config);	}
void Basic_FlexData::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char* units, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	IO_registry::activate(this);
	set(initValue);
	IC4_virtual->flexData_add(parent_id, index, name, row, column, rowSpan, columnSpan, value, units, 1, config);
}

/**
* @brief Updates the value of the Flex Data
*/
int Basic_FlexData::update(int new_value) 		{ if (set(new_value)) 		 IC4_virtual->flexElement_update(index, value);	return value;	}

/**
* @brief Sets the colour property of the Flex Element
*/
void Basic_FlexData::set_colour(FlexData::colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
 * @brief Resets the specific instance of the Basic_FlexData to default colours
 */
void Basic_FlexData::reset_this_flexdata_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// FlexPlot Implementation//////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
FlexPlot::FlexPlot() : FlexElement(){}
FlexPlot::~FlexPlot(){}

/**
* @brief Adds a new Flex Plot (or alters and then shows an existing one)
*/
void FlexPlot::add(const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u16 config) { add(NULL, name, row, column, rowSpan, columnSpan, min, max, config); }
void FlexPlot::add(GUI_Page* parent, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u16 config)
{
	int parent_id = -1; if (parent) parent_id = parent->id();

	value = 0;
	IC4_virtual->flexPlot_add(parent_id, index, name, row, column, rowSpan, columnSpan, min, max, config);
}

/**
* @brief Sets/alters the range of the Flex Plot (min and max y-value shown)
*/
void FlexPlot::set_range			(float min, float max){ IC4_virtual->flexPlot_set_range(index, 0, min, max); }
void FlexPlot::set_secondary_range	(float min, float max){ IC4_virtual->flexPlot_set_range(index, 1, min, max); }

/**
* @brief Sets/alters the domain of the Flex Plot (min and max x-value shown)
*/
void FlexPlot::set_domain(float min, float max) { IC4_virtual->flexPlot_set_domain(index, min, max	); }
void FlexPlot::set_domain(int domain) 			{ IC4_virtual->flexPlot_set_domain(index, domain	); }
/**
* @brief Sets/alters the number of visible data points of the Flex Plot
*/
void FlexPlot::set_visible_datapoints(u16 datapoints){ IC4_virtual->flexPlot_set_visible_datapoints(index, datapoints); }

/**
* @brief Sets/alters from which Dataset the axes labels of the Flex Plot are taken
*/
void FlexPlot::set_axes_labels(Dataset* dataset){ IC4_virtual->flexPlot_set_axes_labels(index, dataset->id()); }

/**
* @brief Sets the colour property of the Flex Element
*/
void FlexPlot::set_colour(colour_set_flag config, u16 r, u16 g, u16 b, u16 a){ IC4_virtual->flexElement_setColour(index, (u16)config, r, g, b, a); }

/**
* @brief Sets the default colour property of the element
*/
void FlexPlot::set_default_colour(colour_set_flag flag, u16 r, u16 g, u16 b, u16 a){
	IC4_virtual->set_default_colour((u16)FlexElement::FLEXPLOT, (u16)flag, r, g, b, a);
}

/**
 * @brief Resets the specific instance of the FlexPlot to default colours
 */
void FlexPlot::reset_this_flexplot_default_colours() { IC4_virtual->reset_specific_element_colours(index); }

/**
 * @brief Resets all FlexPlot colours to their defaults
 */
void FlexPlot::reset_all_flexplot_default_colours() { IC4_virtual->reset_element_default_colours(FlexElement::FLEXPLOT); }
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Dataset Implementation///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Dataset::Dataset() : GUI_thing() {}
Dataset::~Dataset() {}

/**
* @brief Adds a new Dataset (or alters and then shows an existing one)
*/
void Dataset::add(FlexPlot * plot, const char * name, const char * x_label, const char * y_label, u16 config){
	IC4_virtual->dataset_add(index, plot->id(), name, x_label, y_label, config);
}

/**
* @brief Removes/deletes a Dataset (irreversible)
*/
void Dataset::remove(){  IC4_virtual->dataset_remove(index); }

/**
* @brief Adds data points to a Dataset
*/
void Dataset::add_data(int xData, int yData) {	IC4_virtual->dataset_add_int_data(index, xData, yData); }
void Dataset::add_data(int dataPairs, int xData[], int yData[]) {
	for(int i=0; i< dataPairs; i++){
		IC4_virtual->dataset_add_int_data(index, xData[i], yData[i]);
	}
}
void Dataset::add_data(u64 xData, int yData) { IC4_virtual->dataset_add_int_data(index, xData, yData); }

void Dataset::add_data(int dataPairs, float xData[], float yData[]) { IC4_virtual->dataset_add_float_data(index, dataPairs, xData, yData); }
void Dataset::add_data(float xData, float yData) {
	float x[1] = {xData};
	float y[1] = {yData};
	IC4_virtual->dataset_add_float_data(index, 1, x, y);
}

/**
 * @brief Sets the maximum number of datapoints retained by Iris Controls before doing FIFO deletion
 */
void Dataset::set_max_data_points(u32 number_of_data_points){ IC4_virtual->dataset_set_max_data_points(index, number_of_data_points); }

/**
* @brief Hides the Dataset - i.e. makes it invisible on its Flex Plot
*/
void Dataset::hide(){ IC4_virtual->dataset_hide(index); }

/**
* @brief Shows the Dataset - i.e. makes it visible on its Flex Plot
*/
void Dataset::show(){ IC4_virtual->dataset_show(index); }

/**
* @brief Purges the Dataset of all data
*/
void Dataset::purge(){ IC4_virtual->dataset_purge(index); }

/**
* @brief Assigns/re-assigns a dataset to a FlexPlot
*/
void Dataset::assign(FlexPlot* plot){ IC4_virtual->dataset_assign(index, plot->id()); }

/**
* @brief Un-assigns the dataset from its FlexPlot
*/
void Dataset::unassign(){  IC4_virtual->dataset_unassign(index); }

/**
* @brief Sets the colour of the Dataset
*/
void Dataset::set_colour(u16 r, u16 g, u16 b, u16 a){ IC4_virtual->dataset_set_colour(index,r, g, b, a); }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// IO Registry Implementation///////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/**
* @brief Finds an IO_thing from the linked list of all elements
* @param[in]  int id	- The ID of the element to find.
* @param[out] IO_thing* - A pointer to the element if successful, 0 if not.
*/
IO_thing * IO_registry::get (int id){
	for (IO_thing * p = list; p; p = p->next) {
		if (p->id() == id) return p;
	}
	return 0;
}

/**
* @brief Adds an element from the linked list of all created IO elements.
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
 * @brief This function is called the first time the element is added via the serial connection
 * It allows the device to keep track of which elements exist in the application and which only
 * exist on the device and is used for doing CRCs
 * @param[in]  IO_thing * activate_io - A pointer to the element to activate.
 */
void IO_registry::activate (IO_thing * active_io) {
	//check to make sure it isn't already activated
	for (IO_thing * p = active_element_list; p; p = p->next_active) {
		if (p == active_io) return;
	}

	if (!active_element_list) { active_element_list = active_io; }
	else {
		active_io->next_active = active_element_list;
		active_element_list = active_io;
	}
}

/**
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
		deactivate(to_remove);
		return 1;
	}
	while (p) {
		if (p->next && p->next->id() == to_remove->id()) {
			p->next=p->next->next;
			deactivate(to_remove);
			return 1;
		}
		p = p->next;
	}
	errors |= (1<<remove_failed);
	return 0;
}

/**
* @brief Removes an element from the active element linked list.
* @param[in]  IO_thing * to_deactivate - A pointer to the element to deactivate.
* @param[out] int					   - 1 if successful, 0 if not.
*/
int IO_registry::deactivate (IO_thing * to_deactivate) {
	if (!to_deactivate) 		return 0;
	if (!active_element_list)	return 0;

	IO_thing * p = active_element_list;
	if (active_element_list->id() == to_deactivate->id()) {
		active_element_list = active_element_list->next_active;
		return 1;
	}
	while (p) {
		if (p->next_active && p->next_active->id() == to_deactivate->id()) {
			p->next_active=p->next_active->next_active;
			return 1;
		}
		p = p->next_active;
	}
	return 0;
}


/**
* @brief Resets the linked list of active element.
* Active elements are those that have had their "add" calls transmitted to the Iris Controls
* application since the last new connection.
*/
void IO_registry::reset_active_list(){
	while(active_element_list){
		deactivate(active_element_list);
	}
}

/**
* @brief Finds an element and sets its value.
* @param[in]  int id	- The ID of the element to set.
* @param[in]  int value	- The new value.	
* @param[out] int 		- 1 if successful, 0 if not.
* 
* Called when parsing a serial message from the IrisControls application.
* Raises an errors flag if unsuccessful.
*/
int IO_registry::set (int id, int value) {
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
* @brief Finds an element and raises a flag that indicates the user has clicked it.
* @param[in]  int id - The ID of the element
* @param[out] int	 - 1 if successful, 0 if not.
* 
* Called when parsing a serial message from the IrisControls application.
* Raises an errors flag if unsuccessful.
*/
int IO_registry::set_element_pressed (int id) {

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
