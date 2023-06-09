/**
    Tutorial 1 Solution file

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

#define IRISCONTROLS //allows for the use of iris controls libraries to create guis using the iriscontrols API

#include <windows.h>
#include "ic4_library/iriscontrols4.h"
#include "modbus_client/device_applications/actuator.h"
#include "library_linker.h"
#include "Main_GUI.h"
#include <iostream>

using namespace std;

Actuator motors[1]{     //Initialize an array of actuator objects to be used in the application. In this example we are only using one. Each object is initialized with a comport value, but this value can be changed before communications are initialized for this object. 
  {73, "Orca 1", 1}
};

GUI gui(motors);        //Pass in the array of actuator objects used in the application
IrisControls4* IC4_virtual = &gui;      //Pointer to the gui that will be used to check version compatibility and occasional writing to the console with errors
int ic_port_number;

/**

 * @brief Main method featuring a loop that gets called continuously
 * This is the entry point of the application.
 * Motor frames communication are done here, depending on the mode either Sleep, Force or Position will be commanded
 * Return frame contains information about motor position, force, temperature, power, errors.
 * Additional commands can be injected into the stream.
 */
int main() {

    cout << "Please enter the comport number you would like to use to communicate with Iris Controls and press enter: \n\n";

    //monitor for port number until one has been entered.
    while (ic_port_number == 0) {
        cin >> ic_port_number;
    }

    //call setup function, which will set up the connection on the selected comport
    IC4_virtual->setup(ic_port_number);

    while (1) {
        gui.run();

        motors[0].run_out();
        motors[0].run_in();
    }
    return 0;
}