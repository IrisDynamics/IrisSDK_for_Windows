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


/**
 * @brief a ring buffer, having a size that is a power of two (the template argument is that power of two).
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
	 * @brief add a single character to the buffer
	 */
	void printchar( char s ) {

		d[end_index & mask] = s;
		end_index++;
	}

	/**
	* @brief pull a single character off the buffer 
	* 
	* This function assumes size() is greater than zero. Unexpected results occur when called while size is zero. 
	*/
	char popchar() {

		char ret = d[start_index & mask];
		start_index++;
		return ret; 
	}

	void clear() {
		start_index = 0;
		end_index 	= 0;
	}

};


