/**
    Tutorial Starting Point:
    This project serves as a starting point to follow the IrisSDK for Windows tutorial guide.

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

/*
 * Include addional Iris SDK libraries and files within this project
 */
#include <ic4_library/iriscontrols4.h>                      // Iris controls 4 library (GUI Handling)
#include "modbus_client/device_applications/actuator.h"     // Actuator objects (abstracts communication to motor)
#include "Main_GUI.h"                                       // Custom IrisControls GUI interface
#include "library_linker.h"                                 // Must be included in project to allow for library function implementations
#include <iostream>                                         // Used to accept console inputs from user. 
    
using namespace std;

 //Motor connected on comport 67
Actuator motor(1, "Motor 1", 1);    // Initialize the actuator object to use a dummy comport 
                                    // Changing this port can be done using the set_new_comport functino of the Actuator object. 

GUI gui(motor);                         // Pass by reference the motor object to the gui object so that gui elements have access to the motor
IrisControls4* IC4_virtual = &gui;      // Pointer to the gui that will be used by that will be used to check version compatibility and occasional writing ot the console with errors
int ic_port_number;                     // Comport used to communicate with iriscontrols4.


/*
 * @brief Setup function is called once at the first when the Eagle resets (bootloader button pressed or power cycled)
 */
void setup() {
    motor.set_new_comport(67);      // Change the comport being used by the actuator object to the one that the RS422 cable is connected to
    motor.init();                   // Init the actuator object 
    motor.enable();                 // Start pinging the port for a connected motor (will automatically handshake when detected)
}


/*
 * @brief Entry point for the application 
 * Main loop that gets called continuously
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

    //call setup function, which will set up the connection to Iris Controls on the selected comport
    IC4_virtual->setup(ic_port_number);

    //call setup and enable the motor 
    motor.set_new_comport(67);      //change the comport being used be the actuator object to the one that the RS422 cable is connected to
    motor.init();                   //init the actuator object 
    motor.enable();                 // Start pinging the port for a connected motor (will automatically handshake when detected)

    //main loop which will run while the application is open
    while (1) {
        motor.run_in();     // Parse incoming motor frames (Motor -> Windows Virtual Device)
        motor.run_out();    // Send out motor frames (Windows Virtual Device -> Motor)
        gui.run();          // Run IrisControls connectivity, GUI element interaction, serial parsing found in Main_GUI.h 
    }
}
