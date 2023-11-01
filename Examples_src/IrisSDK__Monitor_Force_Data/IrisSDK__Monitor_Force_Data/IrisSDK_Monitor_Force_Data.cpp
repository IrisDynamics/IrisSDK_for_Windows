#include "library_linker.h"
#include "modbus_client/device_applications/actuator.h"
#include <iostream>

Actuator motor{90, "Orca Motor", 1};	//replace port number with your RS422 cable to the orca series motor's com port number

//!< custom communication settings for baud rate and interframe delay to allow faster communication than modbus protocol
Actuator::ConnectionConfig connection_config;
bool was_connected = false;     //has high speed communication been established
int num_data_for_average = 0;
int force_average_buffer[200];
int force_average = 0;
int cycle_count = 0;
bool motion_triggered = false;
uint32_t start_time = 0;
uint32_t data_gather_period = 20; //gather force data every 20 milliseconds 

void first_connection() {
    //set up a kinematic profile that will chain three motions and then stop until triggered.
    motor.set_kinematic_motion(0, 20000, 300, 0, 0, 1, 1);      
    motor.set_kinematic_motion(1, 70000, 600, 0, 0, 1, 2);
    motor.set_kinematic_motion(2, 40000, 400, 0, 0, 0, 0);
    motor.set_stream_mode(Actuator::MotorRead);
    motor.write_register(CTRL_REG_3, Actuator::KinematicMode);  //put the motor into kinematic mode which will also trigger the first motion.
}

void check_for_complete_cycle() {
    motor.update_read_stream(1, KINEMATIC_STATUS);  //read stream kinematic status, this will allow us to keep track of the state of the kinematic motion while also getting motor data
    if (motor.get_orca_reg_content(KINEMATIC_STATUS) & 0x8000) {    //this bit will be true while the motion is active
        if ((uint32_t)(GetTickCount() - start_time) > data_gather_period) {
            start_time = GetTickCount();
            force_average_buffer[num_data_for_average] = motor.get_force_mN();
            num_data_for_average++;
        }
        motion_triggered = false;
    }
    else if (!motion_triggered){
        motor.write_register(KIN_SW_TRIGGER, 0);   //this will trigger the next cycle
        for (int i = 0; i < num_data_for_average; i++) {
            force_average += force_average_buffer[i];
        }
        if (num_data_for_average > 0) {
            force_average = force_average / num_data_for_average;
        }
        std::cout << "cycle_count: " << cycle_count << " Force Avg (N) " << force_average/1000. << std::endl;
        cycle_count++;
        num_data_for_average = 0;
        motion_triggered = true;
        start_time = GetTickCount();
    }

}


int main() {
    /// This section is optional, if not added will default to 625000 bps and 80us
    //connection_config.target_baud_rate_bps = 1250000;// 500000;  //625000 //780000
    //connection_config.target_delay_us = 0;

    motor.set_connection_config(connection_config);     //set the values for high speed streaming
    //motor.set_new_comport(port_number[i]); //this line can be used to update the com port after the program has been compile if you want to update it from the consoles
    motor.init();       //initialize the modbus communication serial port
    motor.enable();     //enable high speed communication starting with a handshake

    while (1) {
        if (motor.is_connected()) {
            if (!was_connected) {  //on first connection 
                first_connection();
            }
            check_for_complete_cycle();
            was_connected = true;
        }
        else {
            was_connected = false;
        }

        //send and receive to the motor
        motor.run_in();
        motor.run_out();
    }
}