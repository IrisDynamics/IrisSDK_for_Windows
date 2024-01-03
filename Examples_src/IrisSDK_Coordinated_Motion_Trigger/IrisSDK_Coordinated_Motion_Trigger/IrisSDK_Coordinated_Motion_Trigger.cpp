/** Coordinated_trigger.cpp

*@brief This is a very simple projects that triggers a kinematic effect simultaneously between two actuators.
*
*   @author Rebecca McWilliam <rmcwilliam@irisdynamics.com>
*    @version 1.1
*
*    @copyright Copyright 2022 Iris Dynamics Ltd
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*
*    For questions or feedback on this file, please email <support@irisdynamics.com>.
*/

#include "library_linker.h"
#include "modbus_client/device_applications/actuator.h"
#include <iostream>
#include <conio.h>
#include <thread>
using namespace std;

//Define Keyboad inputs to console
#define KEY_UP      72

#define NUM_MOTORS 2
Actuator motors[NUM_MOTORS]{
  {0, "Orca A", 1}
, {0, "Orca B", 1}
};

int port_number[NUM_MOTORS];

void motor_comms() {
    while (1) {
        for (int i = 0; i < NUM_MOTORS; i++) {
            motors[i].run_in();
            motors[i].run_out();
        }
    }
}

int main()
{
    //allow user to choose comports to connect motors on
    cout << "Coordinated Trigger" << endl;
    cout << endl << "Enter port of the motor A's RS422" << endl;
    while (1) {
        string port_input;
        getline(cin, port_input);
        try {
            port_number[0] = stoi(port_input);
            break;
        }
        catch (exception e) {
            cout << "Error with entry. Please enter an integer." << endl;
        }
    }
    cout << "Enter port of the motor B's RS422" << endl;
    while (1) {
        string port_input;
        getline(cin, port_input);
        try {
            port_number[1] = stoi(port_input);
            break;
        }
        catch (exception e) {
            cout << "Error with entry. Please enter an integer." << endl;
        }
    }
    cout << "Using ports " + String(port_number[0]) + "and " + String(port_number[1]) << endl;

    //establish hi speed modbus stream 
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].set_new_comport(port_number[i]);
        motors[i].init();
    }
    thread mthread(motor_comms); //process motor communications in seperate thread
    cout << "Press Up Arrow to simultaneously trigger motion id 0 on both motors" << endl;
    int c;  //check for keyboard inputs into console
    while (1) {
        c = 0;
        switch ((c = _getch())) {
        case KEY_UP:
            for (int i = 0; i < NUM_MOTORS; i++) {
                motors[i].set_mode(Actuator::KinematicMode);
                motors[i].trigger_kinematic_motion(0);
            }
            break;
        default:
            break;
        }
    }
    return 1;
}
