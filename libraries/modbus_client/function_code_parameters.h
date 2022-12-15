/**
 * @file function_code_parameters.h
 * @author Kali Erickson <kerickson@irisdynamics.com>
 * 
 * @brief Various parameters related to max data values for specific function codes and expected response lengths
 * 
 * @version 2.2.0
    
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

#ifndef FUNCTION_CODE_PARAMETERS_H_
#define FUNCTION_CODE_PARAMATERS_H_

// Response Lengths -  total response lengths include address, function code, data, and crc bytes.
#define WRITE_OR_GET_COUNTER_RESPONSE_LEN 8
#define READ_EXCEPTION_STATUS_LEN 5
#define NO_ECHO_DIAGNOSTIC_LEN 8

//function code 0x01 - read coils
#define MAX_NUM_READ_COILS 0x7D0

//function code 0x02 - read discrete inputs
#define MAX_NUM_DISCRETE_INPUTS 0x7D0

//function codes 0x03, 0x04, 0x17
#define MAX_NUM_READ_REG 0x7D

//function code 0x05 - write single coil
#define WRITE_COIL_ON   0xFF00
#define WRITE_COIL_OFF  0x0000

//function code 0x06 - write single register
#define MAX_WRITE_VALUE 0xFFFF

//function code 0x0F - write multiple coils
#define MAX_NUM_WRITE_COILS 0x07B0  //??

//function code 0x10 - write multiple registers
#define MAX_NUM_WRITE_REG 0x007B

//function_code 0x17
#define MAX_NUM_WRITE_REG_RW 0x0079

#endif