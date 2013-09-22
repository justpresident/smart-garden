#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Time.h>

#include "water_pump.h"

// HARDWARE
#define PUMP_PIN 13
#define BTN1_PIN 7
#define BTN2_PIN 8

SimpleBtn btn1(BTN1_PIN);
SimpleBtn btn2(BTN2_PIN);
Pump pump(PUMP_PIN);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//##################
#define A_PUMP_SETTINGS 10
#define SAVE_INTERVAL 3600
// system state vars
#define S_NONE 0
#define S_WORK 1
#define S_FREQ_CONTROL 2
#define S_AMOUNT_CONTROL 3

const char* headers[] = {
  "            ",
  "Next watering in:",
  "Frequency:",
  "Amount:"
};

byte main_state;
time_t last_save_time;

void setup() {
  main_state = S_WORK;
  lcd.begin(16,2);
  lcd.noAutoscroll();
  setup_display();
  
  pump.load_settings(A_PUMP_SETTINGS);
  last_save_time = now();
}

void control_state_change() {
  if (!btn2.unique_pressed())
    return;
    
  switch(main_state) {
    case S_WORK:
      main_state = S_FREQ_CONTROL;
      break;
    case S_FREQ_CONTROL:
      main_state = S_AMOUNT_CONTROL;
      if (btn1.was_pressed())
        save();
      
      break;
    case S_AMOUNT_CONTROL:
      main_state = S_WORK;
      if (btn1.was_pressed())
        save();
        
      break;
  }
  btn1.reset();
  btn2.reset();
  setup_display();
}

void setup_display() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(headers[main_state]);
  lcd.setCursor(0,1);
}

void loop() {
  control_state_change();
  lcd.setCursor(0,1);
  
  switch (main_state) {
    case S_WORK:
      work();
      break;
    case S_FREQ_CONTROL:
      freq_control();
      break;
    case S_AMOUNT_CONTROL:
      amount_control();
      break;
  }
  lcd.print(headers[0]);
  
  if (now() - last_save_time > SAVE_INTERVAL) {
    save();
  }
  
  delay(1);
}

void save() {
  int saved = pump.save_settings(A_PUMP_SETTINGS);
  last_save_time = now();

  lcd.setCursor(0,1);
  lcd.print("Saved...    ");

  delay(500);
  pump.load_settings(A_PUMP_SETTINGS);
}

void work() {
  time_t next_time = pump.next_watering_time();
  int days = next_time/DAY_SEC;
  next_time -= (next_time/DAY_SEC)*DAY_SEC;
  int hours = next_time/3600;
  next_time -= (next_time/3600)*3600;
  int minutes = next_time/60;
  int seconds = next_time - (next_time/60)*60;
  
  lcd.print(days);
  lcd.print(" days ");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);

  pump.work();
}


void amount_control() {
  if (btn1.pressed())
    digitalWrite(PUMP_PIN, HIGH);
  else {
    digitalWrite(PUMP_PIN, LOW);
  }
  if (btn1.was_pressed()) {
    pump.watering_amount = btn1.last_press_duration();
  }
  
  lcd.print(int(pump.watering_amount/1000));
  lcd.print(".");
  lcd.print(pump.watering_amount % 1000);
  lcd.print(" sec        ");
}

void freq_control() {
  if (btn1.unique_pressed()) {
    pump.interval_days += 1;
    if (pump.interval_days > 14)
      pump.interval_days = 1;
  }
  lcd.print("Every ");
  lcd.print(pump.interval_days);
  lcd.print(" day");
  if(pump.interval_days > 1)
    lcd.print("s");
}
