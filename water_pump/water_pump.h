#ifndef WaterPump_h
#define WaterPump_h

#include <EEPROM.h>
#include <Arduino.h>

#include <Time.h>

#define DAY_SEC 86400

// EEPROM ############# 

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return ee;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return ee;
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
        return (millis() - last_press_time);
      }
    }
    
    bool pressed() {
      bool now_pressed = (digitalRead(_pin) == HIGH);
      
      if (_cur_pressed && !now_pressed) {
        _cur_pressed = false;
        _last_press_duration = millis() - last_press_time;
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
    unsigned long _last_watering_amount;
    time_t _last_watering_time;
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
    
    int save_settings(int ee) {
      ee = EEPROM_writeAnything(ee, now());
      ee = EEPROM_writeAnything(ee, _last_watering_time);
      ee = EEPROM_writeAnything(ee, _last_watering_amount);
      ee = EEPROM_writeAnything(ee, interval_days);
      ee = EEPROM_writeAnything(ee, watering_amount);
      
      return ee;
    }
    int load_settings(int ee) {
      time_t cur_time;
      ee = EEPROM_readAnything(ee, cur_time);
      if(cur_time > 0)
        setTime(cur_time);
      ee = EEPROM_readAnything(ee, _last_watering_time);
      ee = EEPROM_readAnything(ee, _last_watering_amount);
      ee = EEPROM_readAnything(ee, interval_days);
      ee = EEPROM_readAnything(ee, watering_amount);
      
      return ee;
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

