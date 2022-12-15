#pragma once
/**
* @file dataandsliders.h
* @author Kali Erickson <kerickson@irisdynamics.com>
* @version 1.1
* @brief GUI page that demonstrates various slider use and configuration options

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

#ifndef DATAANDSLIDERS_H_
#define DATAANDSLIDERS_H_

/**
* @class DataAndSliders
* @brief GUI page with examples of different FlexData and FlexSlider configurations
*/
class DataAndSliders {

    FlexLabel tracking_slider_label, nontracking_slider_label,
        tracking_data_label, nontracking_data_label,
        io_slider_label, fb_slider_label, special_colour_label,
        io_data_label, fb_data_label;

    FlexSlider tracking_slider, nontracking_slider,
        io_slider, fb_slider, special_fb_slider;

    FlexData tracking_data, nontracking_data, io_data, fb_data;

    //FlexButton default_colours;

public:

    int isrunning;

    /**
    * @brief adds the FlexElements for the 'Data and Sliders' GUI page.
    */
    void initiate() {

        isrunning = 1;

        int x_anchor = 20;
        int y_anchor = 1;

        //Labels - see labelsandbuttons class for label initialization syntax
        tracking_slider_label.add("<p_style=\"font-size:18px;\">\'Tracking\' Slider</p>", y_anchor, x_anchor, 2, 12);
        tracking_data_label.add("<p_style=\"font-size:18px;\">\'Tracking\' Slider Data</p>", y_anchor, x_anchor + 21, 2, 14);

        nontracking_slider_label.add("<p_style=\"font-size:18px;\">Non-\'Tracking\' Slider</p>", y_anchor + 6, x_anchor, 2, 13);
        nontracking_data_label.add("<p_style=\"font-size:18px;\">Non-\'Tracking\' Slider Data</p>", y_anchor + 6, x_anchor + 21, 2, 16);

        io_slider_label.add("<p_style=\"font-size:18px;\">Input/Output Slider</p>", y_anchor + 12, x_anchor, 2, 12);
        fb_slider_label.add("<p_style=\"font-size:18px;\">Output Slider</p>", y_anchor + 12, x_anchor + 21, 2, 12);
        special_colour_label.add("<p_style=\"font-size:18px;\">Output slider (No Handle):</p>", y_anchor + 16, x_anchor + 21, 2, 16);

        io_data_label.add("<p_style=\"font-size:18px;\">Input/Output Data</p>", y_anchor + 22, x_anchor, 2, 12);
        fb_data_label.add("<p_style=\"font-size:18px;\">Output data</p>", y_anchor + 22, x_anchor + 21, 2, 12);


        //Sliders - each have different configurations, see the wiki for slider config options
        tracking_slider.add("My data changes as you <b>drag</b> me!", y_anchor + 2, x_anchor, 2, 21, 0, 100, 0, 1, FlexSlider::ALLOW_INPUT);  //arguments are display name, row, column, row span, column span, minimum slider value, maximum slider value, initial slider value, implied denominator, configuration setting 
        nontracking_slider.add("My data changes_when_you_<b>drop</b>_me!", y_anchor + 8, x_anchor, 2, 22, 0, 100, 0, 1, FlexSlider::ALLOW_INPUT + FlexSlider::NOT_TRACKING);         //    Watch_how_<b>my</b>_data_changes!

        io_slider.add("Change_my_value!", y_anchor + 14, x_anchor, 2, 17, 0, 100, 0, 1, FlexSlider::ALLOW_INPUT + FlexSlider::FRAME + FlexSlider::PRECISION_2);
        fb_slider.add("Watch_my_value_change!", y_anchor + 14, x_anchor + 21, 2, 18, 0, 100, 0, 1, FlexSlider::PRECISION_2);
        //Very clearly output slider colour configuration, hides the sliders handle
        special_fb_slider.add("Watch_my_value_change_too!", y_anchor + 18, x_anchor + 21, 2, 19, 0, 100, 0, 1, FlexSlider::PRECISION_2);
        special_fb_slider.set_colour(FlexSlider::SLIDER_ADDPAGE, 0, 0, 0, 0);
        special_fb_slider.set_colour(FlexSlider::HANDLE_DISABLED, 0, 0, 0, 0);


        //Data
        tracking_data.add("", y_anchor + 2, x_anchor + 22, 2, 5, 0, 1, "Units", FlexData::UNITS + FlexData::DIGITS_3);    //arguments are display name, row, column, row span, column span, initial value, implied denominator, units (if configured to display units), configuration setting
        nontracking_data.add("", y_anchor + 8, x_anchor + 22, 2, 5, 0, 1, "Units", FlexData::UNITS + FlexData::DIGITS_3);

        io_data.add("Change_my_value!", y_anchor + 24, x_anchor, 2, 13, 0, 1, "Units", FlexData::ALLOW_INPUT + FlexData::FRAME + FlexData::UNITS);
        fb_data.add("Watch_my_value_change!__", y_anchor + 24, x_anchor + 21, 2, 16, 0, 1, "Units", FlexData::UNITS);


    }
    /**
     * @breif shows the previously hidden FlexElements for the 'Data and Sliders' GUI page.
     *
     * This function assumes the initiate() function has previously been called.
     */
    void setup() {
        isrunning = 1;

        tracking_slider_label.show();
        tracking_data_label.show();
        nontracking_slider_label.show();
        nontracking_data_label.show();
        io_slider_label.show();
        fb_slider_label.show();
        io_data_label.show();
        fb_data_label.show();
        special_colour_label.show();

        tracking_slider.show();
        nontracking_slider.show();
        io_slider.show();
        fb_slider.show();
        special_fb_slider.show();

        tracking_data.show();
        nontracking_data.show();
        io_data.show();
        fb_data.show();
    }

    /**
    * @brief Maintains the FlexData and FlexSlider elements for this GUI page.
   *
   * This function assumes the initiate() function has previosuly been called.
   */
    void run() {
        if (!isrunning)
            return;

        int slider_value = tracking_slider.get();
        tracking_data.update(slider_value);

        slider_value = nontracking_slider.get();
        nontracking_data.update(slider_value);

        float slider_value_float = io_slider.get_f();
        fb_slider.update(slider_value_float);
        special_fb_slider.update(slider_value_float);

        int data_value = io_data.get();
        fb_data.update(data_value);
    }

    /**
    * @brief hides the FlexElements for the 'Graphs' GUI page.
    *
    * All FlexElements can be temporarily hidden with the hide() function, and removed permanently with the remove() function
    */
    void shutdown() {
        isrunning = 0;

        tracking_slider_label.hide();
        tracking_data_label.hide();
        nontracking_slider_label.hide();
        nontracking_data_label.hide();
        io_slider_label.hide();
        fb_slider_label.hide();
        io_data_label.hide();
        fb_data_label.hide();
        special_colour_label.hide();

        tracking_slider.hide();
        nontracking_slider.hide();
        io_slider.hide();
        fb_slider.hide();
        special_fb_slider.hide();

        tracking_data.hide();
        nontracking_data.hide();
        io_data.hide();
        fb_data.hide();
    }

};

#endif