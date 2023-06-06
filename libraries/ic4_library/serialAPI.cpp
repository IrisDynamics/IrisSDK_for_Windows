/**
 * @file serialAPI.cpp
 * @author  Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 * @brief Contains the actual IC4 serial API command, line by line.
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

#include "API_Config.h"
#include "iriscontrols4.h"


/**
 * @brief A mini-handshake. Passes the server name to Iris Controls.
 */
void IrisControls4::enquiryResponse(){
	print_c(char(NAME_ENQUIRY));
	print_s(device_id);
	end_tx_frame();
}

/**
* @brief streams the handshake response
*/
void IrisControls4::handshakeResponse(u64 now, uint8_t crc_result) {
	print_c(char(HANDSHAKE));
#ifdef BYTE_STUFFING_PARSING
	print_i((u16)IC4_virtual->byte_stuffing);
#endif
#ifdef MESSAGE_LENGTH_PARSING
	print_i((u16)IC4_virtual->no_byte_stuffing);
#endif
	print_i((u16)IC4_virtual->communication_protocol);//Half or Full Duplex
	print_s(IC4_virtual->device_id); //device ID
	print_s(IC4_virtual->serial_api_version); //IC4 serial API version
	print_s(IC4_virtual->server_name); //IC4 server name
	print_i(now);
	print_i(IC4_virtual->timeout_timer); //timeout timer
	print_i(crc_result);
	print_c((char)END_OF_TRANSMISSION);
}

/**
 * @brief Initiates a disconnect from Iris Controls
 */
void IrisControls4::disconnect(){
	connection_status = disconnected;
	was_timed_out = 0;

	begin_tx_frame();
	print_c((char)DISCONNECT);
	end_tx_frame();
}

/**
 * @brief Transmits the result of the device CRC to Iris Controls
 */
void IrisControls4::tx_crc_result(int result){
	begin_tx_frame();
	print_c((char)DEVICE_STATE_CHECK);
	print_i(result);
	end_tx_frame();
}

/**
* @brief Sets the number of rows and columns the grid has
*/
void IrisControls4::gui_set_grid(u16 num_rows, u16 num_columns){
	begin_tx_frame(); //STX char
	print_c(char(SET_GUI_ROWS_COLS));
	print_i(num_rows);
	print_i(num_columns);
	end_tx_frame();
}

/**
* @brief Queries IC which returns the maximum number of allowed rows/columns
*/
void IrisControls4::gui_query_max_grid(){
	begin_tx_frame(); //STX char
	print_c(char(QUERY_MAX_ROWS_COLS));
	end_tx_frame();
}

/**
* @brief Sets the GUI to the max size of the user's screen
*/
void IrisControls4::gui_set_max_grid(){
	begin_tx_frame(); //STX char
	print_c(char(SET_MAX_ROWS_COLS));
	end_tx_frame();
}

/**
* @brief Injects a delay of delay_length ms into the IC4 serial thread.
*/
void IrisControls4::inject_delay(u16 delay_length){
	if(delay_length > MAX_DELAY_TIME) delay_length = MAX_DELAY_TIME;

	begin_tx_frame(); //STX char
	print_c(char(INJECT_DELAY));
	print_i(delay_length);
	end_tx_frame();
}

/**
* @brief transmits the end of transmission message which signals IC4 to transmit in half duplex. 
*
* In both duplex configurations, this is also used to track the framerate of the GUI 
*/
void IrisControls4::end_of_frame(){	
	begin_tx_frame(); //STX char
	print_c(char(END_OF_TRANSMISSION));
	end_tx_frame();

	refresh_timeout();
	eot_queued = 1;
}

/**
 * @brief Commands Iris Controls to load a configuration file
 */
void IrisControls4::load_config_file(const char * filename){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(filename);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c((char)LOAD_CONFIG_FILE);
	print_s(filename);
	end_tx_frame();
}

/**
 * @brief Assigns a keyboard shortcut to key 'key' which activates the element with the id 'target'
 */
void IrisControls4::assign_keyboard_shortcut(int key, int target){
	begin_tx_frame();
	print_c((char)ASSIGN_KEYBOARD_SHORTCUT);
	print_i(key);
	print_i(target);
	end_tx_frame();
}

/**
 * @brief Sets the default colour for the specified aspect of the specified element type
 */
void IrisControls4::set_default_colour(u16 element_type, u16 config, u16 r, u16 g, u16 b, u16 a){
	begin_tx_frame(); //STX char
	print_c((char)SET_DEFAULT_COLOUR);
	print_i(element_type);
	print_i(config);
	print_i(r);
	print_i(g);
	print_i(b);
	print_i(a);
	end_tx_frame();
}

/**
 * @brief Resets all element colours to their default values
 */
void IrisControls4::reset_all_default_colours(){
	begin_tx_frame();
	print_c((char)RESET_ALL_DEFAULT_COLOURS);
	end_tx_frame();
}

/**
 * @brief Resets one element type's default colours
 */
void IrisControls4::reset_element_default_colours(u16 element_type){
	begin_tx_frame();
	print_c((char)RESET_COLOURS_OF_ELEMENT_TYPE);
	print_i(element_type);
	end_tx_frame();
}

/**
 * @brief Resets one element instance's default colours
 */
void IrisControls4::reset_specific_element_colours(u32 index){
	begin_tx_frame();
	print_c((char)RESET_SPECIFIC_INSTANCE_COLOURS);
	print_i(index);
	end_tx_frame();
}

/**
 * @brief Sets the title of IrisControls' main window
 */
void IrisControls4::set_main_window_title(const char * new_title){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(new_title);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c((char)SET_WINDOW_TITLE);
	print_s(new_title);
	end_tx_frame();
}


/**
* @brief Sets/alters the config of the GUI Thing
*/
void IrisControls4::gui_thing_config(u32 index, u32 config){
	begin_tx_frame(); //STX char
	print_c(char(CONFIG_ELEMENT));
	print_i(index);
	print_i(config);
	end_tx_frame();
}

///////////////////////////////////////// GUI Page Implementation //////////////////////////////////////
/**
* @brief Adds a new GUI Page.
*/
void IrisControls4::gui_page_add(u32 index, u32 parent_index) {
	begin_tx_frame();	//STX char
	print_c(char(ADD_GUI_PAGE));
	print_i(index);
	print_i(parent_index);
	end_tx_frame();
}

/**
* @brief Removes a GUI Page.
*/
void IrisControls4::gui_page_remove(u32 index) {
	begin_tx_frame();	//STX char
	print_c(char(REMOVE_GUI_PAGE));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Shows all elements contained in the GUI Page.
*/
void IrisControls4::gui_page_show(u32 index) {
	begin_tx_frame();	//STX char
	print_c(char(SHOW_GUI_PAGE));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Hides all elements contained in the GUI Page.
*/
void IrisControls4::gui_page_hide(u32 index) {
	begin_tx_frame();	//STX char
	print_c(char(HIDE_GUI_PAGE));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Adds a FlexElement to the GUI Page.
*/
void IrisControls4::gui_page_add_element(u32 index, u32 element_index) {
	begin_tx_frame();	//STX char
	print_c(char(ADD_ELEMENT_TO_PAGE));
	print_i(index);
	print_i(element_index);
	end_tx_frame();
}

/**
* @brief Removes a FlexElement from the GUI Page.
*/
void IrisControls4::gui_page_remove_element(u32 index, u32 element_index) {
	begin_tx_frame();	//STX char
	print_c(char(REMOVE_ELEMENT_FROM_PAGE));
	print_i(index);
	print_i(element_index);
	end_tx_frame();
}

/**
* @brief Adds a child GUI_Page to the GUI Page.
*/
void IrisControls4::gui_page_add_page(u32 index, u32 page_index) {
	begin_tx_frame();	//STX char
	print_c(char(ADD_CHILD_PAGE_TO_PAGE));
	print_i(index);
	print_i(page_index);
	end_tx_frame();
}

/**
* @brief Removes a child GUI_Page from the GUI Page.
*/
void IrisControls4::gui_page_remove_page(u32 index, u32 page_index) {
	begin_tx_frame();	//STX char
	print_c(char(REMOVE_CHILD_PAGE_FROM_PAGE));
	print_i(index);
	print_i(page_index);
	end_tx_frame();
}

///////////////////////////////////////// flex_element Implementation //////////////////////////////////
/**
* @brief Hides the FlexElement with id index
*/
void IrisControls4::flexElement_hide(u32 index){
	begin_tx_frame();	//STX char
	print_c(char(HIDE_ELEMENT));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Shows the FlexElement with id index
*/
void IrisControls4::flexElement_show(u32 index){
	begin_tx_frame(); //STX char
	print_c(char(SHOW_ELEMENT));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Moves the FlexElement with id index to the new row and column
*/
void IrisControls4::flexElement_move(u32 index, u16 row, u16 column){
	begin_tx_frame(); //STX char
	print_c(char(MOVE_ELEMENT));
	print_i(index);
	print_i(row);
	print_i(column);
	end_tx_frame();
}

/**
* @brief Deletes the FlexElement with id index.
*/
void IrisControls4::flexElement_remove(u32 index){
	begin_tx_frame(); //STX char
	print_c(char(REMOVE_ELEMENT));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Resizes the FlexElement with id index to the new row and column spans.
*/
void IrisControls4::flexElement_resize(u32 index, u16 rowSpan, u16 columnSpan){
	begin_tx_frame(); //STX char
	print_c(char(RESIZE_ELEMENT));
	print_i(index);
	print_i(rowSpan);
	print_i(columnSpan);
	end_tx_frame();
}

/**
* @brief Renames the FlexElement with id index. This will change the visible label for things like
* FlexButtons and FlexSlider.
*/
void IrisControls4::flexElement_rename(u32 index, const char* name){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(RENAME_ELEMENT));
	print_i(index);
	print_s(name);
	end_tx_frame();
}

/**
* @brief Updates the value of the flex element
*/
void IrisControls4::flexElement_update(u32 index, int value){
	begin_tx_frame(); //STX char
	print_c(char(UPDATE_VALUE));
	print_i(index);
	print_i(value);
	end_tx_frame();
}

/**
* @brief Sets the colour property of the Flex Element
*/
void IrisControls4::flexElement_setColour(u32 index, u16 config, u16 r, u16 g, u16 b, u16 a){
	begin_tx_frame();
	print_c(char(SET_COLOUR));
	print_i(index);
	print_i(config);
	print_i(r);
	print_i(g);
	print_i(b);
	print_i(a);
	end_tx_frame();
}

/**
 * @brief Disables/Enables the element
 * While disabled: FlexButtons cannot be clicked or toggled, FlexSliders and FlexData are input only.
 */
void IrisControls4::flexElement_disable(u32 index, bool status){
	begin_tx_frame(); //STX char
	print_c(char(DISABLE_ENABLE));
	print_i(index);
	print_i((int)status);
	end_tx_frame();
}

/**
 * @brief Sets the font size of an element to one of the enumerated font sizes
 */
void IrisControls4::flexElement_set_font_size(u32 index, u16 sub_element, u16 font_size){
	begin_tx_frame(); //STX char
	print_c(char(SET_FONT_SIZE));
	print_i(index);
	print_i(sub_element);
	print_i(font_size);
	end_tx_frame();
}

///////////////////////////////////////// flex_button Implementation //////////////////////////////////

/**
* @brief Adds a new FlexButton to the GUI. If the FlexButton had already been added (ie. the index is already
* assigned) this function will instead make any changes to that FlexButton and then make it visible if hidden.
*/
void IrisControls4::flexButton_add(u32 parent_id, u32 index, const char * name, int value, u16 row, u16 column, u16 rowSpan, u16 columnSpan){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(FLEXBUTTON_ADD)); //20 is add flexbutton
	print_s(name);
	print_i(parent_id);
	print_i(index);
	print_i(value);
	print_i(row);
	print_i(column);
	print_i(rowSpan);
	print_i(columnSpan);
	end_tx_frame();
}

/**
* @brief Sets the toggle/checked state of the button.
*/
void IrisControls4::flexButton_set_checked(u32 index, int t){
	begin_tx_frame();	//STX char
	print_c(char(FLEXBUTTON_CHECK));
	print_i(index);
	print_i(t);
	end_tx_frame();
}

////////////////////////////////////////////flexSlider Implementation///////////////////////

/**
* @brief Adds a new Flex Slider (or alters and then shows an existing one)
*/
void IrisControls4::flexSlider_add(u32 parent_id, u32 index, int total_factor, const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int min, int max, int initValue, const char* units, u32 config){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	message_size 	+= size_of_string(units);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(FLEXSLIDER_ADD));
	print_i(parent_id);
	print_i(index);
	print_s(name);
	print_i(min);
	print_i(max);
	print_i(total_factor);
	print_i(initValue);
	print_s(units);
	print_i(row);
	print_i(column);
	print_i(rowSpan);
	print_i(columnSpan);
	print_i(config);
	end_tx_frame();
}

/**
 * @brief Sets the specified slider's minimum and maximum values
 */
void IrisControls4::flexSlider_set_range(u32 index, int min, int max) {
	begin_tx_frame(); //STX char
	print_c(char(SET_FLEXSLIDER_RANGE));
	print_i(index);
	print_i(min);
	print_i(max);
	end_tx_frame();
}


////////////////////////////////////////////flexLabel Implementation///////////////////////

/**
* @brief Adds a new Flex Label (or alters and then shows an existing one)
*/
void IrisControls4::flexLabel_add(u32 parent_id, u32 index, const char * name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(FLEXLABEL_ADD));
	print_i(parent_id);
	print_i(index);
	print_s(name);
	print_i(row);
	print_i(column);
	print_i(rowSpan);
	print_i(columnSpan);
	print_i(config);
	end_tx_frame();
}

////////////////////////////////////////////flex Data Implementation///////////////////////

/**
* @brief Adds a new Flex Data (or alters and then shows an existing one)
*/
void IrisControls4::flexData_add(u32 parent_id, u32 index, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, int initValue, const char * units, int total_factor, u32 config){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = 	size_of_string(name);
	message_size 	+= 	size_of_string(units);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(FLEXDATA_ADD));
	print_i(parent_id);
	print_i(index);
	print_s(name);
	print_i(row);
	print_i(column);
	print_i(rowSpan);
	print_i(columnSpan);
	print_i(initValue);
	print_s(units);
	print_i(total_factor);
	print_i(config);
	end_tx_frame();
}

////////////////////////////////////////////FlexDropdown Implementation///////////////////////
/**
* @brief Adds a new Flex Dropdown (or alters and then shows an existing one)
*/
void IrisControls4::flexDropdown_add(u32 parent_id, u32 index, u16 row, u16 column, u16 rowSpan, u16 columnSpan, u32 config) {
	begin_tx_frame();
	print_c(char(FLEXDROPDOWN_ADD));
	print_i(parent_id);
	print_i(index);
	print_i(row);
	print_i(column);
	print_i(rowSpan);
	print_i(columnSpan);
	print_i(config);
	end_tx_frame();
}

/**
* @brief Adds an option to a Flex Dropdown
*/
void IrisControls4::flexDropdown_add_option(u32 index, u32 option_id, const char* label) {
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(label);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(FLEXDROPDOWN_ADD_OPTION));
	print_i(index);
	print_i(option_id);
	print_s(label);
	end_tx_frame();
}

/**
* @brief Removes an option from a Flex Dropdown
*/
void IrisControls4::flexDropdown_remove_option(u32 index, u32 option_id) {
	begin_tx_frame();
	print_c(char(FLEXDROPDOWN_REMOVE_OPTION));
	print_i(index);
	print_i(option_id);
	end_tx_frame();
}


////////////////////////////////////////////FlexPlot Implementation///////////////////////

/**
* @brief Adds a new Flex Plot (or alters and then shows an existing one)
*/
void IrisControls4::flexPlot_add(u32 parent_id, u32 index, const char* name, u16 row, u16 column, u16 rowSpan, u16 columnSpan, float min, float max, u32 config){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(FLEXPLOT_ADD));
	print_i(parent_id);
	print_i(index);
	print_s(name);
	print_i(row);
	print_i(column);
	print_i(rowSpan);
	print_i(columnSpan);
	print_i(min);
	print_i(max);
	print_i(config);
	end_tx_frame();
}

/**
* @brief Sets/alters the range of the Flex Plot (min and max y-value shown)
*/
void IrisControls4::flexPlot_set_range(u32 index, int config, float min, float max){
	begin_tx_frame(); //STX char
	print_c(char(SET_RANGE));
	print_i(index);
	print_i(config);
	print_i(min);
	print_i(max);
	end_tx_frame();
}

/**
* @brief Sets/alters the domain of the Flex Plot (min and max x-value shown)
*/
void IrisControls4::flexPlot_set_domain	(u32 index, float min, float max){
	begin_tx_frame(); //STX char
	print_c(char(SET_DOMAIN));
	print_i(index);
	print_i(min);
	print_i(max);
	end_tx_frame();
}

/**
* @brief Sets/alters the domain of the Flex Plot (range of x-values shown)
*/
void IrisControls4::flexPlot_set_domain	(u32 index, int domain){
	begin_tx_frame(); //STX char
	print_c(char(SET_WALKING_DOMAIN));
	print_i(index);
	print_i(domain);
	end_tx_frame();
}

/**
* @brief Sets/alters the number of visible data points of the Flex Plot
*/
void IrisControls4::flexPlot_set_visible_datapoints(u32 index, u16 datapoints){
	begin_tx_frame();
	print_c(char(SET_MAX_PLOTTED_POINTS));
	print_i(index);
	print_i(datapoints);
	end_tx_frame();
}

/**
* @brief Sets/alters from which Dataset the axes labels of the Flex Plot are taken
*/
void IrisControls4::flexPlot_set_axes_labels(u32 flexplot_index, u32 dataset_index){
	begin_tx_frame();
	print_c(char(SET_AXES_LABELS));
	print_i(flexplot_index);
	print_i(dataset_index);
	end_tx_frame();
}

////////////////////////////////////////////Dataset Implementation///////////////////////

/**
* @brief Adds a new Dataset (or alters and then shows an existing one)
*/
void IrisControls4::dataset_add(u32 dataset_id, u32 plot_id, const char * name, const char * x_label, const char * y_label, u32 config){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	message_size 	+= size_of_string(x_label);
	message_size 	+= size_of_string(y_label);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(DATASET_ADD));
	print_i(dataset_id);
	print_i(plot_id);
	print_s(name);
	print_s(x_label);
	print_s(y_label);
	print_i(config);
	end_tx_frame();
}

/**
 * @brief Sets the maximum number of datapoints, for the dataset with id index, retained by Iris Controls before doing FIFO deletion
 */
void IrisControls4::dataset_set_max_data_points	(u32 index, u32 number_of_data_points){
	begin_tx_frame();
	print_c(char(DATASET_SET_MAX_SIZE));
	print_i(index);
	print_i(int(number_of_data_points));
	end_tx_frame();
}

/**
* @brief Removes/deletes a Dataset (irreversible)
*/
void IrisControls4::dataset_remove(u32 index){
	begin_tx_frame();
	print_c(char(DATASET_REMOVE));
	print_i(index);
	end_tx_frame();
 }

/**
* @brief Adds float data points to a Dataset
*/
void IrisControls4::dataset_add_float_data(u32 index, u16 dataPairs, float xData[], float yData[]){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = 8 * dataPairs;
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(DATASET_ADD_FLOAT_DATA));
	print_i(index);
	print_i(int(dataPairs));
	for ( u16 i=0 ; i<dataPairs ; i++ ){
		print_i(xData[i]);
		print_i(yData[i]);
	}
	end_tx_frame();
}
/**
* @brief Adds an  integer data point to a Dataset
*/
void IrisControls4::dataset_add_int_data(u32 index, int xData, int yData){
	begin_tx_frame();
	print_c(char(DATASET_ADD_INT_DATA));
	print_i(index);
	print_i(xData);
	print_i(yData);
	end_tx_frame();
}

#ifndef WINDOWS
void IrisControls4::dataset_add_int_data(u32 index, s32 xData, s32 yData){
	begin_tx_frame();
	print_c(char(DATASET_ADD_INT_DATA));
	print_i(index);
	print_i(xData);
	print_i(yData);
	end_tx_frame();
}
#endif

/**
* @brief Adds an  integer data point to a Dataset with a u64 xData input
*/
void IrisControls4::dataset_add_int_data(u32 index, u64 xData, int yData){
	begin_tx_frame();
	print_c(char(DATASET_ADD_TIME_DATA));
	print_i(index);
	print_i(xData);
	print_i(yData);
	end_tx_frame();
}

/**
* @brief Hides the Dataset ie makes it invisible on its Flex Plot
*/
void IrisControls4::dataset_hide(u32 index){
	begin_tx_frame();
	print_c(char(HIDE_DATASET));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Shows the Dataset ie makes it visible on its Flex Plot
*/
void IrisControls4::dataset_show(u32 index){
	begin_tx_frame();
	print_c(char(SHOW_DATASET));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Purges the Dataset of all data
*/
void IrisControls4::dataset_purge(u32 index){
	begin_tx_frame();
	print_c(char(DATASET_PURGE_DATA));
	print_i(index);
	end_tx_frame();
}

/**
* @brief Assigns/re-assigns a dataset to a FlexPlot
*/
void IrisControls4::dataset_assign(u32 dataset_index, u32 flexplot_index){
	begin_tx_frame();
	print_c(char(DATASET_ASSIGN));
	print_i(dataset_index);
	print_i(flexplot_index);
	end_tx_frame();
}

/**
* @brief Un-assigns the dataset from its FlexPlot
*/
void IrisControls4::dataset_unassign(u32 index){
	begin_tx_frame();
	print_c(char(DATASET_UNASSIGN));
	print_i(index);
	end_tx_frame();
 }

/**
* @brief Sets the colour of the Dataset
*/
void IrisControls4::dataset_set_colour(u32 index, u16 r, u16 g, u16 b, u16 a){
	begin_tx_frame();
	print_c(char(DATASET_SET_COLOUR));
	print_i(index);
	print_i(r);
	print_i(g);
	print_i(b);
	print_i(a);
	end_tx_frame();
}


////////////////////////////////////////////Dataset Implementation///////////////////////

/**
 * @brief Add a new DataLog.
 */
void IrisControls4::datalog_add(u32 index, const char * name){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(name);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(LOG_ADD));
	print_i(index);
	print_s(name);
	end_tx_frame();
}

/**
 * @brief Writes to an existing DataLog.
 */
void IrisControls4::datalog_write(u32 index, const char * string){
	#ifdef MESSAGE_LENGTH_PARSING
	u16 message_size = size_of_string(string);
	begin_tx_frame(message_size);
	#endif
	#ifdef BYTE_STUFFING_PARSING
	begin_tx_frame();
	#endif
	print_c(char(LOG_WRITE));
	print_i(index);
	print_s(string);
	end_tx_frame();
}

/**
* @brief Closes an existing DataLog.
*/
void IrisControls4::datalog_close(u32 index) {
	begin_tx_frame();
	print_c(char(LOG_CLOSE));
	print_i(index);
	end_tx_frame();
}



