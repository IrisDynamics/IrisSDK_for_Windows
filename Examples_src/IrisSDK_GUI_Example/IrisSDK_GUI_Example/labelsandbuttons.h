/**
* @file labelsandbuttons.h
* @author Kali Erickson <kerickson@irisdynamics.com>
* @version 1.1
* @brief GUI page with examples of different FlexLabel and FlexButton configurations

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

#ifndef LABELSANDBUTTONS_H_
#define LABELSANDBUTTONS_H_

#include "ic4_library/iriscontrols4.h"

class LabelsAndButtons {

    typedef enum {
        BIG,
        SMALL
    }SizeLabelState;

    FlexLabel labels, buttons, push_buttons, toggle_buttons,                              //section labels
        size_label_big, size_label_small, colour_label, bold_label, light_label,    //example labels
        button1_label, button2_label, button3_label, button4_label;                 //button labels

    FlexButton button1, button2, button3, button4;  //buttons

    int press_count1;
    SizeLabelState size_label_state;
    FlexData num_pushes;

    int press_count2;

    FlexLabel button3_light_label;
    FlexLabel button3_bold_label;

public:

    int isrunning;

    /**
    * @brief adds/initializes the FlexElements for the 'Labels and Buttons' GUI page.
    */
    void initiate() {

        isrunning = 1;
        int x_anchor = 25;
        int y_anchor = 1;

        //section labels
        labels.add("<p_style=\"font-size:22px;\">Labels</p>", y_anchor, x_anchor, 2, 5);     //arguments are display name, row, column, row span, column span. Example of html tags in the display name
        buttons.add("<p_style=\"font-size:22px;\">Buttons</p>", y_anchor, x_anchor + 16, 2, 5);

        push_buttons.add("<p_style=\"font-size:15px;\">Push_Buttons</p>", y_anchor + 2, x_anchor + 16, 1, 6);
        toggle_buttons.add("<p_style=\"font-size:15px;\">Toggle_Buttons</p>", y_anchor + 13, x_anchor + 16, 1, 7);

        //example labels 
        size_label_small.add("This_label_is_small", y_anchor + 5, x_anchor, 2, 8);
        size_label_small.hide();

        size_label_big.add("<p_style=\"font-size:20px;\">This label is big</p>", y_anchor + 5, x_anchor, 2, 11);
        colour_label.add("<p_style=\"font-size:14px;\">This label changes colour</p>", y_anchor + 9, x_anchor, 2, 13);
        light_label.add("<p_style=\"font-size:15px;\">This label is light</p>", y_anchor + 16, x_anchor, 2, 11);
        light_label.hide();
        bold_label.add("<b><p_style=\"font-size:15px;\">This label is bold</p></b>", y_anchor + 16, x_anchor, 2, 11);



        //button labels
        button1_label.add("Push 3 times to change the label's size", y_anchor + 4, x_anchor + 16, 1, 16);
        button2_label.add("Push to change the label and button colour", y_anchor + 8, x_anchor + 16, 1, 17);
        button3_label.add("Toggle to change font", y_anchor + 15, x_anchor + 16, 1, 14);
        button4_label.add("Toggle to hide the other elements", y_anchor + 19, x_anchor + 16, 1, 15);


        //buttons
        button1.add("Size", -1, y_anchor + 5, x_anchor + 16, 2, 5);   //arguments are display name, initial state/value, row, column, row span, column span

        //Button1 demonstration data. See dataandsliders.h for FlexData initialization syntax
        num_pushes.add("", y_anchor + 5, x_anchor + 21, 2, 7, 0, 1, "Pushes", FlexData::UNITS);

        button2.add("Colour", -1, y_anchor + 9, x_anchor + 16, 2, 5);
        button3.add("State", 0, y_anchor + 16, x_anchor + 16, 2, 5);
        button3_light_label.add("Light", y_anchor + 16, x_anchor + 22, 2, 4);
        button3_light_label.hide();
        button3_bold_label.add("Bold", y_anchor + 16, x_anchor + 22, 2, 4);
        button4.add("Hide", 0, y_anchor + 20, x_anchor + 16, 2, 5);

        press_count1 = 0;
        press_count2 = 0;
        size_label_state = BIG;

    }

    /**
    * @brief shows previously hidden elements
    *
    * This function assumes the initiate() function has previously been called.
    */
    void setup() {
        isrunning = 1;

        if (button4.get()) {
            button4_label.show();
            button4.show();
        }
        else {
            //section labels
            labels.show();
            buttons.show();

            push_buttons.show();
            toggle_buttons.show();

            //example labels
            size_label_big.show();
            size_label_state = BIG;
            colour_label.show();
            bold_label.show();
            button3.set_toggle(0);

            //button labels
            button1_label.show();
            num_pushes.show();
            button2_label.show();
            button3_label.show();
            button3_bold_label.show();
            button4_label.show();

            //buttons
            button1.show();
            button2.show();
            button3.show();
            button4.show();
        }

    }

    /**
    * @brief maintains and updates FlexElements for this GUI page
    *
    * This function assumes the initiate() function has previosuly been called.
    */
    void run() {
        if (isrunning == 0)
            return;

        //button1 and size label maintenance
        if (button1.pressed()) {
            press_count1++;
            num_pushes.update(press_count1);
            if (press_count1 == 3) {
                if (size_label_state == BIG) {
                    size_label_big.hide();
                    size_label_small.show();
                    size_label_state = SMALL;
                }
                else if (size_label_state == SMALL) {
                    size_label_small.hide();
                    size_label_big.show();
                    size_label_state = BIG;
                }
                press_count1 = 0;
            }
        }

        //example of changing an elements colour
              //button2 and colour label maintenance
        if (button2.pressed()) {
            press_count2++;
            switch (press_count2) {
            case 1:
                colour_label.set_colour(FlexLabel::TEXT, 255, 255, 255, 255);
                colour_label.set_colour(FlexLabel::BACKGROUND, 255, 0, 0, 255);

                button2.set_colour(FlexButton::TEXT, 255, 0, 0, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 255, 0, 0, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 255, 0, 0, 255);
                break;
            case 2:
                colour_label.set_colour(FlexLabel::TEXT, 255, 255, 255, 255);
                colour_label.set_colour(FlexLabel::BACKGROUND, 0, 200, 0, 255);

                button2.set_colour(FlexButton::TEXT, 0, 200, 0, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 0, 200, 0, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 0, 200, 0, 255);
                break;
            case 3:
                colour_label.set_colour(FlexLabel::TEXT, 255, 255, 255, 255);
                colour_label.set_colour(FlexLabel::BACKGROUND, 0, 0, 255, 255);

                button2.set_colour(FlexButton::TEXT, 75, 75, 255, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 75, 75, 255, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 75, 75, 255, 255);
                break;
            case 4:
                colour_label.set_colour(FlexLabel::TEXT, 255, 0, 0, 255);
                colour_label.set_colour(FlexLabel::BACKGROUND, 0, 0, 0, 0);

                button2.set_colour(FlexButton::TEXT, 255, 0, 0, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 255, 0, 0, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 255, 0, 0, 255);
                break;
            case 5:
                colour_label.set_colour(FlexLabel::TEXT, 0, 200, 0, 255);

                button2.set_colour(FlexButton::TEXT, 0, 200, 0, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 0, 200, 0, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 0, 200, 0, 255);
                break;
            case 6:
                colour_label.set_colour(FlexLabel::TEXT, 75, 75, 255, 255);

                button2.set_colour(FlexButton::TEXT, 75, 75, 255, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 75, 75, 255, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 75, 75, 255, 255);
                break;
            case 7:
                press_count2 = 0;
                colour_label.set_colour(FlexLabel::TEXT, 255, 125, 0, 255);

                button2.set_colour(FlexButton::TEXT, 255, 125, 0, 255);
                button2.set_colour(FlexButton::HOVER_TEXT, 255, 125, 0, 255);
                button2.set_colour(FlexButton::HOVER_BORDER, 255, 125, 0, 255);
            }
        }

        if (button3.toggled()) {
            if (button3.get()) {
                button3_bold_label.hide();
                button3_light_label.show();
                bold_label.hide();
                light_label.show();
            }
            else {
                button3_light_label.hide();
                button3_bold_label.show();
                light_label.hide();
                bold_label.show();
            }
        }

        //example of toggle logic, and hiding and showing GUI elements
        if (button4.toggled()) {
            if (button4.get()) {
                shutdown();
                button4_label.show();
                button4.show();
                isrunning = 1;
            }
            else {
                setup();
            }
        }

    }

    /**
    * @brief hides the FlexElements for the 'Labels and Buttons' GUI page.
    *
    * All FlexElements can be temporarily hidden with the hide() function, and removed permanently with the remove() function
    */
    void shutdown() {
        isrunning = 0;

        //section labels
        labels.hide();
        buttons.hide();

        push_buttons.hide();
        toggle_buttons.hide();

        //example labels
        size_label_big.hide();
        size_label_small.hide();
        colour_label.hide();
        bold_label.hide();
        light_label.hide();



        //button labels
        button1_label.hide();
        num_pushes.hide();
        button2_label.hide();
        button3_label.hide();
        button3_light_label.hide();
        button3_bold_label.hide();
        button4_label.hide();


        //buttons
        button1.hide();
        button2.hide();
        button3.hide();
        button4.hide();
    }

};

#endif#pragma once
