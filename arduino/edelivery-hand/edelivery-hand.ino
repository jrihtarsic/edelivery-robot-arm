// include the library code:
#include <¸.h>
#include "DHT.h"
#include <virtuabotixRTC.h> //Libraries needed


// lcd 
const uint8_t LCD_PIN_DB7= 13;
const uint8_t LCD_PIN_DB6= 12;
const uint8_t LCD_PIN_DB5= 11;
const uint8_t LCD_PIN_DB4= 10;
const uint8_t LCD_PIN_E  =  9;
const uint8_t LCD_PIN_RS =  8;
// DHT11 temperature and humidity
const int DHT11_PIN_S =  0;
// RTC 
const uint8_t RTC_PIN_CLK = 1;
const uint8_t RTC_PIN_DAT = 2;
const uint8_t RTC_PIN_RST = 3;

// LCD Buttons
const uint8_t  BTN_LCD_PIN_RIGHT = A0;
const uint8_t  BTN_LCD_PIN_LEFT = A1;


// LCD Controls
const uint8_t  CTRL_LEFT_PIN_BNT = A2;
const uint8_t  CTRL_LEFT_PIN_H = A3;
const uint8_t  CTRL_LEFT_PIN_V = A4;
const uint8_t  CTRL_RIGHT_PIN_BNT = A5;
const uint8_t  CTRL_RIGHT_PIN_H = A6;
const uint8_t  CTRL_RIGHT_PIN_V = A7;


// status
int BTN_LCD_LEFT_PRESSED=0;
int BTN_LCD_RIGHT_PRESSED=0;
// control status
int CTRL_LEFT_PIN_BNT_PRESSED=0;
int CTRL_RIGHT_PIN_BNT_PRESSED=0;
int CTRL_LEFT_PIN_H_ACTION=0;
int CTRL_LEFT_PIN_V_ACTION=0;
int CTRL_RIGHT_PIN_H_ACTION=0;
int CTRL_RIGHT_PIN_V_ACTION=0;



// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_E, LCD_PIN_DB4, LCD_PIN_DB5,LCD_PIN_DB6, LCD_PIN_DB7);
// initialize libary for DHT11
DHT dht(DHT11_PIN_S, DHT11);
// initialize real time Clock DS 1302
virtuabotixRTC timer(RTC_PIN_CLK, RTC_PIN_DAT, RTC_PIN_RST); 


// DISPLAY REFRESH
unsigned long lastRefresh= 0;
long LCD_REFRESH_INTERVAL = 1000;  



void setup() {
  // start DHT
  dht.begin();
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
 // lcd.print("eDelivery hand!");

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  // this is for initial set.. 
  //timer.setDS1302Time(00, 30, 20, 1, 6, 10, 2019);

  

   pinMode(BTN_LCD_PIN_LEFT, INPUT_PULLUP);
   pinMode(BTN_LCD_PIN_RIGHT, INPUT_PULLUP); 

   pinMode(CTRL_LEFT_PIN_BNT, INPUT_PULLUP); 
   pinMode(CTRL_RIGHT_PIN_BNT, INPUT_PULLUP); 
  
}

void loop() {

  unsigned long currentTime = millis();
   if(showDisplay() && currentTime - lastRefresh > LCD_REFRESH_INTERVAL) {
    lastRefresh = currentTime;
    refreshDisplay();
   }

  
  // READ BUTTONS  
  if(digitalRead(BTN_LCD_PIN_LEFT) == LOW) {
    BTN_LCD_LEFT_PRESSED = 1;
    lcd.setCursor(0, 0);
    lcd.print("LCD BTN LEFT            ");
  } else {
    BTN_LCD_LEFT_PRESSED = 0;
  }

  if(digitalRead(BTN_LCD_PIN_RIGHT) == LOW) {
    BTN_LCD_RIGHT_PRESSED = 1;
    lcd.setCursor(0, 0);
    lcd.print("LCD BTN RIGHT            ");
  } else {
    BTN_LCD_RIGHT_PRESSED = 0;
  }

  if(digitalRead(CTRL_LEFT_PIN_BNT) == LOW) {
    CTRL_LEFT_PIN_BNT_PRESSED = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL BTN LEFT            ");
  } else {
    CTRL_LEFT_PIN_BNT_PRESSED = 0;
  }

  if(digitalRead(CTRL_RIGHT_PIN_BNT) == LOW) {
    CTRL_RIGHT_PIN_BNT_PRESSED = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL BTN RIGHT            ");
  } else {
    CTRL_RIGHT_PIN_BNT_PRESSED = 0;
  }

  // this is just for test 
  // use values to control servo
  if(analogRead(CTRL_LEFT_PIN_H) > 800) {
    CTRL_LEFT_PIN_H_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL LEFT H-UP            ");
  }  else if(analogRead(CTRL_LEFT_PIN_H) < 300) {
    CTRL_LEFT_PIN_H_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL LEFT H-DOWN            ");
  }  else {
    CTRL_LEFT_PIN_H_ACTION = 0;
  } 

  if(analogRead(CTRL_LEFT_PIN_V) < 300) {
    CTRL_LEFT_PIN_V_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL LEFT V-LEFT            ");
  }  else if(analogRead(CTRL_LEFT_PIN_V) > 800) {
    CTRL_LEFT_PIN_V_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL LEFT V-RIGHT            ");
  }  else {
    CTRL_LEFT_PIN_V_ACTION = 0;
  } 
  // RIGHT
  if(analogRead(CTRL_RIGHT_PIN_H) > 800) {
    CTRL_RIGHT_PIN_H_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL RIGHT H-UP            ");
  }  else if(analogRead(CTRL_RIGHT_PIN_H) < 300) {
    CTRL_RIGHT_PIN_H_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL RIGHT H-DOWN            ");
  }  else {
    CTRL_RIGHT_PIN_H_ACTION = 0;
  } 

  if(analogRead(CTRL_RIGHT_PIN_V) < 300) {
    CTRL_RIGHT_PIN_V_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL RIGHT V-LEFT            ");
  }  else if(analogRead(CTRL_RIGHT_PIN_V) > 800) {
    CTRL_RIGHT_PIN_V_ACTION = 1;
    lcd.setCursor(0, 0);
    lcd.print("CTRL RIGHT V-RIGHT            ");
  }  else {
    CTRL_RIGHT_PIN_V_ACTION = 0;
  } 
  
}

void refreshDisplay() {
  // clear LCD
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

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
 while(timeString.length() <16){
   timeString = String(timeString+" ");
 } 
 lcd.print(timeString);


  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (!isnan(t) && !isnan(h)) {
    int it1 = t *10;
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
  delay(1000);
}

boolean showDisplay(){
  return BTN_LCD_LEFT_PRESSED==0 
      && BTN_LCD_RIGHT_PRESSED==0
       && CTRL_LEFT_PIN_BNT_PRESSED==0
        && CTRL_RIGHT_PIN_BNT_PRESSED==0
        && CTRL_LEFT_PIN_H_ACTION ==0 
        && CTRL_LEFT_PIN_V_ACTION ==0
        && CTRL_RIGHT_PIN_H_ACTION ==0 
        && CTRL_RIGHT_PIN_V_ACTION ==0;

}
