/**
    Haptics Stream
    @brief This project is intended to make use of different stream modes and making use of motor haptic effects

    @author Rebecca McWilliam <rmcwilliam@irisdynamics.com>
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

#include "iris_gui.h"   //Custom Iris control GUI interface 
#include "ic4_library/iriscontrols4.h"      //Iris Controls library (GUI handling)
#include "modbus_client/device_applications/actuator.h"     //Actuator objects (abstracts communications to motor)
#include "library_linker.h"     //File for linking function implementations from libraries 
#include <iostream> // Allows for writing to and from the console

using namespace std;

Actuator motor(0, "Orca", 1); //the channel number is left as zero here until it is specified in the gui, this can be instead hardcoded if the comport will remain the same
GUI gui(motor);
IrisControls4* IC4_virtual = &gui;

int ic_port_number;

//!< custom communication settings for baud rate and interframe delay to allow faster communication than modbus protocol
Actuator::ConnectionConfig connection_config;

/** @brief Main is called when the program begins - functions as the main project loop */

int main()
{
    cout << "Please enter the virtual comport associated with Iris Controls and press enter: \n\n";  //this can be hardcoded if you're virutal comport number will remain the same

    //monitor for port number until one has been entered.
    while (ic_port_number == 0) {
        cin >> ic_port_number;
    }
    /// This section is optional, if not added will default to 625000 bps and 80us delay
    connection_config.target_baud_rate_bps = 1250000;  //625000 //780000
    connection_config.target_delay_us = 0;

    motor.set_connection_config(connection_config);

    //call setup function, which will set up the connection on the selected comport
    IC4_virtual->setup(ic_port_number);

    while (1) {

        //call run on the gui each loop 
        gui.run();

        //call run in, and run out to send and receive messages to motor
        motor.run_out();
        motor.run_in();

    }
    CloseHandle(gui.hComm);
    return 0;
}