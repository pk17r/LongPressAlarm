/*
  Arduino's Setup and Loop Functions
  buzzAlarmFn() Function
  updateSecondsToAlarm() Function
*/

// function declerations
void processSetAlarmPageUserInput(byte buttonUserInput);

// time update everyday from internet
bool timeUpdatedFromInternetToday = false;

/*
  The Arduino setup function
*/
void setup(){
  serial_init();

  // ESP32 check program running core
  // Serial.print(F("setup() running on core "));
  // Serial.println(xPortGetCoreID());

  // lcd 1602A init
  lcd_init();

  // init saved data in EEPROM
  esp32_preferences_eeprom_init();
  
  // get current local time from Internet
  while(!connectWiFiAndUpdateCurrentTimeFromInternet()) {
    Serial.println(F("Disconnecting and Reconnecting WiFi and attempting current time update from internet again."));
    failedToObtainTimeScreen();
    delay(3000);
  }

  // initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // initialize push button
  pushBtn.setButtonPin(BUTTON_PIN);

  timer_init();
  timer_enable(timeUpdateTimerPtr);
}

/*
  The Arduino loop function
*/
void loop(){
  // update secondsToAlarm
  updateSecondsToAlarm();

  // Activate Buzzer at Alarm Time
  if(alarmActive && secondsToAlarm == 0)
    buzzAlarmFn();

  // light sleep until alarm or if backlight is Off
  if(secondsToAlarm > 100 && !backlightOn)
    putEsp32ToLightSleep();

  // update local time every second (controlled by timer ISR)
  if(timeNeedsToBeUpdated) {
    updateCurrentTime();
    timeNeedsToBeUpdated = false;
    // print local time if not in Set Alarm Page
    if(!setAlarmPageActive)
      displayLocalTimeAndDate();
  }
  
  // try update time at noon everyday
  if(!timeUpdatedFromInternetToday && currentTimeInfo.tm_hour == 12) {
    connectWiFiAndUpdateCurrentTimeFromInternet();
    timeUpdatedFromInternetToday = true;
  }
  if(timeUpdatedFromInternetToday && currentTimeInfo.tm_hour == 0)   //reset for next day time update
    timeUpdatedFromInternetToday = false;

  // turn off display backlight after BACKLIGHT_TURNOFF_AFTER_MS of being On
  if(backlightOn && millis() - backlightTurnedOnAtMs > BACKLIGHT_TURNOFF_AFTER_MS) {
    lcd.noBacklight();
    backlightOn = false;
  }

  // check for button press user input
  byte buttonUserInput = pushBtn.checkButtonStatus();
  if(buttonUserInput)
    if(!backlightOn)
      turnBacklightOn();
    else
      processSetAlarmPageUserInput(buttonUserInput);

  // serial inputs and processing for debugging and development
  if(Serial.available()) {
    processSerialInput();
  }
}

/*
  Function that starts buzzer and Alarm Screen
  It wait for user to press button to pause buzzer
  User needs to continue to press and hold button for
  ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS to end alarm.
  If user stops pressing button before alarm end, it will
  restart buzzer and the alarm end counter.
  If user does not end alarm by ALARM_MAX_ON_TIME_MS milliseconds,
  it will end alarm on its own.
*/
void buzzAlarmFn() {
  // end Set Alarm Page flag if at all On
  setAlarmPageActive = false;
  //start buzzer!
  buzzer_enable();
  bool alarmStopped = false, buzzerPausedByUser = false;
  unsigned long alarmStartTimeMs = millis();
  int buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
  alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
  while(!alarmStopped) {
    // if user presses button then pauze buzzer and start alarm end countdown!
    if(pushBtn.buttonActiveDebounced()) {
      if(!buzzerPausedByUser) {
        buzzer_disable();
        buzzerPausedByUser = true;
      }
      unsigned long buttonPressStartTimeMs = millis(); //note time of button press
      // while button is pressed, display seconds countdown
      while(pushBtn.buttonActiveDebounced() && !alarmStopped) {
        // display countdown to alarm off
        if(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS - (millis() - buttonPressStartTimeMs) / 1000 < buttonPressSecondsCounter) {
          buttonPressSecondsCounter--;
          alarmOnScreen(buttonPressSecondsCounter);
        }
        // end alarm after holding button for ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS
        if(millis() - buttonPressStartTimeMs > ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS * 1000) {
          alarmStopped = true;
          // good morning screen! :)
          goodMorningScreen();
        }
      }
    }
    // activate buzzer if button is not pressed by user
    if(!pushBtn.buttonActiveDebounced() && !alarmStopped) {
      if(buzzerPausedByUser) {
        buzzer_enable();
        buzzerPausedByUser = false;
      }
      // if user lifts button press before alarm end then reset counter and re-display alarm-On screen
      if(buttonPressSecondsCounter != ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS) {
        // display Alarm On screen with seconds user needs to press and hold button to end alarm
        buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
        alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
      }
    }
    // if user did not stop alarm within ALARM_MAX_ON_TIME_MS, make sure to stop buzzer
    if(millis() - alarmStartTimeMs > ALARM_MAX_ON_TIME_MS) {
      buzzer_disable();
      alarmStopped = true;
    }
  }
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
}

/*
  Seconds to next alarm
*/
void updateSecondsToAlarm() {
  unsigned long secondsRightNowToday = currentTimeInfo.tm_hour * 60 * 60 + currentTimeInfo.tm_min * 60 + currentTimeInfo.tm_sec;
  unsigned long alarmSecondsToday = alarmHour * 60 * 60 + alarmMin * 60;
  if(alarmSecondsToday >= secondsRightNowToday)
    secondsToAlarm = alarmSecondsToday - secondsRightNowToday;
  else
    secondsToAlarm = alarmSecondsToday + 24*60*60 - secondsRightNowToday;
}
