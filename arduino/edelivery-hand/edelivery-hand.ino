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
// Other constants
const unsigned long LCD_INFO_REFRESH_INTERVAL = 5000;
const unsigned long LCD_INTRO_INTERVAL = 5000;
const unsigned long MODE_CHANGE_INTERVAL = 1500;
const unsigned long BUTTON_LONG_CLICK = 1500;
const unsigned long BUTTON_LONG_PRESS_INTERVAL = 200;


// --------------------------------------
// Robot arm status variables
// --------------------------------------
const uint8_t ARM_MODE_NOT_DEFINED = -1;
const uint8_t ARM_MODE_INFO = 0; // mode shows date time and temperature and
const uint8_t ARM_MODE_CONTROL = 1; // reads from the control
const uint8_t ARM_MODE_ACTION1 = 2; // quick action 1
const uint8_t ARM_MODE_SELECT = 4; // mode selection
const uint8_t ARM_MODE_DATE_SETUP = 5; // date selection
const uint8_t ARM_MODE_INTRO = 6; // startup message


// button timestamp when it was pressed
unsigned long BTN_LCD_LEFT_PRESSED = 0;
unsigned long BTN_LCD_RIGHT_PRESSED = 0;
bool BTN_LCD_LEFT_LONGPRESS_FIRED = false;
bool CONSOLE_WARNING_SHOWN = false;

// Detect flag if control is conneced
int CTRL_ON = 0;

// control statuses
int CTRL_LEFT_PIN_BNT_PRESSED = 0;
int CTRL_RIGHT_PIN_BNT_PRESSED = 0;
int CTRL_LEFT_PIN_H_ACTION = 0;
int CTRL_LEFT_PIN_V_ACTION = 0;
int CTRL_RIGHT_PIN_H_ACTION = 0;
int CTRL_RIGHT_PIN_V_ACTION = 0;

// quick action
ArmPositionList quickAction1;
// return to base positionaction
ArmPositionList returnAction;


// ARM MODE
int ARM_MODE = ARM_MODE_INFO;
int ARM_MODE_PREVIOUS = ARM_MODE_INFO;
// when choosing arm mode this is
// current selected mode
int ARM_MODE_CHANGE_MODE = ARM_MODE_NOT_DEFINED;

// DateTime setup mode
// 0 - day, 1- month, 2 - year, 3 - hour, 4 - minutes
int DATETIME_SETUP_MODE = 0;


// DISPLAY REFRESH
unsigned long lastInfoRefreshTime = 0;
unsigned long introStartTime = 0;
unsigned long modeChangeTime = 0;

unsigned long BUTTON_LONG_PRESS_EVENT = 0;


struct ArmConstraints {
  ArmPosition initPosition;
  ArmPosition minPosition;
  ArmPosition maxPosition;

  ArmConstraints():
    initPosition(102,    73, 30, 20, 0),
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



/****************************************************************
   Initialization function
****************************************************************/
void setup() {
  // start DHT
  dht.begin();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);


  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  // this is for initial set..
  //timer.setDS1302Time(00, 1, 1, 1, 1, 1, 2019);

  //


  pinMode(BTN_LCD_PIN_LEFT, INPUT_PULLUP);
  pinMode(BTN_LCD_PIN_RIGHT, INPUT_PULLUP);
  pinMode(CTRL_LEFT_PIN_BNT, INPUT_PULLUP);
  pinMode(CTRL_RIGHT_PIN_BNT, INPUT_PULLUP);

  // set current arm position to init position
  currentArmPosition = armConstraints.initPosition;
  // quick action 1 vector
  quickAction1.AddPosition(35, 55, 86, 80, 20);
  quickAction1.AddPosition(35, 55, 45, 70, 30);
  quickAction1.AddPosition(35, 110, 45, 70, 30);
  quickAction1.AddPosition(35, 110, 45, 20, 10);
  quickAction1.AddPosition(35, 58, 80, 20, 30);
  quickAction1.AddPosition(98, 58, 80, 20, 10);
  quickAction1.AddPosition(98, 143, 149, 20, 20);
  quickAction1.AddPosition(98, 160, 160, 20, 10);
  quickAction1.AddPosition(98, 123, 120, 50, 10);





  // go to init position because servo could detach
  quickAction1.AddPosition(armConstraints.initPosition.pediestal,
                           armConstraints.initPosition.lower_arm,
                           armConstraints.initPosition.upper_arm,
                           armConstraints.initPosition.gripper, 20);

  // create return position action
  returnAction.AddPosition(armConstraints.initPosition.pediestal,
                           armConstraints.initPosition.lower_arm,
                           armConstraints.initPosition.upper_arm,
                           armConstraints.initPosition.gripper, 20);

  // setup init mode
  initProcess(ARM_MODE_INTRO);
}

/****************************************************************
    Main program loop function
 ****************************************************************/
void loop() {
  readLCDButtons();


  switch (ARM_MODE) {
    case ARM_MODE_SELECT:
      processModeSelection();
      break;
    case ARM_MODE_INFO:
      processInfoMode();
      break;
    case ARM_MODE_CONTROL:
      if ( readControl() ) {
        // attach servo motors
        if (CONSOLE_WARNING_SHOWN) {
          showJoystickControlMessage();
          servosAttach();
          CONSOLE_WARNING_SHOWN = false;
        }
        // show current datetime if left button clicked
        if ( CTRL_LEFT_PIN_BNT_PRESSED == 1 ) {
          lastInfoRefreshTime = 0;
          showArmPosition();
        }
        else {
          processControlValues();
        }
      } else if (!CONSOLE_WARNING_SHOWN) {
        // show warning
        servosDetach();

        lcd.setCursor(0, 0);
        lcd.print("Joystick console");
        lcd.setCursor(0, 1);
        lcd.print("not detected!");
        CONSOLE_WARNING_SHOWN = true;
      }
      break;
    case ARM_MODE_ACTION1:
      processAction1();
      break;
    case ARM_MODE_DATE_SETUP:
      processDateTimeSetup();
      break;
    case ARM_MODE_INTRO:
      processIntroMessage();
      break;
  }
}

void showJoystickControlMessage () {

  lcd.setCursor(0, 0);
  lcd.print("Joystick        ");
  lcd.setCursor(0, 1);
  lcd.print("control!        ");
  delay(100);
}

void servosAttach() {
  servoPediestal.attach(SRV_PIN_PEDIESTAL);
  servoLowerArm.attach(SRV_PIN_LOWER_ARM);
  servoUpperArm.attach(SRV_PIN_UPPER_ARM);
  servoGripper.attach(SRV_PIN_GRIPPER);
  delay(50);
}

void servosDetach() {
  // go to initial position
  currentArmPosition = armConstraints.initPosition;
  updateServoPosition();

  // got to initial poi
  servoPediestal.detach();
  servoLowerArm.detach();
  servoUpperArm.detach();
  servoGripper.detach();
  delay(50);
}

/**
    set process init variables
*/
void initProcess(int iMode) {
  lcd.clear();
  ARM_MODE_PREVIOUS = ARM_MODE;
  ARM_MODE = iMode;

  switch (iMode) {
    case ARM_MODE_SELECT:
      servosDetach();
      modeChangeTime = millis();
      ARM_MODE_CHANGE_MODE = getNextArmMode( ARM_MODE  );
      break;
    case ARM_MODE_INFO:
      servosDetach();
      delay(100);
      lastInfoRefreshTime = 0;
      break;
    case ARM_MODE_CONTROL:
      showJoystickControlMessage();
      CONSOLE_WARNING_SHOWN = false;
      servosAttach();
      break;
    case ARM_MODE_ACTION1:
      lcd.setCursor(0, 0);
      lcd.print("Quick action!");
      delay(100);

      servosAttach();
      // start from current arm position
      quickAction1.StartMovement(currentArmPosition);
      processAction1();
      break;
    case ARM_MODE_DATE_SETUP:
      servosDetach();
      DATETIME_SETUP_MODE = 0;
      break;
    case ARM_MODE_INTRO:
      servosDetach();
      lcd.setCursor(0, 0);
      lcd.print("Hello Adrien,   ");
      lcd.setCursor(0, 1);
      lcd.print("I am eDel. hand!");
      introStartTime = millis();
      break;
  }
}
/**
   Intro message check if is showing long enough
   then go to INFO mode
*/
void processIntroMessage() {
  if ( millis() - introStartTime > LCD_INTRO_INTERVAL  )  {
    introStartTime = 0;
    initProcess(ARM_MODE_INFO);
  }
}

/**
   Process mode selection  1
*/
void processModeSelection() {
  lcd.setCursor(0, 0);
  lcd.print("Set mode:       ");
  lcd.setCursor(0, 1);

  if (millis() -  modeChangeTime > MODE_CHANGE_INTERVAL) {
    ARM_MODE_CHANGE_MODE = getNextArmMode(ARM_MODE_CHANGE_MODE == ARM_MODE_NOT_DEFINED ? ARM_MODE : ARM_MODE_CHANGE_MODE );
    modeChangeTime = millis();
  }

  switch (ARM_MODE_CHANGE_MODE) {
    case ARM_MODE_INFO:
      lcd.print("Show DateTime  ");
      break;
    case ARM_MODE_CONTROL:
      lcd.print("Manual control     ");
      break;
    case ARM_MODE_ACTION1:
      lcd.print("Quick Action       ");
      break;
    case ARM_MODE_DATE_SETUP:
      lcd.print("DateTime setup  ");
    case ARM_MODE_INTRO:
      lcd.print("Intro message   ");
      break;
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
    // when end go to preious mode
    initProcess(ARM_MODE_PREVIOUS);
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
  // clear the rest of string
  lcd.print("                ");

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
}

void showArmPosition() {
  // display values to LCD
  lcd.setCursor(0, 0);
  lcd.print("PD:");
  lcd.print((int)currentArmPosition.pediestal);
  lcd.print(" LA:");
  lcd.print((int)currentArmPosition.lower_arm);
  lcd.print("              "); // clean

  lcd.setCursor(0, 1);
  lcd.print("UA:");
  lcd.print((int)currentArmPosition.upper_arm);
  lcd.print(" GRP:");
  lcd.print((int)currentArmPosition.gripper);
  lcd.print("              ");

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
    if (!BTN_LCD_LEFT_LONGPRESS_FIRED && millis() - BTN_LCD_LEFT_PRESSED  > BUTTON_LONG_CLICK) {
      lcdLeftButtonLongPressEvent();
      BTN_LCD_LEFT_LONGPRESS_FIRED = true;
    }
  } else {
    if (BTN_LCD_LEFT_PRESSED > 0) {
      if ( millis() - BTN_LCD_LEFT_PRESSED  > BUTTON_LONG_CLICK) {
        lcdLeftButtonLongClickEvent();
      } else {
        lcdLeftButtonClickEvent();
      }
    }
    BTN_LCD_LEFT_LONGPRESS_FIRED = false;
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

  CTRL_ON = iCtrLeftH > 15
            || iCtrLeftV > 15
            || iCtrRightH > 15
            || iCtrRightV > 15;


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
    // set back the message
    if (CTRL_LEFT_PIN_BNT_PRESSED == 1) {
       showJoystickControlMessage();
    }

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
void processInfoMode() {
  // Do not refresh to often because it is time and current consuming..  :)
  unsigned long currentTime = millis();
  if ( currentTime - lastInfoRefreshTime > LCD_INFO_REFRESH_INTERVAL) {
    lastInfoRefreshTime = currentTime;

    timer.updateTime();
    // print date time
    lcd.setCursor(0, 0);
    // todo fix 00 formating..
    String timeString = addLeadingZero(timer.dayofmonth);
    timeString = String(timeString + "/");
    timeString = String(timeString + addLeadingZero(timer.month));
    timeString = String(timeString + "/");
    timeString = String(timeString + timer.year);
    timeString = String(timeString + " ");
    timeString = String(timeString + addLeadingZero(timer.hours));
    timeString = String(timeString + ":");
    timeString = String(timeString + addLeadingZero(timer.minutes));


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
}

String addLeadingZero(uint8_t val){
    String str = String(val);
    while (str.length() <2){
     str = String("0" + str);
    }
    return str;
}

/**
   Left LCD button click event
*/
void lcdLeftButtonLongPressEvent() {
  // got to select mode
  initProcess(ARM_MODE_SELECT);
}

int getNextArmMode(int iCurrent) {
  switch (iCurrent) {
    case ARM_MODE_INTRO:
      return  ARM_MODE_INFO;
    case ARM_MODE_INFO:
      return  ARM_MODE_CONTROL;
    case ARM_MODE_CONTROL:
      return  ARM_MODE_DATE_SETUP;
    case ARM_MODE_DATE_SETUP:
      return ARM_MODE_INTRO;
    default:
      return  ARM_MODE_INFO;
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
    // if arm mode not in
    if (ARM_MODE != ARM_MODE_ACTION1) {
      // start quick action 1
      initProcess(ARM_MODE_ACTION1);
    }
  }
}

/**
  Left LCD button LONG click event
*/
void lcdLeftButtonLongClickEvent() {
  if (ARM_MODE_CHANGE_MODE != ARM_MODE_NOT_DEFINED) {
    initProcess(ARM_MODE_CHANGE_MODE);
    ARM_MODE_CHANGE_MODE = ARM_MODE_NOT_DEFINED;
  } else {
    initProcess(ARM_MODE_INFO);

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
  } else if (ARM_MODE == ARM_MODE_INFO) {
    initProcess(ARM_MODE_CONTROL);
  } else {
    initProcess(ARM_MODE_INFO);
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
