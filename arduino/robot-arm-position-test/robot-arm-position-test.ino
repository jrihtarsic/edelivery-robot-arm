#include <SoftwareSerial.h>

#include<Servo.h>
Servo servoFoot;
Servo servoArm;
Servo servoUpperArm;
Servo servoGrip;

int foot_val;
int foot_delta;
int foot_angle;

int arm_val;
int arm_delta;
int arm_angle;

int upperArm_val;
int upperArm_delta;
int upperArm_angle;

int grip_val;
int grip_delta;
float grip_angle;


int val;
int delta;


void setup() {

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  servoFoot.attach(8);
  servoArm.attach(9);
  servoUpperArm.attach(10);
  servoGrip.attach(11);

  foot_angle = 90;
  arm_angle = 90;
  upperArm_angle = 90;
  grip_angle = 90;

  Serial.begin(9600);

}

void loop()

{

  foot_angle = getNewValue(0, A0, foot_angle, 0, 180);
  servoFoot.write(foot_angle);

  delay(15);
  arm_angle = getNewValue(0, A1, arm_angle, 0, 180);
  servoArm.write(arm_angle);
  delay(15);

  upperArm_angle = getNewValue(1, A2, upperArm_angle, 0, 180);
  servoUpperArm.write(upperArm_angle);
  delay(15);
  grip_angle = getNewValue(0, A3, grip_angle, 0, 180);
  servoGrip.write(grip_angle);
  delay(15);

  Serial.flush();
  Serial.print("Foot: ");
  Serial.print(foot_val);
  Serial.print(" -->");
  Serial.print(foot_angle);
  Serial.print(" Arm: ");
  Serial.print(analogRead(A1));
  Serial.print(" -->");
  Serial.print(arm_angle);
  Serial.print(" upperArm: ");
  Serial.print(analogRead(A2));
  Serial.print(" -->");
  Serial.print(upperArm_angle);
  Serial.print(" Grip: ");
  Serial.print(grip_val);
  Serial.print(" -->");
  Serial.println(grip_angle);
}

int getNewValue(int offset, int inputVal, int oldVal, int minValue, int maxValue ) {
  int val  = analogRead(inputVal);
  int delta = map(val, 0, 1023, -10, 10) + offset;
  int angle  = oldVal + delta;
  return angle > maxValue ? maxValue : (angle < minValue ? minValue : angle);
}
