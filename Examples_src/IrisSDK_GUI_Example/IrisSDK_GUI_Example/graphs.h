/**
  @file graphs.h
  @author Kali Erickson <kerickson@irisdynamics.com>
  @version 1.1
  @brief GUI page

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

#ifndef GRAPHS_H_
#define GRAPHS_H_

#include "ic4_library/iriscontrols4.h"

/**
  @class Graphs
  @brief GUI page with examples of 3 different types of graphs and different graph settings
*/
class Graphs {

    FlexPlot time_plot, scatter_plot, dual_plot;

    Dataset time_data_0, time_data_1, time_data_2, scatter_data, dual_data_primary, dual_data_secondary;

    FlexLabel timeplot_upperlabel, timeplot_lowerlabel, scatterplot_label, scatterplot_save_label, dualplot_label;

    FlexSlider timeplot_slider_0, timeplot_slider_1, timeplot_slider_2, dual_slider, dual_inverse_slider;

    FlexButton addition_button, subtraction_button, change_plot_colour;

    FlexData scatter_value;

    int num_presses;

public:

    int isrunning;

    /**
      @brief adds/initializes the FlexElements for the 'Graphs' GUI page.
    */
    void initiate() {

        isrunning = 1;
        int y_anchor = 0;
        int x_anchor = 22;


        //time plot - example of adding more than one Dataset per Flexplot
        time_plot.add("Time_Plot", y_anchor, x_anchor, 10, 15, 0, 10, //arguments are Display name, row, column, rowSpan, columnSpan, configuration settings, min, max
            FlexPlot::DATASET_SELECT_MENU +
            FlexPlot::AXES_LABEL_MENU +
            FlexPlot::TIMEPLOT +
            FlexPlot::WALKING +
            FlexPlot::NAME_LABEL);
        time_plot.set_visible_datapoints(70);
        time_data_0.add(&time_plot, "Slider 1 Data", "Time", "Slider 1 Value", Dataset::TIMEPLOT + Dataset::NONE);  //arguments are Flexplot reference, name, x_label, y_label, configuration settings
        time_data_0.set_colour(75, 75, 255, 255);
        time_data_1.add(&time_plot, "Slider 2 Data", "Time", "Slider 2 Value", Dataset::TIMEPLOT + Dataset::NONE);
        time_data_1.set_colour(255, 0, 0, 255);
        time_data_2.add(&time_plot, "Slider 3 Data", "Time", "Slider 3 Value", Dataset::TIMEPLOT + Dataset::NONE);
        time_data_2.set_colour(0, 255, 0, 255);
        time_plot.set_axes_labels(&time_data_1);
        time_plot.set_axes_labels(&time_data_2);
        time_plot.set_axes_labels(&time_data_0);
        time_data_0.show();
        time_data_1.show();
        time_data_2.show();

        timeplot_upperlabel.add("Select which slider's datatsets to plot", y_anchor + 1, x_anchor + 16, 1, 17);
        timeplot_lowerlabel.add("with_the_\"Datasets\" menu on the Time Plot", y_anchor + 2, x_anchor + 16, 1, 17);
        timeplot_slider_0.add("Slider 1", y_anchor + 3, x_anchor + 18, 2, 12, 0, 10, 0, 1, FlexSlider::ALLOW_INPUT + FlexSlider::TRACKING + FlexSlider::PRECISION_2);
        timeplot_slider_0.set_colour(FlexSlider::HANDLE_COLOUR, 75, 75, 255, 255);
        timeplot_slider_1.add("Slider 2", y_anchor + 5, x_anchor + 18, 2, 12, 0, 10, 0, 1, FlexSlider::ALLOW_INPUT + FlexSlider::TRACKING + FlexSlider::PRECISION_2);
        timeplot_slider_1.set_colour(FlexSlider::HANDLE_COLOUR, 255, 0, 0, 255);
        timeplot_slider_2.add("Slider 3", y_anchor + 7, x_anchor + 18, 2, 12, 0, 10, 0, 1, FlexSlider::ALLOW_INPUT + FlexSlider::TRACKING + FlexSlider::PRECISION_2);
        timeplot_slider_2.set_colour(FlexSlider::HANDLE_COLOUR, 0, 255, 0, 255);

        //scatter plot example
        scatter_plot.add("Scatter Plot", y_anchor + 11, x_anchor, 10, 15, 0, 100,
            FlexPlot::WALKING +
            FlexPlot::NAME_LABEL +
            FlexPlot::SAVE_DATA_BUTTON);
        scatter_plot.set_visible_datapoints(20);
        scatter_data.add(&scatter_plot, "Value", "# Of Button Presses", "Value", Dataset::CIRCLE + Dataset::SCATTER_PLOT);
        scatter_plot.set_axes_labels(&scatter_data);
        scatter_data.show();

        scatterplot_label.add("Adjust the value to create the Scatter Plot's dataset", y_anchor + 12, x_anchor + 16, 1, 22);
        addition_button.add("Add", -1, y_anchor + 15, x_anchor + 25, 2, 4);
        subtraction_button.add("Subtract", -1, y_anchor + 17, x_anchor + 25, 2, 4);
        scatter_value.add("Value ", y_anchor + 16, x_anchor + 30, 1, 7, 0, 1);  //note that if none of the config options are needed (config param = 0), then no configuration parameter should be passed
        change_plot_colour.add("Change Plot Colour", 0, y_anchor + 15, x_anchor + 18, 3, 7);
        scatterplot_save_label.add("Press Save Data to store the dataset in a text file", y_anchor + 19, x_anchor + 16, 2, 21); //The text file will be stored in the 'savedData' directory which will appear in the same directory as the iriscontrols4 executable

        //Dual axis plot example
        dual_plot.add("Dual Axis Plot", y_anchor + 22, x_anchor, 10, 15, 0, 100,
            FlexPlot::LEGEND +
            FlexPlot::LEGEND_BUTTON +
            FlexPlot::MOUSE_BUTTON +
            FlexPlot::TIMEPLOT +
            FlexPlot::WALKING +
            FlexPlot::NAME_LABEL);
        dual_plot.set_secondary_range(0, 10);
        dual_plot.set_visible_datapoints(70);
        dual_data_primary.add(&dual_plot, "Slider Value", "Time", "Value", Dataset::TIMEPLOT + Dataset::DIAMOND);
        dual_data_secondary.add(&dual_plot, "Inverse Slider Value", "Time", "(100 - Value) / 10", Dataset::TIMEPLOT + Dataset::SECONDARY_Y_AXIS);
        dual_data_primary.show();
        dual_data_secondary.show();

        dualplot_label.add("Adjust the input slider to test the dual axis feature", y_anchor + 23, x_anchor + 16, 1, 22);
        dual_slider.add("Value             ", y_anchor + 25, x_anchor + 18, 2, 16, 0, 100, 0, 1, FlexSlider::ALLOW_INPUT + FlexSlider::TRACKING + FlexSlider::FRAME + FlexSlider::PRECISION_2);
        dual_inverse_slider.add("(100 - Value) / 10", y_anchor + 27, x_anchor + 18, 2, 16, 0, 10, 10, 1, FlexSlider::PRECISION_2);
    }

    /**
      @brief shows the previously hidden FlexElements for the 'Graphs' GUI page.

       This function assumes the initiate() function has previosuly been called.
    */
    void setup() {
        isrunning = 1;

        time_plot.show();
        scatter_plot.show();
        dual_plot.show();

        timeplot_upperlabel.show();
        timeplot_lowerlabel.show();
        timeplot_slider_0.show();
        timeplot_slider_1.show();
        timeplot_slider_2.show();

        scatterplot_label.show();
        addition_button.show();
        subtraction_button.show();
        scatter_value.show();
        change_plot_colour.show();
        scatterplot_save_label.show();

        dualplot_label.show();
        dual_slider.show();
        dual_inverse_slider.show();
    }

    /**
       @brief Maintains the graphs for this GUI page and their respective FlexElements.

      This function assumes the initiate() function has previosuly been called.
    */
    void run() {
        if (!isrunning)
            return;

        //updating time plot datasets
        float x = micros();
        float y = timeplot_slider_0.get_f();
        time_data_0.add_data(x, y);

        y = timeplot_slider_1.get_f();
        time_data_1.add_data(x, y);

        y = timeplot_slider_2.get_f();
        time_data_2.add_data(x, y);

        //updating scatter plot dataset
        //first update scatter_value based on button presses
        if (addition_button.pressed()) {
            scatter_value.update(scatter_value.get() + 5);
            num_presses++;
            scatter_data.add_data(num_presses, scatter_value.get());
        }
        if (subtraction_button.pressed()) {
            scatter_value.update(scatter_value.get() - 5);
            num_presses++;
            scatter_data.add_data(num_presses, scatter_value.get());
            if (scatter_value.get() < 0)
                scatter_value.update(0);
        }
        //now update plot colour based on colour change button
        if (change_plot_colour.toggled()) {
            if (change_plot_colour.get() == 1) {
                scatter_plot.set_colour(FlexPlot::TEXT, 255, 255, 255, 255);
                scatter_plot.set_colour(FlexPlot::FRAME, 255, 255, 255, 255);
            }
            else {
                scatter_plot.set_colour(FlexPlot::TEXT, 255, 125, 0, 255);
                scatter_plot.set_colour(FlexPlot::FRAME, 0, 0, 0, 0);
            }
        }

        //updating dual plot datasets
        //first update dual plot sliders
        dual_inverse_slider.update((100 - dual_slider.get_f()) / 10);

        dual_data_primary.add_data(x, dual_slider.get_f());
        dual_data_secondary.add_data(x, dual_inverse_slider.get_f());

    }

    /**
      @brief hides the FlexElements for the 'Graphs' GUI page.

      All FlexElements can be temporarily hidden with the hide() function, and removed permanently with the remove() function
    */
    void shutdown() {
        isrunning = 0;

        time_plot.hide();
        scatter_plot.hide();
        dual_plot.hide();

        timeplot_upperlabel.hide();
        timeplot_lowerlabel.hide();
        timeplot_slider_0.hide();
        timeplot_slider_1.hide();
        timeplot_slider_2.hide();

        scatterplot_label.hide();
        addition_button.hide();
        subtraction_button.hide();
        scatter_value.hide();
        change_plot_colour.hide();
        scatterplot_save_label.hide();

        dualplot_label.hide();
        dual_slider.hide();
        dual_inverse_slider.hide();
    }
};

#endif#pragma once
#pragma once
