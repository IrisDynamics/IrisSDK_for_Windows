// IrisSDK_Write_Stream_with_Haptics.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
/**@brief The IrisSDK_Write_Stream_with_Haptics is intended to demonstrate the use of updating a haptic effect with a write stream while maintaining a
* stream in haptic mode.
* This project does not require the use of IrisControls, or virtual com port
* This is set up to communicate with 2 motors simultaneously
* One of the motors is streamed with a update to the constant force effect value based on a sine wave over time
* The other motor has a spring effect with a saturation value that gets updated as different positions thresholds are reached which essentially results in constant
* forces in each zone
* Both motors have a spring with a large deep zone to simulate stops on the ends of travel.
* Note ensure the motor has been fully retracted before running this program as the motor powers up with it's current position being zero and updates it as it is retracted.
* 
*   @author Rebecca McWilliam <rmcwilliam@irisdynamics.com>
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

#include "library_linker.h"
#include "modbus_client/device_applications/actuator.h"
#include "irisSDK_libraries/Signal_Generator.h"
#include <iostream>

#define NUM_MOTORS 2
Actuator motor[NUM_MOTORS]{ {0, "orca 0", 1}, {0, "orca 1", 1} };
//IrisControls4* IC4_virtual;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********UPDATA THESE TO YOUR RS422 PORT VALUES ensure they have been updated to have com port latency to 1 ms*******/////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u16 port_number[NUM_MOTORS] = { 81 , 74 }; //In this case these values have been hardcoded but if the port values are not consistent 
                                      //they can be adjusted before initializing the object as needed
//!< custom communication settings for baud rate and interframe delay to allow faster communication than modbus protocol
Actuator::ConnectionConfig connection_config;

u32 const_force_effect[NUM_MOTORS] = { 0, 0 };
u32 zone_position_um[3] = { 100000, 75000, 50000};
u32 zone_force_N[3] = { 20, 40, 60 };
SineWave sine_force_signal;
u8 current_zone = 0;
u8 last_zone = 8;
bool was_connected[NUM_MOTORS] = { false, false };

void update_zones() {
   if (motor[0].get_position_um() < zone_position_um[2]) current_zone = 2;
   else if (motor[0].get_position_um() < zone_position_um[1]) current_zone = 1;
   else current_zone = 0;
   if (last_zone != current_zone) {
    last_zone = current_zone;
    motor[0].set_spring_effect(1, 10000, zone_position_um[current_zone], 0, zone_force_N[current_zone], 2);
   }
}

int main()
{
    sine_force_signal.init(0, 40000, 10000, 0.1);

    /// This section is optional, if not added will default to 625000 bps and 80us
    connection_config.target_baud_rate_bps = 1250000;// 500000;  //625000 //780000
    connection_config.target_delay_us = 0;

    //set comport number, then init and enable streaming.
    for (int i = 0; i < NUM_MOTORS; i++) {
        motor[i].set_connection_config(connection_config);
        motor[i].set_new_comport(port_number[i]);
        motor[i].init();
        motor[i].enable();

    }



    while (1) {
        if (motor[0].is_connected()) {
            update_zones();
            if (!was_connected[0]) {
                motor[0].set_spring_effect(0, 10000, 60000, 40);
                motor[0].set_mode(Actuator::HapticMode);
                motor[0].set_stream_mode(Actuator::MotorRead);
                motor[0].update_read_stream(2, SHAFT_SPEED_MMPS);
                motor[0].enable_haptic_effects(Actuator::Spring0 + Actuator::Spring1);
            }
        }
        else was_connected[0] = false;
        if (motor[1].is_connected()) {
            sine_force_signal.run();
            motor[1].update_write_stream(2, CONSTANT_FORCE_MN, sine_force_signal.get_value());
            if (!was_connected[1]) {
                motor[1].set_spring_effect(0, 10000, 60000, 40);
                motor[1].set_mode(Actuator::HapticMode);
                motor[1].set_stream_mode(Actuator::MotorWrite);
                motor[1].update_write_stream(2, CONSTANT_FORCE_MN, 0);
                motor[1].enable_haptic_effects(Actuator::Spring0 + Actuator::ConstF);
            }
        }
        else was_connected[1] = false;


        for (int i = 0; i < NUM_MOTORS; i++) {
            //parse incomming messages and manage high speed stream
            motor[i].run_in();
            motor[i].run_out();
        }
        //Sleep(1); //loop runing too fast??
    }
}



