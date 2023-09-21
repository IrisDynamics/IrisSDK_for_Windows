/** ForceLinking.cpp

*@brief This is an example of connecting two Orca series motors via "Force Linking". 
* In this program high speed modbus communications are established. In order for this to be accomplished, the comport's latency must be set to 1 ms in the device manager.
* The program allows you to change the bias, this is which motor has the advantage over the other. ie pushing on one motor will result in a larger force on the other motor.
* This program also allows you to invert the direction of the link
* In case something goes wrong press ESC key on the keyboard to put the motors to sleep.
* 
* The linking effect is accomplished by setting a spring center between the two motors and streaming new spring center values based on the bias and position input to each motor.
* 
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
#define KEY_LEFT    75
#define KEY_RIGHT   77
#define KEY_DOWN    80
#define KEY_ESCAPE  27

#define NUM_MOTORS 2
Actuator motors[NUM_MOTORS]{
  {0, "Orca A", 1}
, {0, "Orca B", 1}
};
Actuator::ConnectionConfig connection_params;



uint16_t spring_configuration[6] = { 6000                //spring gain
                                 , (uint16_t)(65000)    //spring center low register
                                 , (65000 >> 16)        //spring center high register
                                 ,  0                   //spring coupling 
                                 ,  0                   //spring dead zone 
                                 ,  0};                 //spring saturation

int32_t max_positions[2] = { {130000}, {130000} };      //this is the total range of positions used to normalize position, this can be used to allow small movements on one motor to convert to large movements on another

int32_t final_target[2];        //Spring center target positions
bool mode_set = false;
bool invert = false;
int bias_value = 0;
float weights[2] = { 1, 1 };

int port_number[NUM_MOTORS];

/** @brief This function does the basic set up to set the spring configuration and enable effects, then calculates the appropriate spring center for each motor.
*/
void calculate_targets_haptic() {

    if (!mode_set) {
        for (int i = 0; i < NUM_MOTORS; i++) {
            motors[i].write_registers(S0_GAIN_N_MM, 6, spring_configuration);
            motors[i].write_register(HAPTIC_STATUS, Actuator::Spring0);
            motors[i].set_mode(Actuator::HapticMode);
        }
        mode_set = true;
    }
    if (bias_value == 0) {
        weights[0] = 1;
        weights[1] = 1;
    }
    else if (bias_value < 0) {
        weights[1] = -bias_value;
        weights[0] = 1;
    }
    else if (bias_value > 0) {
        weights[1] = 1;
        weights[0] = bias_value;
    }

    // normalized positions 
    float normal_position[2];
    normal_position[0] = (float)motors[0].get_position_um() / max_positions[0];
    normal_position[1] = (float)motors[1].get_position_um() / max_positions[1];

    // reverse motor A polarity 
    if (invert) normal_position[0] = 1 - normal_position[0];

    float link_target_sum;
    link_target_sum = (normal_position[0] * weights[0] + normal_position[1] * weights[1]) / (weights[0] + weights[1]);

    // denormalize positions 
    final_target[0] = link_target_sum * max_positions[0];
    final_target[1] = link_target_sum * max_positions[1];
    // rereverse motor A polarity
    if (invert) final_target[0] = max_positions[0] - final_target[0];

    motors[0].update_write_stream(2, S0_CENTER_UM, final_target[0]);
    motors[1].update_write_stream(2, S0_CENTER_UM, final_target[1]);




}


//timer is used to allow smooth communications.
void motor_comms() {
    while (1) {
        if (motors[0].is_connected() && motors[1].is_connected()) {
            calculate_targets_haptic();
        }
        for (int i = 0; i < NUM_MOTORS; i++) {
            motors[i].run_in();
            motors[i].run_out();
        }
    }
}

int main()
{
    //allow user to choose comports to connect motors on
    cout << "Force Linking Demo Connect 2 motors to begin. Ensure Comport Latency set to 1 ms in device manager" << endl;
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
        connection_params.target_baud_rate_bps = 1250000;
        connection_params.target_delay_us = 0;
        motors[i].set_connection_config(connection_params);
        motors[i].init();
        motors[i].set_stream_mode(Actuator::MotorWrite);
        motors[i].enable();
    }
     
    thread mthread(motor_comms); //process motor communications in seperate thread

    cout << endl << "Arrow Up: Change the bias towards A" << endl;//key up
    cout << "Arrow Down: Change the bias toward B" << endl;//key down
    cout << "Arrow Left: Change Polarity"<< endl;//key down
    cout << "ESC: Put Motors to Sleep" << endl;//key down

    int c;  //check for keyboard inputs into console
    while (1) {
        c = 0;
        switch ((c = _getch())) {
        case KEY_UP:
            bias_value++;
            cout << "  Bias " + String(bias_value);
            break;
        case KEY_DOWN:
            bias_value--;
            cout << "  Bias " + String(bias_value);
            break;
        case KEY_LEFT:
            if (invert == false) invert = true;
            else invert = false;
            cout << "Polarity inverted"<< endl;
            break;
        case KEY_RIGHT:
            motors[0].set_mode(Actuator::HapticMode);
            motors[1].set_mode(Actuator::HapticMode);
            cout  << "Resume Force Linking" << endl;
            break;
        case KEY_ESCAPE:
            motors[0].set_mode(Actuator::SleepMode);
            motors[1].set_mode(Actuator::SleepMode);
            cout << "Motors to Sleep" << endl;
            cout << "Press Right arrow to return to Force Linking" << endl;
            break;
        default:
            break;
        }
    }
    return 1;
}
