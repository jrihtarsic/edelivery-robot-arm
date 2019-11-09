// include the library code:
#include <LiquidCrystal.h>
#include "DHT.h"
#include <virtuabotixRTC.h> //Libraries needed
#include<Servo.h>
#include<ArmPositionList.h>


// --------------------------------------
// SETUP Arduino pins
// --------------------------------------
// LCD pins
const uint8_t LCD_PIN_DB7 = 13;
const uint8_t LCD_PIN_DB6 = 12;
const uint8_t LCD_PIN_DB5 = 11;
const uint8_t LCD_PIN_DB4 = 10;
const uint8_t LCD_PIN_E  =  9;
const uint8_t LCD_PIN_RS =  8;
// DHT temperature and humidity
const int DHT11_PIN_S    =  0;
// RTC
const uint8_t RTC_PIN_CLK = 1;
const uint8_t RTC_PIN_DAT = 2;
const uint8_t RTC_PIN_RST = 3;
// LCD Buttons
const uint8_t  BTN_LCD_PIN_RIGHT  = A0;
const uint8_t  BTN_LCD_PIN_LEFT   = A1;
// Control joystick
const uint8_t  CTRL_LEFT_PIN_BNT  = A2;
const uint8_t  CTRL_LEFT_PIN_V    = A3;
const uint8_t  CTRL_LEFT_PIN_H    = A4;
const uint8_t  CTRL_RIGHT_PIN_BNT = A5;
const uint8_t  CTRL_RIGHT_PIN_V   = A6;
const uint8_t  CTRL_RIGHT_PIN_H   = A7;
// Servo motors
const uint8_t  SRV_PIN_PEDIESTAL = 7;
const uint8_t  SRV_PIN_LOWER_ARM = 6;
const uint8_t  SRV_PIN_UPPER_ARM = 5;
const uint8_t  SRV_PIN_GRIPPER   = 4;


// --------------------------------------
// Robot arm status variables
// --------------------------------------
const uint8_t ARM_MODE_CONTROL = 0;
const uint8_t ARM_MODE_ACTION1 = 1;
const uint8_t ARM_MODE_DATE_SETUP = 2;

// BUTTON Status
unsigned long BTN_LCD_LEFT_PRESSED = 0;
unsigned long BTN_LCD_RIGHT_PRESSED = 0;

//  Is control conneced
int CTRL_ON = 0;

int CTRL_LEFT_PIN_BNT_PRESSED = 0;
int CTRL_RIGHT_PIN_BNT_PRESSED = 0;
int CTRL_LEFT_PIN_H_ACTION = 0;
int CTRL_LEFT_PIN_V_ACTION = 0;
int CTRL_RIGHT_PIN_H_ACTION = 0;
int CTRL_RIGHT_PIN_V_ACTION = 0;

ArmPositionList quickAction1;

// ARM MODE
int ARM_MODE = ARM_MODE_CONTROL;

// DateTime setup mode
// 0 - day, 1- month, 2 - year, 3 hour, 4 minutes
int DATETIME_SETUP_MODE = 0;


// DISPLAY REFRESH
unsigned long lastRefresh = 0;
const unsigned long LCD_REFRESH_INTERVAL = 1000;
const unsigned long BUTTON_LONG_CLICK = 2000;
const unsigned long BUTTON_LONG_PRESS_INTERVAL = 200;
unsigned long BUTTON_LONG_PRESS_EVENT = 0;


struct ArmConstraints {
  ArmPosition initPosition;
  ArmPosition minPosition;
  ArmPosition maxPosition;

  ArmConstraints():
    initPosition(102,    73, 30, 17, 0),
    minPosition(5,    55, 30, 17, 0),
    maxPosition(175, 175, 165, 110, 0) {};

};
ArmConstraints armConstraints;
ArmPosition currentArmPosition(107, 61, 52, 90, 0);


// --------------------------------------
// Initialize components
// --------------------------------------
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_E, LCD_PIN_DB4, LCD_PIN_DB5, LCD_PIN_DB6, LCD_PIN_DB7);
// initialize libary for DHT11
DHT dht(DHT11_PIN_S, DHT22);
// initialize real time Clock DS 1302
virtuabotixRTC timer(RTC_PIN_CLK, RTC_PIN_DAT, RTC_PIN_RST);
// servo motors
Servo servoPediestal;
Servo servoLowerArm;
Servo servoUpperArm;
Servo servoGripper;



void setup() {
  // start DHT
  dht.begin();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("eDelivery hand!");

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  // this is for initial set..
  //timer.setDS1302Time(00, 30, 20, 1, 6, 10, 2019);

  //
  servoPediestal.attach(SRV_PIN_PEDIESTAL);
  servoLowerArm.attach(SRV_PIN_LOWER_ARM);
  servoUpperArm.attach(SRV_PIN_UPPER_ARM);
  servoGripper.attach(SRV_PIN_GRIPPER);


  pinMode(BTN_LCD_PIN_LEFT, INPUT_PULLUP);
  pinMode(BTN_LCD_PIN_RIGHT, INPUT_PULLUP);

  pinMode(CTRL_LEFT_PIN_BNT, INPUT_PULLUP);
  pinMode(CTRL_RIGHT_PIN_BNT, INPUT_PULLUP);

  // pull all joystick pins to up  if they are all
  // 1023 than joistick is not connected
  pinMode(CTRL_LEFT_PIN_V, INPUT_PULLUP);
  pinMode(CTRL_LEFT_PIN_H, INPUT_PULLUP);
  pinMode(CTRL_RIGHT_PIN_V, INPUT_PULLUP);
  pinMode(CTRL_RIGHT_PIN_H, INPUT_PULLUP);


  currentArmPosition = armConstraints.initPosition;


  quickAction1.StartMovement(90, 136, 125, 60);
  quickAction1.AddPosition(31, 55, 90, 70, 50);
  quickAction1.AddPosition(31, 70, 33, 70, 20);
  quickAction1.AddPosition(31, 110, 33, 70, 20);
  quickAction1.AddPosition(31, 110, 33, 20, 20);
  quickAction1.AddPosition(31, 79, 51, 20, 50);
  quickAction1.AddPosition(31, 75, 109, 20, 50);
  quickAction1.AddPosition(90, 136, 125, 24, 20);

  quickAction1.moveToStart();


}

void loop() {
  readLCDButtons();

  if (ARM_MODE == ARM_MODE_DATE_SETUP) {
    processDateTimeSetup();

  } else if (ARM_MODE == ARM_MODE_ACTION1) {
    processAction1();
  } else {
    // read values from control
    if ( readControl() && CTRL_LEFT_PIN_BNT_PRESSED == 0 ) {
      processControlValues();
    } else {
      //
      unsigned long currentTime = millis();
      if ( currentTime - lastRefresh > LCD_REFRESH_INTERVAL) {
        lastRefresh = currentTime;

        refreshDisplayTimeDateHumidity();
      }
    }
  }
}

/**
   Process action 1
*/
void processAction1() {
  if (quickAction1.UpdateCurrentPosition()) {

    ArmPosition pos = quickAction1.getCurrentPosition();
    currentArmPosition = pos;

    updateServoPosition();
  } else {
    ARM_MODE = ARM_MODE_CONTROL;
  }

}
/**
   Process dateTume
*/
void processDateTimeSetup() {
  timer.updateTime();
  lcd.setCursor(0, 0);
  switch (DATETIME_SETUP_MODE) {
    case 0:
      lcd.print("Setup:dayOfMonth");
      lcd.setCursor(0, 1);
      lcd.print(String(timer.dayofmonth));
      break;
    case 1:
      lcd.print("Setup: month");
      lcd.setCursor(0, 1);
      lcd.print(String(timer.month));
      break;
    case 2:
      lcd.print("Setup: year");
      lcd.setCursor(0, 1);
      lcd.print(String(timer.year));
      break;
    case 3:
      lcd.print("Setup: hour");

      lcd.setCursor(0, 1);
      lcd.print(String(timer.hours));
      break;
    case 4:
      lcd.print("Setup: minute");
      lcd.setCursor(0, 1);
      lcd.print(String(timer.minutes));
      break;
  }
  lcd.print("    ");

}

void processControlValues() {

  // check if right control button was pressed - if yes reset values
  // else calculate new position
  if (CTRL_RIGHT_PIN_BNT_PRESSED == 0) {
    currentArmPosition.pediestal  += CTRL_RIGHT_PIN_H_ACTION;
    currentArmPosition.lower_arm += CTRL_RIGHT_PIN_V_ACTION;
    currentArmPosition.upper_arm += CTRL_LEFT_PIN_V_ACTION;
    currentArmPosition.gripper   += CTRL_LEFT_PIN_H_ACTION;

    currentArmPosition.pediestal = setBorders(currentArmPosition.pediestal, armConstraints.minPosition.pediestal, armConstraints.maxPosition.pediestal);
    currentArmPosition.lower_arm = setBorders(currentArmPosition.lower_arm,  armConstraints.minPosition.lower_arm,  armConstraints.maxPosition.lower_arm);
    // calculate borders for upper arm
    currentArmPosition.upper_arm = setBorders(currentArmPosition.upper_arm,  armConstraints.minPosition.upper_arm,  armConstraints.maxPosition.upper_arm);
    currentArmPosition.gripper   = setBorders(currentArmPosition.gripper,  armConstraints.minPosition.gripper,  armConstraints.maxPosition.gripper);

  } else {
    currentArmPosition = armConstraints.initPosition;
  }
  updateServoPosition();
}


void updateServoPosition() {

  servoPediestal.write(currentArmPosition.pediestal);
  servoLowerArm.write(currentArmPosition.lower_arm);
  servoUpperArm.write(currentArmPosition.upper_arm);
  servoGripper.write(currentArmPosition.gripper);
  // waits for the servos to reach the position
  delay(30);

  // display values to LCD
  lcd.setCursor(0, 0);
  lcd.print("PD:");
  lcd.print((int)currentArmPosition.pediestal);
  lcd.print(" LA:");
  lcd.print((int)currentArmPosition.lower_arm);
  lcd.print("    "); // clean

  lcd.setCursor(0, 1);
  lcd.print("UA:");
  lcd.print((int)currentArmPosition.upper_arm);
  lcd.print(" GRP:");
  lcd.print((int)currentArmPosition.gripper);
  lcd.print("    ");

}
/**
   Method calculates servmo turn speed
*/
int calculateDeltaAngle(int value) {
  int iDeltaAngle = 0;

  if (value > 950) {
    iDeltaAngle = 4;
  } else if (value > 900) {
    iDeltaAngle = 2;
  } else if (value > 700) {
    iDeltaAngle = 1;
  } else if (value < 50) {
    iDeltaAngle = -4;
  } else if (value < 100) {
    iDeltaAngle = -2;
  } else if (value < 300) {
    iDeltaAngle = -1;
  }
  return iDeltaAngle;
}


/**
   Make sure value is inside boundaries
*/
int setBorders(int value, int minVal, int maxVal) {
  return value > minVal ? (value < maxVal ? value : maxVal) : minVal;
}
/**
   Read LCD button status and fire event if button is clicked
   Milis go back to zero approximately 50 days! -
   Buttons are pullup so at press they have low value!
*/
void readLCDButtons() {
  // read left button
  if (digitalRead(BTN_LCD_PIN_LEFT) == LOW) {
    BTN_LCD_LEFT_PRESSED = BTN_LCD_LEFT_PRESSED > 0 ? BTN_LCD_LEFT_PRESSED : millis();
  } else {
    if (BTN_LCD_LEFT_PRESSED > 0) {
      if ( millis() - BTN_LCD_LEFT_PRESSED  > BUTTON_LONG_CLICK) {
        lcdLeftButtonLongClickEvent();
      } else {
        lcdLeftButtonClickEvent();
      }
    }
    BTN_LCD_LEFT_PRESSED = 0;
  }

  // read right button
  if (digitalRead(BTN_LCD_PIN_RIGHT) == LOW) {
    BTN_LCD_RIGHT_PRESSED = BTN_LCD_RIGHT_PRESSED > 0 ? BTN_LCD_RIGHT_PRESSED : millis();
    if ( millis() - BTN_LCD_RIGHT_PRESSED  > BUTTON_LONG_CLICK) {
      lcdRightButtonLongPressEvent();
    }

  } else {
    if (BTN_LCD_RIGHT_PRESSED > 0) {
      if ( millis() - BTN_LCD_RIGHT_PRESSED  > BUTTON_LONG_CLICK) {
        lcdRightButtonLongClickEvent();
      } else {
        lcdRightButtonClickEvent();
      }
    }
    BUTTON_LONG_PRESS_EVENT = 0;
    BTN_LCD_RIGHT_PRESSED = 0;
  }
}

/*
  Mehtod reads values from joystick and caluclates
  delta angle for each motor

*/
boolean readControl() {
  int iCtrLeftH = analogRead(CTRL_LEFT_PIN_H);
  int iCtrLeftV = analogRead(CTRL_LEFT_PIN_V);
  int iCtrRightH = analogRead(CTRL_RIGHT_PIN_H);
  int iCtrRightV = analogRead(CTRL_RIGHT_PIN_V);
  // test if any of the values are not pulled up;
  //

  CTRL_ON = iCtrLeftH < 950
            || iCtrLeftV < 950
            || iCtrRightH < 950
            || iCtrRightV < 950;


  if (!CTRL_ON) {
    CTRL_LEFT_PIN_H_ACTION = 0;
    CTRL_LEFT_PIN_V_ACTION = 0;
    CTRL_RIGHT_PIN_H_ACTION = 0;
    CTRL_RIGHT_PIN_V_ACTION = 0;
    return false;
  }

  CTRL_LEFT_PIN_H_ACTION = calculateDeltaAngle(iCtrLeftH);
  CTRL_LEFT_PIN_V_ACTION = calculateDeltaAngle(iCtrLeftV) * -1;
  CTRL_RIGHT_PIN_H_ACTION = calculateDeltaAngle(iCtrRightH);
  CTRL_RIGHT_PIN_V_ACTION = calculateDeltaAngle(iCtrRightV);


  if (digitalRead(CTRL_LEFT_PIN_BNT) == LOW) {
    CTRL_LEFT_PIN_BNT_PRESSED = 1;
  } else {
    CTRL_LEFT_PIN_BNT_PRESSED = 0;
  }

  if (digitalRead(CTRL_RIGHT_PIN_BNT) == LOW) {
    CTRL_RIGHT_PIN_BNT_PRESSED = 1;
  } else {
    CTRL_RIGHT_PIN_BNT_PRESSED = 0;
  }
  return true;
}

/*
  Mehtod reads date/time, temperature and humidity
  and prints values to Lcd display
*/
void refreshDisplayTimeDateHumidity() {

  timer.updateTime();
  // print date time
  lcd.setCursor(0, 0);
  // todo fix 00 formating..
  String timeString = String(timer.dayofmonth);
  timeString = String(timeString + "/");
  timeString = String(timeString + timer.month);
  timeString = String(timeString + "/");
  timeString = String(timeString + timer.year);
  timeString = String(timeString + " ");
  timeString = String(timeString + timer.hours);
  timeString = String(timeString + ":");
  timeString = String(timeString + timer.minutes);


  // make sure that is 16 chars to clear row
  while (timeString.length() < 16) {
    timeString = String(timeString + " ");
  }
  lcd.print(timeString);


  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (!isnan(t) && !isnan(h)) {
    int it1 = t * 10;
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("T:"); // Prints string "Temp." on the LCD
    lcd.print((int)t); //  trucate the decimal
    lcd.print(" C,");
    lcd.print(" H:");
    lcd.print((int)h); // trucate the decimal
    lcd.print(" %");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Reading T&H ... "); //
  }
}
/**
   Left LCD button click event
*/
void lcdLeftButtonClickEvent() {

  if (ARM_MODE == ARM_MODE_DATE_SETUP) {
    DATETIME_SETUP_MODE++;
    if (DATETIME_SETUP_MODE > 4) {
      DATETIME_SETUP_MODE = 0;
    }
    lcd.clear();
  } else {
    // start quick action 1
    quickAction1.StartMovement(currentArmPosition);
    ARM_MODE = ARM_MODE_ACTION1;
  }
}

/**
  Left LCD button LONG click event
*/
void lcdLeftButtonLongClickEvent() {
  ARM_MODE = ARM_MODE == ARM_MODE_DATE_SETUP ? ARM_MODE_CONTROL : ARM_MODE_DATE_SETUP;
  quickAction1.StopMovement();
  // mode is changed clear LCD
  lcd.clear();

  if (ARM_MODE == ARM_MODE_DATE_SETUP) {
    DATETIME_SETUP_MODE = 0;
  }
}


/**
   Right LCD button click event
*/
void lcdRightButtonClickEvent() {
  if (ARM_MODE == ARM_MODE_DATE_SETUP) {
    timer.updateTime();
    switch (DATETIME_SETUP_MODE) {
      case 0: {
          timer.dayofmonth++;
          timer.dayofmonth = timer.dayofmonth > 31 ? 1 : timer.dayofmonth;
          break;
        case 1:
          timer.month++;
          timer.month = timer.month > 12 ? 1 : timer.month;
          break;
        case 2:
          timer.year++;
          timer.year = timer.year > 2050 ? 2019 : timer.year;
          break;
        case 3:
          timer.hours++;
          timer.hours = timer.hours > 24 ? 1 : timer.hours;
          break;
        case 4:
          timer.minutes++;
          timer.minutes = timer.minutes > 59 ? 1 : timer.minutes;
          break;
        }
    }
    timer.setDS1302Time(00, timer.minutes, timer.hours, 0, timer.dayofmonth, timer.month, timer.year);
  }
}
/**
  Right LCD button LONG click event
*/
void lcdRightButtonLongPressEvent() {
  if ( millis() - BUTTON_LONG_PRESS_EVENT  > BUTTON_LONG_PRESS_INTERVAL) {
    lcdRightButtonClickEvent();
    BUTTON_LONG_PRESS_EVENT =  millis() ;
  }

}

void lcdRightButtonLongClickEvent() {

}
