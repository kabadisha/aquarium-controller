#include <LiquidCrystal.h> // https://www.arduino.cc/en/Reference/LiquidCrystal
#include <DS1302RTC.h>     // http://playground.arduino.cc/Main/DS1302RTC
#include <Time.h>          // http://playground.arduino.cc/Code/Time
#include <TimerOne.h>      // http://playground.arduino.cc/Code/Timer1

// Setup RTC pins: 
//            CE, IO, CLK
DS1302RTC RTC(11, 10, 9);

/*
 * If you want to set the time on the RTC you need to upload a sketch that allows you to set it via serial.
 * I couldn't include the code to do so here because the serial pins conflict with the LCD
 * Check out the 'SetSerial' example sketch included with the DS1302RTC library. That does what you need.
 */

// Initialize the display with the numbers of the interface pins:
//                RS CS D0 D1 D2 D3
LiquidCrystal lcd(6, 7, 0, 1, 2, 3);

const int LIGHTS_PWM_PIN = 5;
const int SOLENOID_PIN = 8;

void setup() {
  // put your setup code here, to run once:

  // setSyncProvider() causes the Time library to synchronize with the
  // external RTC by calling RTC.get() every five minutes by default.
  setSyncProvider(RTC.get);
  
  pinMode(LIGHTS_PWM_PIN, OUTPUT);
  digitalWrite(LIGHTS_PWM_PIN, LOW);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 4);
  lcd.clear();
  
  // Print a message to the LCD.
  // Set the cursor to column 0, line 0
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  if (RTC.haltRTC()) {
    lcd.print("Please set time!");
  } else {
    Timer1.initialize(1000000);
    Timer1.attachInterrupt(displayDateTime); // update time & date every 0.5 seconds
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(SOLENOID_PIN, LOW);
  lcd.setCursor(0, 2);
  lcd.print("CO2: OFF        ");

  int brightness = 0;
  while(brightness < 255) {
    brightness++;
    analogWrite(LIGHTS_PWM_PIN, brightness);
    delay(10);
  }

  lcd.setCursor(0, 3);
  lcd.print("LIGHTS: ON      ");

  delay(2000);

  digitalWrite(SOLENOID_PIN, HIGH);
  lcd.setCursor(0, 2);
  lcd.print("CO2: ON         ");

  while(brightness > 0) {
    brightness--;
    analogWrite(LIGHTS_PWM_PIN, brightness);
    delay(10);
  }

  analogWrite(LIGHTS_PWM_PIN, 0);

  lcd.setCursor(0, 3);
  lcd.print("LIGHTS: OFF     ");
  delay(2000);
}

// Prefixes single digits with 0
void printTwoDigits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}

void displayDateTime() {
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  printTwoDigits(day());
  lcd.print("/");
  printTwoDigits(month());
  lcd.print("/");
  lcd.print(year());

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  printTwoDigits(hour());
  lcd.print(":");
  printTwoDigits(minute());
  lcd.print(":");
  printTwoDigits(second());
}
