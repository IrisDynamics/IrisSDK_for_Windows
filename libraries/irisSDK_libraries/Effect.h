#pragma once 

/**
 * Templates for 1D and 2D Feedback and Haptic Effects
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

//#include <General_Macros.h>
//#include "Arduino.h"
#include "device_config.h"

class Effect {
  public: 

    int enabled = 1;
    
//    Effect() {}
    virtual ~Effect() {}

    int saturation = 0xFFFF;
    
    virtual void start(uint32_t dur = 0) {
      start_time = millis();
      duration = dur;      
      enabled = 1; 
    }
    virtual void stop() {
      start_time = 0;
      enabled = 0;
    }
    virtual void update() {}
    virtual void update(float v) {}
    virtual int getForce() {if (!enabled) return 0; return constrain(force,-saturation, saturation);}
    
    virtual void set (int newforce) {force = newforce;}
    
  protected: 
    int constrain(int force, int min, int max) {
        if (force < min) return min;
        if (force > max) return max;
        return force;
    }
    int force = 0;
    uint32_t start_time, last_update, duration;    
    
};

class Effect_2D : public Effect{
  public: 
    
		virtual ~Effect_2D(){};
		int getForce() {return abs(x_force) + abs(y_force);}		
    virtual int32_t getXforce() {return constrain(x_force,-saturation, saturation);}
    virtual int32_t getYforce() {return constrain(y_force,-saturation, saturation);}
    
  protected: 
    int x_force = 0, y_force = 0;   

    
};


