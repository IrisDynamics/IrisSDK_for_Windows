/**
 * @file mb_config.h
    
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

#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_

//If you are not using the K20, KV31, or Zynq7000 platforms, uncomment ONE of the defines below:
//#define ATMEGA328
//#define WINDOWS
//#define QT_WINDOWS
//#define ATTINY1617


#if defined(CPU_MKV31F256VLH12) || defined(__MK20DX256__) || defined(ATMEGA328) || defined(IRIS_ZYNQ_7000) || defined(WINDOWS) || defined(QT_WINDOWS) || defined(ATTINY1617)
#else 
#error Must uncomment one of the platform types in mb_config.h
#endif

/// Modbus virtual register adresses - Static - do not adjust ///
#define MB_BAUD_HIGH        65532
#define MB_BAUD_LOW         65533
#define MB_DELAY            65534
#define MB_SERVER_ID        65535

//

#define DEFAULT_INTERFRAME_uS	2000	//2000 
#ifdef WINDOWS
#define DEFAULT_INTERCHAR_uS	16000//8000 	//8000	// 700
#else
#define DEFAULT_INTERCHAR_uS	8000
#endif
#define DEFAULT_TURNAROUND_uS	500		//2000	//2000
#define DEFAULT_RESPONSE_uS    	50000	//750000 - windows		//100000

#define DEFAULT_CONNECTION_PAUSE_uS		750000



//uncomment one of the following buffer size options - MUST CONFIGURE
#if defined(__MK20DX256__)
#define NUM_MESSAGES        16// 8  //4  //32  //64
#elif defined(ATTINY1617)
#define NUM_MESSAGES        4//2
#elif defined(ATMEGA328)
#define NUM_MESSAGES        4//2
#else
#define NUM_MESSAGES        64  //8  //4  //32  //64
#endif
//uncomment one of the following baud rate options
#define UART_BAUD_RATE      19200  //9600  //1000000  //625000  //500000   //Modbus specified default is 19200bps


#if defined(ATMEGA328)
#include <avr/pgmspace.h>
#define LESS_DATA_MEM   PROGMEM
#define ACCESS_PROGMEM(x)   pgm_read_word(&x)
#else
#define LESS_DATA_MEM
#define ACCESS_PROGMEM(x)   x
#endif

#endif
