#ifndef WaterPump_h
#define WaterPump_h

#include <Time.h>


#include "Arduino.h"

#define DAY_SEC 86400
#define MAX_ULONG 4294967295

unsigned long time_diff(unsigned long time_new, unsigned long time_old) {
  if (time_new < time_old)
    return time_new + (MAX_ULONG - time_old);
  else
    return time_new - time_old;
}

class SimpleBtn {
  private:
    byte _pin;
    bool _cur_pressed;
    bool _was_pressed;
    unsigned long last_press_time;
    unsigned long _last_press_duration;
  public:
    SimpleBtn(byte pin) {
      _pin = pin;
      pinMode(_pin, INPUT);
    
      last_press_time = millis(); 
      _cur_pressed = false;
      reset();
    }
    
    void reset() {
      _was_pressed = false;
    }
    
    bool was_pressed() {
      return _was_pressed;
    }
    
    unsigned long last_press_duration() {
      if (!was_pressed())
        return 0;
      else if (!pressed())
        return _last_press_duration;
      else { // if button is pressed now
        return time_diff(millis(), last_press_time);
      }
    }
    
    bool pressed() {
      bool now_pressed = (digitalRead(_pin) == HIGH);
      
      if (_cur_pressed && !now_pressed) {
        _cur_pressed = false;
        _last_press_duration = time_diff(millis(), last_press_time);
      } else if (!_cur_pressed && now_pressed) {
        last_press_time = millis();
        _cur_pressed = true;
        _was_pressed = true;
      }
      
      return now_pressed;
    }
    
    bool unique_pressed() {
      if (!_cur_pressed) {
        if (pressed())
          return true;
      }
      pressed();
      return false;
    }
};

class Pump {
  private:
    int _pin;
    time_t _last_watering_time;
    unsigned long _last_watering_amount;
    
  public:
    int interval_days;
    unsigned long watering_amount;
    
    Pump(int pin) {
      _pin = pin;
      pinMode(_pin, OUTPUT);
      
      _last_watering_time = 0;
      _last_watering_amount = 0;
      interval_days = 0;
      watering_amount = 0;
    }
    
    void do_watering(unsigned long amount = 0) {
      if (amount == 0)
        amount = watering_amount;
      digitalWrite(_pin, HIGH);
      delay(amount);
      digitalWrite(_pin, LOW);
      _last_watering_time = now();
      _last_watering_amount = amount;
    }
    
    time_t next_watering_time() {
      if (interval_days > 0)
        return _last_watering_time + interval_days * DAY_SEC - now();
      else
        return 0;
    }
    
    void work() {
      if (interval_days > 0 && _last_watering_time + interval_days * DAY_SEC <= now())
        do_watering();
    }
};
#endif

