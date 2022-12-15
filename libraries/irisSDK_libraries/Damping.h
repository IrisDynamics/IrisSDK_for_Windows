/**
 *  @file Damping.h
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

#pragma once 
#include "effect.h"

// =================================================== DAMPING EFFECT 
class DampingEffect : public Effect {
  public:

    float k; 
		float dummy;

    DampingEffect(float & _speed, float k_ = 0): speed(_speed) {
      k = k_;
    }		
    DampingEffect() : speed(dummy) {
      k = 0;
    }
    virtual ~DampingEffect() {};
    
    void update() {
      //Printl("--Updating Damping"); 
      force = speed * k;
    }    
		
    void update( float v) {
      //Printl("--Updating Damping"); 
      force = v * k;
    }    
		
    
    void adjustK(float newk) {
      k = newk;
    }
  protected: 
    float & speed;
};

class DampingEffect_2D : public Effect_2D {
  public:

    DampingEffect_2D(float & xderiv, float & yderiv, float kx, float ky) {
      XDamp = new DampingEffect(xderiv, kx);
      YDamp = new DampingEffect(yderiv, ky);
    }
    DampingEffect_2D(float kx, float ky) {
      XDamp = new DampingEffect(kx);
      YDamp = new DampingEffect(ky);
    }		
		~DampingEffect_2D() {
			delete XDamp;
			delete YDamp;
		}
    
    void update() {
      XDamp->update();
      YDamp->update();
      
      x_force = XDamp->getForce();
      y_force = YDamp->getForce();
    }
    
    void update(float v1, float v2) {
      XDamp->update(v1);
      YDamp->update(v2);
      
      x_force = XDamp->getForce();
      y_force = YDamp->getForce();
    }
		
    void adjustK (float new_k) {
      XDamp->k = new_k;
      YDamp->k = new_k;
    }

  protected: 
    DampingEffect * XDamp, * YDamp;
    
};
