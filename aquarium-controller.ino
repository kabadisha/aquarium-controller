#include <LiquidCrystal.h>

#include <DS1302RTC.h>
#include <Time.h>

// Setup RTC pins: CE, IO, CLK
DS1302RTC RTC(11, 10, 9);

// initialize the display with the numbers of the interface pins
//                RS  CS D0 D1 D2 D3
LiquidCrystal lcd(6, 7, 0, 1, 2, 3);

const int LIGHTS_PWM_PIN = 5;
const int SOLENOID_PIN = 8;

void setup() {
  // put your setup code here, to run once:
  pinMode(LIGHTS_PWM_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 4);
  lcd.clear();
  
  // Print a message to the LCD.
  // set the cursor to column 0, line 0
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  if (RTC.haltRTC()) {
    lcd.print("Please set time!");
  }
  lcd.setCursor(0, 1);
  lcd.print("hello, world2!");
  lcd.setCursor(0, 2);
  lcd.print("hello, world3!");
  lcd.setCursor(0, 3);
  lcd.print("hello, world4!");

  //setSyncProvider() causes the Time library to synchronize with the
  //external RTC by calling RTC.get() every five minutes by default.
  setSyncProvider(RTC.get);
}

void loop() {
  // put your main code here, to run repeatedly:
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
