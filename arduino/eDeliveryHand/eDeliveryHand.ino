#include <Servo.h>
#include <DS1302.h>

// twelve servo objects can be created on most boards
/*Base Servo start*/
Servo baseServo;  // create servo object to control the base servo
int baseServoControlPin = 2;
int baseServoPos = 0;    // variable to store the new servo position
int baseServoLastPosition = 0; // variable to store the last servo position
/*Base Servo end*/

/*Business Card fetch push button start*/
int businessCardInPin = 3;
int businessCardPinValue = 0;
/*Business Card fetch push button end*/

/*Clock VMA301 start*/
namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int kCePin   = 4;  // Chip Enable = RST
const int kIoPin   = 5;  // Input/Output = DAT
const int kSclkPin = 6;  // Serial Clock - CLK

// Create a DS1302 object.
DS1302 rtc(kCePin, kIoPin, kSclkPin);

String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sunday";
    case Time::kMonday: return "Monday";
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
}

void printTime() {
  // Get the current time and date from the chip.
  Time t = rtc.time();

  // Name the day of the week.
  const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
           day.c_str(),
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);

  // Print the formatted string to serial so we can see the time.
  Serial.println(buf);
}

}  // namespace
/*Clock VMA301 end*/

void setup() {
  // put your setup code here, to run once:
  /*Hello Adrien LCD message start*/
  
  /*Hello Adrien LCD message end*/

  /*Read from RTC DS1302 start*/
  Serial.begin(9600);
  // Initialize a new chip by turning off write protection and clearing the
  // clock halt flag. These methods needn't always be called. See the DS1302
  // datasheet for details.
  rtc.writeProtect(false);
  rtc.halt(false);

  // Make a new time object to set the date and time.
  // Sunday, September 22, 2013 at 01:38:50.
  //Time t(2019, 10, 1, 00, 24, 00, Time::kTuesday);

  // Set the time and date on the chip.
  //rtc.time(t);
  /*Read from RTC DS1302 end*/
  /* Setup objects for business card fetch start */
  baseServo.attach(baseServoControlPin);  // attaches the baseServo on pin 2
  baseServo.write(0);
  pinMode(businessCardInPin, INPUT_PULLUP);    // declare business card pushbutton to read input only on input pullup transition.
  /* Setup objects for business card fetch end */
}

void loop() {
  /*Print time from RTC DS1302 start*/
  printTime();
  delay(1000);
  /*Print time from RTC DS1302 start*/

  /*Fetch BusinessCard start*/
  retrieveBusinessCard();
  /*Fetch BusinessCard end */
}

void retrieveBusinessCard() {
  /*Logic to fetch business card start*/
  businessCardPinValue = digitalRead(businessCardInPin);  // read input value
  if (businessCardPinValue == LOW) { // check if the input is HIGH (button released)

    for (baseServoPos = 0; baseServoPos <= 90; baseServoPos += 1) { // turn 90 degrees to fetch business card
      baseServo.write(baseServoPos);
      delay(30);
    }
    delay(1000);

    for (baseServoPos = 90; baseServoPos >= 0; baseServoPos -= 1) { // turn back to hand out business card
      baseServo.write(baseServoPos);
      delay(30);
    }
  }
  /*Logic to fetch business card end*/
}
