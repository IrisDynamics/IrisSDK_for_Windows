/**
    Tutorial 3 Solution file

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


// Include addional SDK libraries and files within this project
#include <ic4_library/iriscontrols4.h>      // Iris controls 4 library (GUI Handling)
#include "modbus_client/device_applications/actuator.h"    // Actuator objects (abstracts communication to motor)
#include "Main_GUI.h"    // Custom IrisControls GUI interface
#include "library_linker.h"     //File for linking function implementations from libraries 
#include <iostream>     //allows for writing to and from the console

using namespace std;

//Initialize an array of actuator objects to be used in the application. In this example we are only using one. Each object is initialized with a comport value, but this value can be changed before communications are initialized for this object.
Actuator motors[1]{
  {15, "Orca 1", 1}
};

// Pass a pointer to the first motor object to the gui object so that gui elements have access to the motors. 
GUI gui(motors);
//Pointer to the gui that will be used to check version compatibility and occasional writing to the console with errors
IrisControls4* IC4_virtual = &gui;
int ic_port_number;


/**
 * @brief Main loop that gets called continuously
 * This is the entry point of the application
 * Motor frames communication are done here, depending on the mode either Sleep, Force or Position will be commanded
 * Return frame contains information about motor position, force, temperature, power, errors.
 * Additional commands can be injected into the stream.
 */
int main() {

    cout << "Please enter the number of the virtual comport you would like to use to communicate with Iris Controls and press enter: \n\n";

    //monitor for port number until one has been entered.
    while (ic_port_number == 0) {
        cin >> ic_port_number;
    }

    //call setup function, which will set up the connection on the selected comport
    IC4_virtual->setup(ic_port_number);

    while (1) {
        //call run on the gui
        gui.run();

        //call run out and set_force_mN on the motor 
        motors[0].set_force_mN(gui.target_force);
        
        motors[0].run_out();
        motors[0].run_in();
    }
    return 0;
}