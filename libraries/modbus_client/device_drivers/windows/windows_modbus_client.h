/**
 * @file windows_modbus_client.h
 * @author Kate Colwell <kcolwell@irisdynamics.com>
 *
 * @brief  Virtual device driver for Modbus client serial communication using the Windows API
 *
 * This class extends the virtual ModbusClient base class
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

#pragma once 

#include<iostream>
#include <windows.h>
#include <timeapi.h>
#include <vector>
#include <sstream>
#include "../../modbus_client.h"
#include "../../transaction.h"

 /**
  * @class windows_ModbusClient
  * @brief Extension of the ModbusClient virtual class that implements functions for the Eagle K20's timers, and interrupts. UART channel specific functions to be
 */

class windows_ModbusClient : public ModbusClient {

protected:

    //int channel_number;
    uint32_t cycles_per_us;

    bool disconnected_msg_sent = false;
    bool motor_disconnected = false;
    bool comms_enabled = true;

public:
    int channel_number;

    int sucCounter = 0;
    int failCounter = 0;
    int callBCounter = 0;
    int isrCounter = 0;
    windows_ModbusClient* wmc;
    bool cont = true;
    bool canEnd = true;

    //comport 
    HANDLE hSerial;     //handle to the serial port 
    DCB dcbSerialParams = { 0 };
    bool serial_success = false;     //flag bool to indicate if the handle to the serial port was created successfully

    LPHANDLE dupe;
    DCB test_dcb;

    //thread stuff 
    HANDLE threadHandle;
    LPDWORD threadID;

    //for messaging
    std::vector <char> sendBuf;
    DWORD      dwRes;
    DWORD      dwCommEvent;
    DWORD      dwStoredFlags;
    OVERLAPPED o;
    DWORD dwEventMask = 0;
        
    windows_ModbusClient(int _channel_number, uint32_t _cycles_per_us) : ModbusClient(_channel_number, _cycles_per_us)
    {
        channel_number = _channel_number;
        cycles_per_us = _cycles_per_us;
    }


    uint8_t get_port_number() {
        return channel_number;
    }


    /** @brief Returns true if a com port was previously opened and was not successfully closed */
    bool connection_state() {
        bool in_connection_state = false;

        if (serial_success) {
            in_connection_state = true;
        }

        return in_connection_state;
    }

    void start_new_listening_thread() {
        threadID = 0;
        threadHandle = CreateThread(NULL, 0, ListeningThread, wmc, 0, threadID);
        if (threadHandle == NULL) {
            OutputDebugString((LPCWSTR) L"Unable to create listening thread\r\n");
        }
    }


    /**
    * @brief Monitors the serial port for incoming bytes
    * @note this method runs in a separate thread and continuously checks if a new byte has arrived in the serial port.
    */
    static DWORD WINAPI ListeningThread(LPVOID lpParam) {
        while (1) {
            // infinite while loop for modbus client IO
            windows_ModbusClient* w = (windows_ModbusClient*)(lpParam);
            check_coms(w);
        }
    }


    // @brief Function checks if a new byte has arrived in the serial port.
    static int check_coms(windows_ModbusClient* w) {
        DWORD dwCommEvent;
        if (WaitCommEvent(w->hSerial, &dwCommEvent, NULL)) {
            if (dwCommEvent == EV_RXCHAR) {
                w->uart_isr();
            }
            else if (dwCommEvent == 0) {
                OutputDebugString((LPCWSTR) L"Error in comm event\r\n");
            }
        }
        else {
            LPCWSTR eventErr = L"Error checking for incoming bytes\n";
            OutputDebugString(eventErr);
            int error = GetLastError(); 
        }
        return 0;
    }



    void set_new_comport(int comport) {
        channel_number = comport;
    }


    void disable_comport_comms() {
        run_out();
        cont = false;
        //adjust_baud_rate(19200);
        if (!threadHandle == NULL) {
            if (SuspendThread(threadHandle) == -1) {
                OutputDebugString((LPCWSTR) L"Unable to suspend thread");
            }
        }
        if (!PurgeComm(hSerial, PURGE_TXABORT)) {
            OutputDebugString((LPCWSTR)L"Unable to purge tx com\r\n");
        }
        if (!PurgeComm(hSerial, PURGE_RXABORT)) {
            OutputDebugString((LPCWSTR)L"Unable to purge rx com\r\n");
        }
        if (!FlushFileBuffers(hSerial)) {
            OutputDebugString((LPCWSTR)L"Unable to flush file buffer\r\n");
        }
        if (!CloseHandle(hSerial)) {
            OutputDebugString((LPCWSTR)L"Unable to close hSerial handle\r\n");
        }
        //reset_state();
        //messages.reset();
        serial_success = false;
        comms_enabled = false;
    }


    bool port_available(int port_num) {

        cont = true;

        //return value
        bool port_available = false;

        //get port number from channel parameters
        std::ostringstream portOsStr;
        portOsStr << "\\\\.\\COM" << port_num;
        std::string portStr(portOsStr.str());
        LPCSTR portName = portStr.c_str();

        //open handle to comport
        HANDLE hSerialCheck = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE | OPEN_ALWAYS, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);  //FILE_FLAG_OVERLAPPED //FILE_ATTRIBUTE_NORMAL
        if (hSerialCheck == INVALID_HANDLE_VALUE) {
            LPCWSTR portErr = L"Error opening comport\n";
            OutputDebugString(portErr);
            DWORD err = GetLastError();
            //OutputDebugStringA((LPCSTR)err);
            serial_success = false;
        }
        else {
            hSerial = hSerialCheck;
            serial_success = true;
            port_available = true;
        }

        return port_available;

    }

    /**
     * @brief Intializes the com port settings 
     * @param baud The baud rate as defined in the client_config.h file
    */
    void init(int baud) override {

        //check to see if we are able to open a comport on this port - if not, exit out 
        if (!port_available(channel_number)) {
            //not able to connect on this port, exit out 
            serial_success = false;
            return;
        }
        else {
            serial_success = true;
        }

        cont = true;

        //set comport parameters - this is where init used to be called. 
        //get current state
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            LPCWSTR getErr = L"Error getting current port state\n";
            OutputDebugString(getErr);
        }
        //set new state
        dcbSerialParams.BaudRate = baud;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = EVENPARITY;
        // dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
        if (!SetCommState(hSerial, &dcbSerialParams)) {
            LPCWSTR paramErr = L"Error setting serial port state\n";
            OutputDebugString(paramErr);
        }

        //set comm Mask - will notify when a byte arrives in the port 
        if (!SetCommMask(hSerial, EV_RXCHAR)) {
            LPCWSTR maskErr = L"Error setting port com mask\n";
            OutputDebugString(maskErr);
        }

        //initialize hEvent in the overlapped structure
        o.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (o.hEvent == NULL) {
            LPCWSTR eventErr = L"Error setting overlapped event\n";
            OutputDebugString(eventErr);
        }

        //intialize the rest of the overlapped structure to 0
        o.Internal = 0;
        o.InternalHigh = 0;
        o.Offset = 0;
        o.OffsetHigh = 0;

        //create a commstimeout structure 
        COMMTIMEOUTS timeouts{
            MAXDWORD,
            0,
            0,
            0,
            0
        };

        //send to setCommTimeouts 
        if (!SetCommTimeouts(hSerial, &timeouts)) {
            LPCWSTR cmtmtsError = L"Error setting comm timeouts\n";
            OutputDebugString(cmtmtsError);
        }


        wmc = this;

        start_new_listening_thread();

        //
        disconnected_msg_sent = false;
        motor_disconnected = false;


        //set everything to a clear state 
        reset_state();
        sendBuf.clear();
        
    }

    //////////// Virtual function implementations ////////////

    /**
     * @brief Get the device's current system time
     * @return the current system time in microseconds
    */
    uint64_t get_system_time_us() {
        LARGE_INTEGER ticks;
        if (!QueryPerformanceCounter(&ticks)) {
            LPCWSTR tickErr = L"Error getting ticks for checking system time";
            OutputDebugString(tickErr);
        }
        return ticks.QuadPart / 10;
        //return ticks.QuadPart;
    }

public:

    /**
     * @brief If there are bytes left to send, add them to a buffer, then send them all together.
    */
    //need messages to be switched to protected, not private 
    void tx_enable() override {
        if (!serial_success) return;
        char charBuf[128] = { 0 };
        DWORD dwBytesWritten = 0;

        //while there are bytes left to send in the transaction, continue adding them to sendBuf
        while (messages.get_active_transaction()->bytes_left_to_send()) {
            if (my_state == emission) {
                send();
            }
        }

        //copy the contents of sendBuf over to charBuf - type issue
        for (int i = 0; i < sendBuf.size(); i++) {
            charBuf[i] = sendBuf[i];
        }

        //write the contents of charBuf to the serial port
        if (!WriteFile(hSerial, charBuf, sendBuf.size(), &dwBytesWritten, &o)) {
            if (GetLastError() != ERROR_IO_PENDING) {

                //ERROR_IO_PENDING - means the IO request was succesfully queued and will return later 
                LPCWSTR writeErr = L"Error sending bytes\n";
                OutputDebugString(writeErr);

                __try {
                    OutputDebugString((LPCWSTR)GetLastError());
                }
                __except (filter(GetExceptionCode(), GetExceptionInformation())) {
                    if (!disconnected_msg_sent) {
                        OutputDebugString((LPCWSTR) L"Motor has been disconnected\r\n");
                        disconnected_msg_sent = true;
                        motor_disconnected = true;
                    }
                }
                
 
            }
        }


        __try{
            FlushFileBuffers(hSerial);
        }
        __except (filter(GetExceptionCode(), GetExceptionInformation())) {
            if (!disconnected_msg_sent) {
                OutputDebugString((LPCWSTR)L"Motor has been disconnected\r\n");
                disconnected_msg_sent = true;
                motor_disconnected = true;
            }
        }

        if (motor_disconnected) {
            if (comms_enabled) disable_comport_comms();
            //int port_to_check = get_port_number();

            //if (port_available(port_to_check)) {
            //    set_new_comport(port_to_check);
            //}
        }
     
        sendBuf.clear();
    }



    /* @Brief  */
    int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
    {
        //if (!disconnected_msg_sent) puts("in access violation filter");
        if (code == EXCEPTION_ACCESS_VIOLATION)
        {
            //if (!disconnected_msg_sent) puts("caught access violation as expected.");
            return EXCEPTION_EXECUTE_HANDLER;
        }
        else
        {
            //if (!disconnected_msg_sent) puts("didn't catch access violation as expected.");
            return EXCEPTION_CONTINUE_SEARCH;
        };
    }





    /**
     * @brief Not using interupts, so no implementation needed.
    */
    void tx_disable() override {

    }

    /**
     * @brief Loads the send buffer with the next byte
     * @param byte		The byte to be transmitted.
     */
    void send_byte(uint8_t data) override {
        sendBuf.push_back(data);
    }

    /**
     * @brief Return the next byte received by the serial port.
     */
    uint8_t receive_byte() override {

        char buff = 0;
        int toRead = 1;
        DWORD bytesRead = 0;

        if (!ReadFile(hSerial, &buff, toRead, &bytesRead, &o)) { 
            if (GetLastError() != ERROR_IO_PENDING) {
                //ERROR_IO_PENDING - means the IO request was succesfully queued and will return later 
                LPCWSTR readErr = L"Error recieving bytes\n";
                OutputDebugString(readErr);
                OutputDebugString((LPCWSTR)GetLastError());
            }
            //else {
            //   // ERROR_IO_PENDING indicated - the readfile has not failed, but has not completed yet
            //    while (readPending) {
            //        bool complete = GetOverlappedResult(hSerial, &o, (LPDWORD)toRead, FALSE);
            //        if (!complete) {
            //            //read is not yet complete
            //            switch (dwError = GetLastError()) {
            //            case ERROR_IO_INCOMPLETE: {
            //                //operation is still pending, run through the loop again
            //                readPending = true;
            //                break;
            //            }
            //            default:
            //                //check for any other errors 
            //                LPCWSTR pendingErr = L"Error recieving byte - IO pending\n";
            //                OutputDebugString(LPCWSTR(pendingErr));
            //                OutputDebugString((LPCWSTR)GetLastError());
            //            }
            //        }
            //    }
            //    ResetEvent(o.hEvent);
            //}
        }

        return buff;
    }

    /**
     * @brief Adjust the baud rate
     * @param baud_rate the new baud rate in bps
     * this method overrides the modbus default delay
    */
    void adjust_baud_rate(uint32_t baud_rate_bps) override {

        //LPHANDLE dupe;
       // if (DuplicateHandle(GetCurrentProcess(), hSerial, GetCurrentProcess(), dupe, NULL, FALSE, 0)) {

            if (!GetCommState(hSerial, &dcbSerialParams)) {

            }

            dcbSerialParams.BaudRate = baud_rate_bps;

            if (!SetCommState(hSerial, &dcbSerialParams)) {

            }
      
    }

    /**
    * @brief Get the device's current system time in cycles - not using cycles, so just returns elapsed time in microseconds
    */
    uint32_t get_system_cycles() override {
        LARGE_INTEGER ticks;
        if (!QueryPerformanceCounter(&ticks)) {
            LPCWSTR tickErr = L"Error getting ticks for checking system time";
            OutputDebugString(tickErr);
        }
        return ticks.QuadPart / 10;
    };


    /**
   * @brief Called whenever there is new data to recieve in the serial port.
   */
    void uart_isr() override {
        if (my_state == reception) {
            while (byte_ready_to_receive()) {

                receive();
                //    //slot called when new data arrives in the port 
                //    //as long as the state is reception and there is data to recieve, it will be recieved. 
                //    //receive method handles slotting data into the correct transaction.

            }

        }
    }

    /**
    * @brief checks the comport to determine if at least one byte is ready to be read
    */
    bool byte_ready_to_receive()override {

        LPDWORD lpErrors = 0;
        COMSTAT lpStat { 0 };
        if (!ClearCommError(hSerial, lpErrors, &lpStat)) {
            LPCWSTR clearErr = L"Issue checking com errors - needed to check for number of incoming bytes\n";
            OutputDebugString(clearErr);
            return false;
        }
        return (lpStat.cbInQue > 0);
    };


    /**
    * @brief ends the listening thread, purges the comport and closes it 
    */
    ~windows_ModbusClient() {

        //delete timers in the timer queue 
        //have to signal the timer here to stop the timer 

        cont = false;

        CloseHandle(threadHandle);

        FlushFileBuffers(hSerial);
        PurgeComm(hSerial, PURGE_TXABORT);
        PurgeComm(hSerial, PURGE_RXABORT);
        CloseHandle(hSerial);

    }
};

extern windows_ModbusClient modbus_client;


