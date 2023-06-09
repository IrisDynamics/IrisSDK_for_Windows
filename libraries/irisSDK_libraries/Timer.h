/**
 * @file  Timer.h
 * @author  Aiden Bull <abull@irisdynamics.com>
 * @brief Contains a simple timer class.

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

#include "device_config.h"

/**
 *	@class	Timer Timer.h "irisSDK_libraries/Timer.h" 
 *	@brief	An object intended to be used as a countdown timer (But does support counting up). Offers a selection of simple operations. 
 */
class Timer {
	uint32_t start_time = 0;
	uint32_t duration = 0;

public:
	Timer() {
		start_time = millis();
	}

	/**
	 *	@brief		Sets the timer's duration to the value indicated by the parameter and restarts the timer. 
	 *	@param[in]  uint32_t _duration - The duration of the timer in milliseconds. 
	 */
	void set(uint32_t _duration) {
		start_time = millis();
		duration = _duration;
	}

	/**
	 *	@brief		Restarts the timer, but does not modify duration. 
	 */
	void reset() {
		start_time = millis();
	}

	/**
	 *	@brief		Returns true if the timer has expired and false otherwise. 
	 *	@return		True if the timer has expired. 
	 * 
	 *	@note		bool - If the timer has not yet been set, it is treated as if it has expired. 
	 */
	bool has_expired() {
		return (start_time + duration) <= millis();
	}

	/**
	 *	@brief		Returns the amount of time left until the timer expires. 
	 *	@return		uint32_t - Time in milliseconds until the timer will expire. 
	 * 
	 *	@note
	 *	If the timer has expired, will return 0. 
	 */
	uint32_t time_remaining() {
		if (has_expired()) return 0;
		return (start_time + duration) - millis();
	}

	/**
	 *	@brief		Returns the amount of time that the timer has been running for. 
	 *	@return		uint32_t - Time in milliseconds since the timer was last set. 
	 * 
	 *	@note
	 *	If the timer has not yet been set, returns time since object construction. 
	 */
	uint32_t time_elapsed() {
		return millis() - start_time;
	}
};