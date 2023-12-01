/** KinTrigger_with_Logging.cpp : This file contains the 'main' function. Program execution begins and ends there.

*@brief This example allows connection to an Orca Series Motor. The console window will ask for the comport associated with the RS422 cable of the motor. 
* The program will monitor for up arrow or down arrow keyboard inputs. When up arrow is pressed the motor's mode will toggle between kinematic mode and sleep mode. 
* When in kinematic mode, motion ID 0 will be triggered using the down arrow. 
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
#include "irisSDK_libraries/Log.h"
#include <iostream>
#include <conio.h>
#include <thread>
#include <chrono>
using namespace std;

int port_number;
#define KEY_UP    72
#define KEY_DOWN  80
#define MOTION_ACTIVE   0x8000
Actuator motor(0, "Orca", 1);
Actuator::ConnectionConfig connection_params;
Log motor_log;
std::string file_name = "Orca_Log";
bool was_moving = false;
bool is_moving = false;
uint64_t start_time = 0;
int32_t last_position = 0;

//timer is used to allow smooth communications.
void motor_comms() {
    while (1) {
        motor.run_in();
        motor.run_out();
        is_moving = (motor.get_orca_reg_content(KINEMATIC_STATUS) & MOTION_ACTIVE);
        if ((motor.get_orca_reg_content(MODE_OF_OPERATION) == Actuator::KinematicMode) && motor.new_data() && is_moving) {
            if (was_moving != is_moving) {
                std::string message = std::string()
                    + "==New Motion Triggered==\n"
                    + "\tTime(ms)"
                    + "\tPosition" 
                    + "\tSpeed"
                    + "\tForce" 
                    + "\tPower" 
                    + "\tVoltage"
                    + "\tErrors" 
                    ;
                motor_log.write(message);
                start_time = micros();
            }
            else {
                float ms_time = (micros() - start_time) / 1000.;
                
                std::string message = std::string()
                    + "\t" + std::to_string(ms_time)
                    + "\t" + std::to_string(motor.get_position_um())
                    + "\t" + std::to_string((motor.get_position_um()-last_position)/ ms_time)
                    + "\t" + std::to_string(motor.get_force_mN())
                    + "\t" + std::to_string(motor.get_power_W())
                    + "\t" + std::to_string(motor.get_voltage_mV())
                    + "\t" + std::to_string(motor.get_errors())
                    ;
                motor_log.write(message);
            }

        }
        last_position = motor.get_position_um();
        was_moving = is_moving;
    }
}

int main()
{
    cout << "Enter port of the motor's RS422 cable" << endl;
    while (1) {
        string port_input;
        getline(cin, port_input);
        try {
            port_number = stoi(port_input);
            break;
        }
        catch (exception e) {
            cout << "Error with entry. Please enter an integer." << endl;
        }
    }

    cout << "Using port " + String(port_number) << endl;
    cout << endl << "Arrow Up: Toggle Kinematic /Sleep Mode" << endl;//key up
    cout << "Arrow Down: Re Trigger Motion ID 0 and Logging" << endl << endl;//key up

    motor_log.open(file_name);
    
    motor.set_new_comport(port_number);
    connection_params.target_baud_rate_bps = 1250000;
    connection_params.target_delay_us = 0;
    motor.set_connection_config(connection_params);
    motor.init();
    motor.set_stream_mode(Actuator::MotorRead);
    motor.update_read_stream(1, KINEMATIC_STATUS);
    motor.enable();
    
    thread mthread(motor_comms);
    //
    int c;
    while (1) {
        c = 0;
        switch ((c = _getch())) {
        case KEY_UP:
            if (motor.get_mode_of_operation() != Actuator::KinematicMode) {
                cout << "Entering Kinematic Mode" << endl;
                motor.set_mode(Actuator::KinematicMode);
            }
            else {
                cout << "Entering Sleep Mode" << endl;
                motor.set_mode(Actuator::SleepMode);
            }

            break;
        case KEY_DOWN:
            cout << "Motion Trigger" << endl;  
            motor.trigger_kinematic_motion(0); //this value could be configurable
            break;
        default:
            break;
        }
    }
    return 1;
}
