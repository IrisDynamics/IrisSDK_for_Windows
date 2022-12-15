#pragma once

/**
    @file Page.h
    @author Aiden Bull <abull@irisdynamics.com>
    @brief Base class to derive from when making pages for IrisControls app
    @version 1.1

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

#include "../ic4_library/iriscontrols4.h"

class Page {

protected:

    GUI_Page page_elements;                            // GUI_Page object to handle displaying/hiding all elements in home page

    bool first_setup = true;                            // Boolean value tracks whether or not the page has been initialized yet

    bool is_running = false;                             // Boolean value tracks whether or not the page is active

public:

    /** 
        @brief Write all custom first setup code here 
    */
    //Need to come up with a better name for this
    virtual void first_setup_f() = 0;

    void setup() {
        if (first_setup) {
            first_setup = false;

            first_setup_f();
        }
        
        show();
        is_running = true;
    }

    /**
        @brief Handle any GUI updates here 
    */
    virtual void run_gui() {
        if (!is_running) return;
    }

    /**
        @brief Handle any non-GUI updates here
    */
    virtual void run() {
        if (!is_running) return;
    }

    /**
        @brief Include any code that's called every time the page is shown here
    */
    virtual void show() {
        page_elements.show();
    }

    /**
        @brief Include code to hide the page here. Don't forget to set is_running to false
    */
    virtual void hide() {
        page_elements.hide();
        is_running = false;
    }

    /**
        @brief Resets all Page elements in GUI view
    */
    virtual void reset() {
        hide();
        first_setup = true;
    }
};
