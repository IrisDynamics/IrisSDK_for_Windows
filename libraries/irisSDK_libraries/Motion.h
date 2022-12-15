/**
    @file Motion.h
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


class Position {
  public: 
  
//    int errors;
//    int logging = 0;
//  
//    static const int log_size = 10;  // even 
//    float error_log[log_size];
//    float history[log_size/2+1];
//  
//    float max_delta = 50;
//    
//    void log () {
//      static float last_position = -1; 
//      if (last_position == -1) last_position = position; 
//      if (abs(last_position - position) > max_delta) {
//        errors++;         
//        logging = log_size/2; 
//        for (int i = 0; i < log_size/2; i++) error_log[i] = history[i];     
//        error_log[log_size/2] = position;
//      }
//      else if (logging) {
//        error_log[log_size - logging--] = position;
//      }
//      for (int i = 0; i < log_size/2; i++) history[i] = history[i+1];
//      history[log_size/2] = position;     
//
//      last_position = position;
//    }
//    
  
    float alpha = 1;
    int dir = 1;

    Position(float a) {
      alpha = a;
    }
    
    virtual void update(float new_measurment) {position = new_measurment;}
    virtual void zero() {position = 0;}
    virtual float get() {return position;}

  //protected: 
    float position = 0; 
};



// This class will integrate sensors like encoders or Iris magnetic sensors that repeat at a fixed interval 
// This class is often used in conjunction with an AbsolutePosition so that the integrated value can be 
// reported from 0 to some maximum number 
class IncrementalPosition : public Position {
  public: 

    float range = 0;
    float value = 0;
    int min = 9000, max = -9000;

    int initialized = 0;

    IncrementalPosition(float a = 1., float r = 2047) : Position(a) {
      range = r; 
    }
  
    void update(float new_measurement) {      
      
//      if (initialized < 50) {
//        position = new_measurement;
//        phase = 0;
//        initialized++;
//        return;
//      }        
//      new_measurement *= dir;
      if (new_measurement - position > range / 2) phase --;  
      else if (new_measurement - position < -range / 2) phase ++;  
      position = new_measurement;      
    }
    float get () {
      value = phase * range + position;
//      if (value < min)  min = value;
//      if (value > max)  max = value;       
      return value;
    }
    void reset (float p) {
      //initialized = 0;
      position = p;
      phase = 0;
    }
  protected: 
    int phase = 0;
    
};


// 
class AbsolutePosition : public Position{
  public: 
    float normalizaed_range = 4096;
    int initialized = 0;
    
    AbsolutePosition(float a, float r) : Position(a) {
      normalizaed_range = r;
    }
    AbsolutePosition(float min_, float max_, float a, float r) : Position(a) {
      min = min_;
      max = max_;
      normalizaed_range = r;
//      initialized = 50;
    }
    void reset (float p) {
      initialized = 0;
      raw      = p;
      last_raw = p;
      min      = p;
      max      = p;
	 visible_range = max - min; 
    }
    void set (float new_pos) {
      normalized = new_pos;
    }
    void  setBounds(float min_, float max_) {
      min = min_;
      max = max_;     
    }
    void update(float new_raw) {

      if (abs(last_raw - new_raw) > 100) {
        errors++;
        last_raw = new_raw;  
        return; 
      }
      //
      if (initialized < 50) {
        raw = new_raw;
        last_raw = new_raw;
        min = new_raw;
        max = new_raw;
        initialized++;
        return;
      }        

      last_raw = new_raw; 
      position = new_raw;
      
      raw = new_raw * alpha + raw * (1 - alpha);
     if (raw < min) min = raw; 
     if (raw > max) max = raw; 
     normalized = normalizaed_range * (raw - min) / (max - min);   
     corrected = raw - min;    
	 visible_range = max - min; 
	 
    }

//    float get () {
//      //if (dir == 1)   
//      return normalized;      
//      //if (dir == -1) return range - normalized;
//      //else return 0;
//    }

    float getRaw() {
      return dir*raw;
    }
	float visibleRange() {
		return visible_range;
	}

  //protected: 
    int errors = 0; 
//    int initialized = 0; 
    float visible_range = 0;
	float raw = 0; 
    float last_raw = 0;
    float min, max;
    float normalized;
    float corrected;
};


// Measures the rate at which the passed information is changing
// Set alpha between 0 and 1 depending on desired filtering 
// Close to 0: Very smooth filtering, very slow to respond to changes (high lag) 
// Close to 1: Very little filtering, very fast to respond.
class Derivative {

  public: 
    float alpha = 1; 
    
    Derivative(float a) : alpha(a) {}
    
    void update(float new_information) {
      uint32_t time_now = micros();
      float derivative_now = 1000000. * (float)(new_information - previous_information) / (float)(time_now - previous_time);
      previous_time = time_now; 
      previous_information = new_information;
      filtered  = filtered * (1-alpha) + derivative_now * alpha;
    }

    float get() {
      return filtered ;
    }

    void reset(float z = 0) {
      filtered = z;
    }

  //protected:
    float filtered = 0; 
    float previous_information;
    uint32_t previous_time;
  
};

//*/