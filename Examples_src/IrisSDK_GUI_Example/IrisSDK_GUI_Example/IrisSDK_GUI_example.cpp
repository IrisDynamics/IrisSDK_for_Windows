/**
    GUI Example File

    @author Kate Colwell <kcolwell@irisdynamics.com>
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
#define IRISCONTROLS

#include "ic4_library/iriscontrols4.h"      //Iris Controls library (GUI handling)
#include "library_linker.h"     //File for linking function implementations for libraries 
#include "gui.h"        //Custom Iris control GUI interfaces
#include "iostream"     //Allows for writing to and from the console

using namespace std;

//Now create an instance of this class for the virtual windows device and IrisControls4 to interact with
GUI gui;
IrisControls4* IC4_virtual = &gui;  //The ic4_library will access this object through the IC4_virtual pointer, so it must be connected to the class instance that was just declared

//Variable for controlling polling rate
unsigned int last_iriscontrols4_update;

//port number to use when communicating with ic4
int ic_port_number;


/**

 * @brief Main method featuring a loop that gets called continuously
 * This is the entry point of the application.
 * Motor frames communication are done here, depending on the mode either Sleep, Force or Position will be commanded
 * Return frame contains information about motor position, force, temperature, power, errors.
 * Additional commands can be injected into the stream.
 */
int main() {

    cout << "Please enter the comport number you would like to use to comminicate with Iris Controls and press enter: \n\n";

    //monitor for port number until one has been entered.
    while (ic_port_number == 0) {
        cin >> ic_port_number;
    }

    //call setup function, which will set up the connection on the selected comport
    IC4_virtual->setup(ic_port_number);

    while (1) {
        gui.check();
        gui.send();

        // Example of detecting a new connection to the IrisControls4 app
        if (gui.new_connection()) {
            gui.initiate();           //calls the gui function that will add/make visible all the desired elements
        }

        // Example of checking and updating the IrisControls4 elements peridocially
        if (gui.is_connected() && (millis() - last_iriscontrols4_update > 16)) {
            last_iriscontrols4_update = millis();
            gui.run();                //calls the gui function that will maintain/update all the GUI elements
            gui.end_of_frame();       //Used to track the framerate of the GUI, MUST be included to avoid disconnection
        }
    }
    return 0;
}