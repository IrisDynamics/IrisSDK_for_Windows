/**
 * @file circularbuffer.h
 * @author  Kyle Hagen <khagen@irisdynamics.com> Rebecca McWilliam <rmcwilliam@irisdynamics.com>
 * @version 2.2.2
    
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
#pragma once

#include "config.h"		// note: this must be implemented by the platform
#include "API_Config.h"


/**
* @class CircularBuffer
* @brief A ring buffer, having a size that is a power of two (the template argument is that power of two).
*
*/
template< int buffer_bits >
class CircularBuffer {
public:

	char d		[(1<<buffer_bits)] = {0};		///< character buffer
	const u32 max_size =  1<<buffer_bits;			///< buffer size must be a power of 2
	const u32 mask =  max_size -1;		///< used to mask the indexes instead of checking bounds  when incrementing

	u32 start_index = 0, 					///< incrementing index to the first character in the buffer
		end_index	= 0;						///< following index to the last character.

//	bool overflow = 0; 			///< flag indicating that at some point, the buffer has overflowed

	u32 size() {
		return end_index - start_index;
	}


	/**
	* @brief Add a single character to the buffer.
	* @param[in] char s - The char to add.
	*/
	void printchar(char s) {

		d[end_index & mask] = s;
		end_index++;
	}

	/**
	* @brief Pull a single character off the buffer.
	* @return char - The char from the buffer.
	* 
	* This function assumes size() is greater than zero. Unexpected results occur when called while size is zero. 
	*/
	char popchar() {

		char ret = d[start_index & mask];
		start_index++;
		return ret; 
	}

	/**
	* @brief Resets the start and end index of the circular buffer, clearing it.	
	*/
	void clear() {
		start_index = 0;
		end_index 	= 0;
	}

	/**
	* @brief Makes sure that any junk bytes are cleared from the buffer.
	* @return int - The number of bytes that had to be popped off.
	* 
	* @note
	* Called after an attempt to parse a console message. To ensure that the console message
	* parsing was complete and there are no stray chars left on the buffer before the next message.
	*/
	int recover() {

		int bytes_to_pop = bytes_to_trailer();

		for (int i = 0; i < bytes_to_pop; i++) popchar(); //pop any remaining payload

		popchar(); //pop TRAILER
		popchar(); //pop FLAG
							
		return bytes_to_pop;
	}

	/**
	* @brief Checks how many bytes remain in the buffer before the next trailer.
	* @return int - The number of bytes.
	* 
	* @note
	* Used by parse_int and parse_double to make sure enough data remains on the RX buffer
	* when parsing arguments out of console commands.
	*
	* If there is no trailer, will return the size of the buffer.
	*/
	int bytes_to_trailer() {

		u32 start = start_index;

		while (start != end_index) {
			if  (	d[start			& mask]	== TRAILER 
				&&	d[(start + 1)	& mask]	== FLAG
				)
			{ break; }
			start++;
		}

		return start - start_index;
	}

};


