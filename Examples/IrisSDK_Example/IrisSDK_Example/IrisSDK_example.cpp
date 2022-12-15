/**
    @file IrisSDK_example.cpp
    @author Kate Colwell <kcolwell@irisdynamics.com>
    @brief Example GUI page for the Iris SDK
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


#include "windows_gui.h"        //Custom Iris Controls GUI interface 
#include "ic4_library/iriscontrols4.h"      //Iris Controls library (GUI handling)
#include "modbus_client/device_applications/actuator.h"     //Actuator object (abstracts communication to motor)
#include "library_linker.h"     //File for linking function implementations
#include <iostream>         //Allos for writing to and from the console

using namespace std;

//Actuators created with dummy ports 
Actuator motors[2]{
  {73, "Orca 1", 1}
, {72, "Orca 2", 1}
};

int array_size = 2;
GUI test_gui(motors, array_size);
IrisControls4* IC4_virtual = &test_gui;

bool flag = true;
int ic_port_number;

/** @brief Allows for a delay so that the processor isn't being spammed  */
void delayMicroSeconds(float microseconds)
{
    __int64 timeEllapsed;
    __int64 timeStart;
    __int64 timeDelta;

    QueryPerformanceFrequency((LARGE_INTEGER*)(&timeDelta));

    __int64 timeToWait = (double)timeDelta * (double)microseconds / 1000000.0f;

    QueryPerformanceCounter((LARGE_INTEGER*)(&timeStart));

    timeEllapsed = timeStart;

    while ((timeEllapsed - timeStart) < timeToWait)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)(&timeEllapsed));

    };
}

/** @brief Main is called when the program begins - functions as the main project loop */

int main()
{
    cout << "Please enter the number of the virtual comport you would like to use to comminicate with Iris Controls and press enter: \n\n";

    //monitor for port number until one has been entered.
    while (ic_port_number == 0) {
        cin >> ic_port_number;
    }

    //call setup function, which will set up the connection on the selected comport
    IC4_virtual->setup(ic_port_number);

    while (1) {

        //call run on the gui each loop 
        test_gui.run();

        //call run in on all motors, and run out on any enabled motors
        for (int i = 0; i < array_size; i++) {
            motors[i].run_out();
            motors[i].run_in();
        }
    }
    CloseHandle(test_gui.hComm);
    return 0;
}

