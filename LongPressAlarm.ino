/*
  Powerful Alarm Clock with Long Press Alarm End Button to actually make you Wake Up!
  No more Snoozes!!
  
  Prashant Kumar

  Single button operation. Auto update of time everyday using WiFi.
  Once alarm starts, user needs to press and hold its button for 25 seconds to end alarm.
  Alarm pauses once user presses button and shows a 25 seconds countdown. If user leaves the button
  before countdown end, the countdown resets and alarm starts again. A 3 minute max alarm time is there
  if alarm is not deactivated by user. To make single button operation possible, a push button arduino
  library has been made, named PushButtonTaps. It classifies push button presses as No Tap, Single Tap,
  Double Tap and Long Press.

  ESP32 is put into Light Sleep when display backlight is Off. It is awaken intermitently or by button press.

  Power Consumption:
  - While using WiFi ~ 160-260mA 0.8W-1.3W - for a few seconds to update time from internet once everyday
  - While Display backlight is On - 92mA 460mW - for 20 seconds when button is pressed
  - While Alarm and Buzzer Beep is On - 125mA 625mW - during 0.8 second beeps while Alarm is On
  - While Display backlight is Off but ESP32 is awake - 75mA - happens for ~30 milliseconds during display time updates
  - While Display backlight is Off and ESP32 is on Light Sleep - 17.5mA 88mW - happens around 99.8% of the time of the day

  Continous Power Consumption is 5V-17.5mA-88mW, 99.8% of time during normal operation. Intermittent Max is 1.3W.

  Video: https://youtu.be/btpxy9vB6y4
  Github: https://github.com/pk17r/LongPressAlarm
  
  Components Required:
  Microcontroller - ESP32
  Buzzer - Passive Buzzer - KSSG1203-42 - Rated Frequency 2048Hz, 3-5V, 35mA
  Fast switching diode - INA4148 - across buzzer to contain back currents (optional)
  Display - LCD 1602A with PCF8574T I/O Expander - I2C Communication
  Level Shifter - TXS0108E - to connect 5V LCD I2C to 3.3V ESP32 I2C as ESP32 pins are not 5V tolerant
  Button - 1x Tactile Button Pulled Up with a 10K resistor pull-up to 3.3V and a 100nF Decoupling Capacitor to ground
  MOSFET - 1x N7000 NPN MOSFET - Powers Buzzer with 5V and driven by ESP32 Buzzer Drive Pin to Gate
  LED - 1x 5mm LED that flashes along with Buzzer, driven by ESP32 Buzzer Drive Pin
  Resistor - 1x 10Ohm - In series with Buzzer to contain current to 35mA - 5V to Buzzer+

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  Features:
  - requires user to press and hold button for 25 seconds to end alarm, otherwise it will restart buzzing
  - one button operation - uses only 1 button to set/enable/disable alarm and end active alarm
  - remembers set alarm time on reboot
  - updates time using WiFi from NTC server at start of microcontroller and everyday 1 hr after the set alarm time
  - drives a 90dB passive buzzer at rated frequency using timer of ESP32 and a mosfet at 5V
  - updates displayed time every second, shows today's day and date

  Requires:
  - Add a file named "secrets_file.h" with your WiFi SSID and Password in the following format:
  #define WIFI_SSID         "ssid"
  #define WIFI_PASSWORD     "password"

  File descriptions:
  - LongPressAlarmClock.ino - this empty file. File names govern Arduino's file pre-processor order
  - sketch.ino - contains Arduino's setup and loop functions as well buzz alarm function
  - a_display_lcd1602a.ino - main display file
  - a_preferences_esp32_nvs_memory.ino - ESP32's NVS memory functions
  - a_timer_esp32.ino - timer file
  - a_wifi_and_time_info.ino - wifi and time info file
  - b_display_lcd1602a_dependent_functions.ino - display functions that depend upon objects from other files
  - c_ESP32_Light_Sleep_n_PushBtn.ino - contains ESP32 Put to Light Sleep and Wake functions & Push Button
  - z_user_serial_inputs.ino - serial inputs files
  - secrets_file.h - wifi ssid and password file
*/
