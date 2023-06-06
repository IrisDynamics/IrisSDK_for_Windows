#pragma once
/**
* @file gui.h
* @author Kali Erickson <kerickson@irisdynamics.com>
* @version 1.1
* @brief  Control flow for IrisControls4 Example GUI
          This is intended to demonstrate how to create and interact with
          GUI elements on the IrisControls4 application.
          No actuator control or communications are present here.
          Parsing serial commands from the GUI is also demonstrated.

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


#ifndef GUI_H_
#define GUI_H_

#include "labelsandbuttons.h"
#include "dataandsliders.h"
#include "graphs.h"
#include "ic4_library/device_drivers/windows/ic4_windows.h"

/**
* @class GUI
* @brief Contains all graphic elements of the GUI, and the overridden parseApp function for this program
*/
class GUI : public IC4_windows {

    //classes that represent certain pages of the GUI
    LabelsAndButtons page1;
    DataAndSliders page2;
    Graphs page3;

    // The following are one type of GUI element supported by the SDK: FlexButtons. For initialization syntax and other options, see labelsandbuttons.h
    FlexButton labels_buttons, data_sliders, graphs;

    //to track which page of the gui is requested to be running
    int open_page;

public:
    GUI( ) 
    {
        set_server_name("GUI Example");
        set_device_id("windows");
    }

    /**
    * @brief adds/initializes the FlexElements for the GUI with the desired confugrations and effects
    */
    void initiate() {
        // Here is where the initial state of the GUI is set up

        gui_set_grid(35, 65);   //sets the size of the iriscontrols4 window

        int x_anchor = 0;
        int y_anchor = 24;

        labels_buttons.add("Labels and Buttons", 1, y_anchor, x_anchor, 2, 6);  //see the labelsandbuttons.h file for add() syntax
        data_sliders.add("Data_and_Sliders", 0, y_anchor, x_anchor + 7, 2, 6);  //note that spaces in element names can also be represented by underscores
        graphs.add("Graphs", 0, y_anchor, x_anchor + 14, 2, 6);

        //note that based on the element type, configuration, and name length, the row and column spans may need to be increased
        //flush between adding each page as there are many elements on each

        page3.initiate();
        IC4_virtual->flush();
        page3.shutdown();
        IC4_virtual->flush();
        page2.initiate();
        IC4_virtual->flush();
        page2.shutdown();
        IC4_virtual->flush();
        page1.initiate(); //leaves page1/labelsandbuttons page open as the main/entry page

        //example of printing to the console
        print_l("\rWelcome to The IrisControls4 Example GUI!\r\r");
        print_l("Press the buttons below this console to switch pages and explore the different GUI elements.\r\r");
        print_l("Or test this GUI's custom commands by typing any of the following commands below:\r");
        print_l("- test_0\r- test_1 \'an integer\' \'a decimal\'\r\r");
    }

    /**
    * @brief maintains and updates the states of the GUI elements
    */
    void run() {

        //check if the page has been changed
        change_page();

        //will run only if the respective page is open
        page1.run();
        page2.run();
        page3.run();

    }

    /**
     * @brief this parser contains special text commands for this application
     *
     * Custom serial commands can be added here by following the existing format.
     */
    int parse_app(char* cmd, char* args) override {
        START_PARSING

            COMMAND_IS "test_0" THEN_DO
            print_l("Test Passed!\r");

        COMMAND_IS "test_1" THEN_DO
            unsigned int index = 0;
        int a = IrisControls4::parse_int(args, index);
        double b = IrisControls4::parse_double(args, index);
        print_l("test passed: arg 1 is ");
        print_d(a);
        print_l("; arg 2 is ");
        print_f(b);

        FINISH_PARSING

            return 1;
    }

    /**
    * @brief  Runs the setup/shutdown sequences for the three pages according to the value of open_page.
    */
    void change_page() {
        int page = navigation();
        switch (page) {
        case 1:
            if (!page1.isrunning) {
                page3.shutdown();
                page2.shutdown();
                page1.setup();
            }
            break;
        case 2:
            if (!page2.isrunning) {
                page3.shutdown();
                page1.shutdown();
                page2.setup();
            }
            break;
        case 3:
            if (!page3.isrunning) {
                page2.shutdown();
                page1.shutdown();
                page3.setup();
            }
            break;
        }
    }

    /**
    * @brief Determines the current page number based on button toggle logic.
    * @return An integer representing the current page. 0 for main 'Labels and Buttons' page, 1 for 'Data and Sliders' page, 2 for 'Graphs' page
    */
    int navigation() {

        if (labels_buttons.toggled()) {
            if (labels_buttons.get()) {
                open_page = 1;
                data_sliders.set_toggle(0);
                graphs.set_toggle(0);
            }
            else if (open_page == 1) {
                labels_buttons.set_toggle(1);
            }
        }

        if (data_sliders.toggled()) {
            if (data_sliders.get()) {
                open_page = 2;
                labels_buttons.set_toggle(0);
                graphs.set_toggle(0);
            }
            else if (open_page == 2) {
                data_sliders.set_toggle(1);
            }
        }

        if (graphs.toggled()) {
            if (graphs.get()) {
                open_page = 3;
                labels_buttons.set_toggle(0);
                data_sliders.set_toggle(0);
            }
            else if (open_page == 3) {
                graphs.set_toggle(1);
            }
        }
        return open_page;
    }

};

#endif