/**
 * @file ConstantForce.h
    @version 1.1
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
#ifndef CONSTANTFORCE_H_
#define CONSTANTFORCE_H_
#include "Effect.h"

class ConstantForce : public Effect {
  public: 
    void update ( int new_force ) { 
      force = new_force;
      last_update = millis();       
    }
    int getForce() {
      //Printl("--Getting Constant Force"); 
   //   if (millis() - last_update < nofade_duration) return force;       
   //   else if (millis() - last_update < nofade_duration + rampdown_duration) {
		 // return force * (1. - (millis() - last_update - nofade_duration) / (float)rampdown_duration);      
	  //}
   //   else return 0;
		return force;
    }
    void setNoFadeDuration(int ms) {
//      ms = min (ms, 500); 
//      nofade_duration = ms;
    }
    void setRampdownDuration(int ms) {
//      ms = min (ms, 1500); 
//      rampdown_duration = ms;
    }
  protected: 
    int force; 
    uint32_t last_update = 0, nofade_duration = 100, rampdown_duration = 1000;
};

#endif
