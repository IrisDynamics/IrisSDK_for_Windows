/**
 * @file config.h
 * @author  Dan Beddoes <dbeddoes@irisdynamics.com>
 * @version 2.2.0
 *	Contains pre-processor logic used to define:
 *  - The device in use
 *  - The size of the software buffers
 *  - The type of serial message formatting in use
    
    Copyright 2022 Iris Dynamics Ltd 
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

#define WINDOWS

#ifdef IRIS_ZYNQ_7000
#	include <xil_types.h>
#define TX_BUFFER_BITS		27
#define RX_BUFFER_BITS		13
#define FULL_DUPLEX
#elif defined(CPU_MKV31F256VLH12)
#	include "device_drivers/kv31/kv31_types.h"
#define TX_BUFFER_BITS		12
#define RX_BUFFER_BITS		9
#define IC4_BAUDRATE		480600
#define HALF_DUPLEX
#elif defined(__MK20DX256__)
#	include "device_drivers/k20/k20_types.h"
#define TX_BUFFER_BITS		13	//note to k8: something to try is making this smaller, might speed up the crash 
#define RX_BUFFER_BITS		13
#define FULL_DUPLEX
#elif defined(WINDOWS)
#include "device_drivers/windows/windows_types.h"
#define TX_BUFFER_BITS		11
#define RX_BUFFER_BITS		9
#define IC4_BAUDRATE		230400
#define FULL_DUPLEX
#else
#error Platform define missing from project.
#endif

/**
 * @brief Pre-processor logic that defines the way outgoing serial messages are formatted
 * 
 * One of the following defines must be uncommented.
 * 
 * Byte Stuffing Parsing: 
 * Ensures that a FLAG char occurring in a message payload cannot be mistaken
 * for a message flag. This is accomplished by stuffing an ESC char into the message before the FLAG
 * char. When the message is parsed in the application, it throws away any ESC char it encounters
 * but then always parses the next char as part of the payload. Since this means that the parser throws out
 * ESC chars, an ESC occurring in the payload must also have an ESC char stuffed in front of it.
 * Using this method the application's parser can be certain whether or not it has received a complete 
 * message and there is no ambiguity about whether special chars are part of the payload or not.
 * https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 * 
 * Message Length Parsing:
 * The application stores a table of message lengths (in bytes) for known length messages. Every message sent
 * contains an extra 16 bits corresponding to the size of any unknown part of the message (e.g. a string). The application's
 * message parser will not parse a command's arguments unless enough data has been received to meet or exceed that
 * message's expected length.
 */
#define BYTE_STUFFING_PARSING
//#define MESSAGE_LENGTH_PARSING

#if !defined(BYTE_STUFFING_PARSING) && !defined(MESSAGE_LENGTH_PARSING)
#error Must uncomment either byte stuffing or no stuffing in config.h
#endif

#if defined(BYTE_STUFFING_PARSING) && defined(MESSAGE_LENGTH_PARSING)
#error Must only uncomment one of BYTE_STUFFING and NO_STUFFING
#endif

