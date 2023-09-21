#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

class Log {
public:
	~Log() {
		close();
	}

	/**
	 *	@brief	Returns true if the log has a currently opened file
	 */
	int is_open() { return file.is_open(); }

	/**
	 *	@brief	Writes the contents of the str parameter to the file. Also writes a timestamp if 
	 *			verbose mode is on.
	 *	@param	std::string str  The log message to be written
	 *	@throws	std::runtime_error	If this method is called on a log without an opened file
	 */
	void write(std::string str) {
		if (!file.is_open()) throw std::runtime_error("Tried to write to unopened log file");
		if (verbose_mode) str = get_timestamp() + str;
		file << str << "\r\n" << std::flush;
	}

	/**
	 *	@brief	Opens a file located at the given path, creating one if it doesn't exist, and appending 
	 *			to the file if it does exist.
	 *	@param	std::string path  The file location of the desired log file. Begins relative paths at 
								  the calling .exe
	 *	@param	std::string file_ext  The file extension of the desired file, defaults to .txt
	 *	@throws	std::runtime_error	If this method fails to open the file, or if a file is already open
	 *	@notes	If verbose mode is on, writes a message signaling a successful open
	 */
	void open(std::string path, std::string file_ext = ".txt") {
		std::string full_name = path + file_ext;

		if (is_open()) throw std::runtime_error("Could not open file: " + full_name + ". The file: " + file_name + " is already open.");

		file_name = full_name;

		file.open(full_name, std::ios::out | std::ios::app | std::ios::binary);
		if (!is_open()) throw std::runtime_error("Failed to open log file: " + path);
		if (verbose_mode) write("Opened File");
	}

	/**
	 *	@brief	Closes the current file, if it is open
	 *	@notes	If verbose mode is on, writes a message signaling closing the file
	 */
	void close() {
		if (is_open() && verbose_mode) write("Closed File");
		file_name = "";
		file.close();
	}

	/**
	 *	@brief	Turns verbose mode on or off. If verbose mode is on, then, in addition to user defined
	 *			writes, the log will:
	 *				1. Include a timestamp with every log write
	 *				2. Write a message upon opening the file
	 *				3. Write a message upon closing the file
	 */
	void set_verbose_mode(bool active) {
		verbose_mode = active;
	}

	// Included for interoperability between log implementations
	void add(const char* name) {
		open(name);
	}
	u32 id() { return -1; }

private:
	bool verbose_mode = true;

	std::string file_name = "";

	std::ofstream file;

	// Referenced from awesoon's answer here: https://stackoverflow.com/questions/16357999/current-date-and-time-as-string
	std::string get_timestamp() {
		time_t now = time(0);

		tm localtm;
		localtime_s(&localtm, &now);

		std::stringstream ss;
		ss << std::put_time(&localtm, "%a %d %b %Y %H:%M:%S: ");

		return ss.str();
	}
};
