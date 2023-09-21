/*!
 * @file IC4_windows.h
 * @author  Rebecca McWilliam <rmcwilliam@irisdynamics.com>
 * @version 1.0, Sept 2022
 * @brief IrisControls4 device driver for the windows device emulator
 *
 * Copyright (C) Iris Dynamics Ltd - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *
 * This object extends the IrisControls virtual base object and implements;
 * 1. USB messaging via virtual usb ports
 *
 * This object does not add a serial parser, but an application may extend this object and override the parser to add custom serial parsing.
 */
#ifndef _IC4_WINDOWS_H_
#define _IC4_WINDOWS_H_

#include "../../iriscontrols4.h"
#include<windows.h>
#include<stdio.h>
#include <vector>
#include <sstream>


class IC4_windows : public IrisControls4 {


public:
	typedef enum {
		tx,
		rx
	} states;
	volatile states gui_frame_state = rx;

	HANDLE hComm;
	DCB dcbSerialParams = { 0 };
	std::vector <char> sendBuf;
	DWORD      dwRes;
	DWORD      dwCommEvent;
	DWORD      dwStoredFlags;
	OVERLAPPED o;
	DWORD dwEventMask = 0;

	IC4_windows() {
		set_full_duplex();
	}

	void setup(int comport) {

		std::ostringstream portOsStr;
		portOsStr << "\\\\.\\COM" << comport;
		std::string portStr(portOsStr.str());
		LPCSTR portName = portStr.c_str();

		hComm = CreateFileA(portName,                //port name
			GENERIC_READ | GENERIC_WRITE, //Read/Write
			0,                            // No Sharing
			NULL,                         // No Security
			OPEN_EXISTING,// Open existing port only
			0,            // Non Overlapped I/O
			NULL);        // Null for Comm Devices

		if (hComm == INVALID_HANDLE_VALUE) {
			printf("Error in opening serial port");
		}
		else {
			setup_sucess = true;
			printf("\nConnection on serial port successful");
		}




		//get current state
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(hComm, &dcbSerialParams)) {
			LPCWSTR getErr = L"Error getting dcb state\n";
			OutputDebugString(getErr);
		}

		//set new state
		dcbSerialParams.BaudRate = 220713;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = EVENPARITY;
		// dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
		if (!SetCommState(hComm, &dcbSerialParams)) {
			LPCWSTR paramErr = L"Error setting serial port state\n";
			OutputDebugString(paramErr);
		}
	}

	/**
	* @brief Return the system time in microseconds.
	*/
	uint64_t system_time() override {
		LARGE_INTEGER ticks;
		if (!QueryPerformanceCounter(&ticks)) {
			LPCWSTR tickErr = L"Error getting ticks for checking system time";
			OutputDebugString(tickErr);
		}
		return ticks.QuadPart / 10;
	}

	void send() {
		DWORD dwBytesWritten = 0;
		while (transmit_buffer.size()) {
			//Serial.write(transmit_buffer.popchar());
			char charBuf[10] = { 0 };
			charBuf[0] = transmit_buffer.popchar();
			WriteFile(hComm, charBuf, 1, &dwBytesWritten, &o);
		}
		//flush butters and clear the sendbuf for next message 
		FlushFileBuffers(hComm);
		if (eot_queued) {
			eot_queued = 0;
			gui_frame_state = rx;
		}
	}

	void receive() {
		LPDWORD lpErrors = 0;
		COMSTAT lpStat{ 0 };
		if (!ClearCommError(hComm, lpErrors, &lpStat)) {
			LPCWSTR clearErr = L"Issue checking com errors\n";
			OutputDebugString(clearErr);
		}
		char buff = 0;
		int toRead = 1;
		DWORD bytesRead = 0;
		int n = lpStat.cbInQue;

		for (int i = 0; i < n; i++) {
			if (!ReadFile(hComm, &buff, toRead, &bytesRead, &o)) {
				if (GetLastError() != ERROR_IO_PENDING) {
					//ERROR_IO_PENDING - means the IO request was succesfully queued and will return later 
					LPCWSTR readErr = L"Error recieving bytes\n";
					OutputDebugString(readErr);
					OutputDebugString((LPCWSTR)GetLastError());
				}
			}
			receive_char(buff);
			//LPCWSTR paramErr = L"receive_char\n";
		   
		}
	}

	virtual int random_number() {
		return 0;
	}	

	void handle_eot() {
		gui_frame_state = tx;
	}

	/**
	* @brief Returns a string formatted int.
	* @param[in] int d - The int to print.
	* @param[out] const char * - The formatted input value.
	* Used to print integers to the console in Iris Controls.
	*/
	const char* val_to_str(int d) override {
		int i = 0;
		while (std::to_string(d)[i] != '\0') {
			var_to_str_array[i] = std::to_string(d)[i];
			i++;
		}

		return (var_to_str_array);
	}

	/**
	* @brief Returns a string formatted int.
	* @param[in] unsigned int d - The int to print.
	* @param[out] const char * - The formatted input value.
	* Used to print integers to the console in Iris Controls.
	*/
	const char* val_to_str(unsigned int u) override {

		int i = 0;
		while (std::to_string(u)[i] != '\0') {
			var_to_str_array[i] = std::to_string(u)[i];
			i++;
		}

		return (var_to_str_array);		
	}

	/**
	* @brief Returns a string formatted int.
	* @param[in] u64 d - The uint64_t to print.
	* @param[out] const char * - The formatted input value.
	* Used to print integers to the console in Iris Controls.
	*/
	const char* val_to_str(u64 u) override {

		int i = 0;
		while (std::to_string(u)[i] != '\0') {
			var_to_str_array[i] = std::to_string(u)[i];
			i++;
		}	
				
		return (var_to_str_array);
	}

	/**
	* @brief Returns a string formatted uint64_t.
	* @param[in] float f - The float to print.
	* @param[out] const char * - The formatted input value.
	* Used to print floats to the console in Iris Controls
	*/
	const char* val_to_str(float f) override {
		int i = 0;
		while (std::to_string(f)[i] != '\0') {
			var_to_str_array[i] = std::to_string(f)[i];
			i++;
		}

		return (var_to_str_array);
	}




	/**
	* @fn int parse_device_driver(char* cmd)
	* @brief Attempts to parse commands that were not successfully parsed by higher level parsers.
	* @param[in] char* cmd - The command to be parsed
	* @param[out] int - 1 if parsed successfully, 0 otherwise
	*/
	int parse_device_driver(char* cmd) override {

		std::string command_list = "\Windows: \r\r system_time\r";

		START_PARSING

		COMMAND_IS "system_time" THEN_DO

			print_l("System Time: \r");
			print_d(system_time());
			print_l(" *mu*s\r");			

		COMMAND_IS "help" THEN_DO

			print_l(command_list.c_str());

		FINISH_PARSING
	}

};


#endif
