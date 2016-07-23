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

const int CO2_ON_HOUR = 7;
const int CO2_ON_MINUTE = 0;
const int CO2_OFF_HOUR = 21;
const int CO2_OFF_MINUTE = 30;

const int LIGHTS_ON_HOUR = 8;
const int LIGHTS_ON_MINUTE = 0;
const int LIGHTS_OFF_HOUR = 22;
const int LIGHTS_OFF_MINUTE = 30;

// This is the number of milliseconds between each increment of brightness when ramping
// E.g a value of 2000 means 2x255 = 8.5 minutes of ramp
const long MILLIS_PER_INCREMENT = 2300; // Roughly 10 minutes

// Max brightness is 255
int currentBrightness = 0;

unsigned long previousMillis = 0;

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
    Timer1.attachInterrupt(updateDisplay); // update lcd display every 1 second
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  handleCo2();
  handleLights();

  // We have to have some small delay each cycle in order to allow the interrupt to execute.
  delay(500);
}

// Turn the CO2 on or off
void co2On(bool on) {
  if (on) {
    digitalWrite(SOLENOID_PIN, HIGH);
    lcd.setCursor(0, 2);
    lcd.print("CO2: ON         ");
  } else {
    digitalWrite(SOLENOID_PIN, LOW);
    lcd.setCursor(0, 2);
    lcd.print("CO2: OFF        ");
  }
}

void handleCo2() {
  int currentHour = hour();
    int currentMin = minute();
    if ((currentHour > CO2_ON_HOUR || (currentHour == CO2_ON_HOUR && currentMin >= CO2_ON_MINUTE))
        && (currentHour < CO2_OFF_HOUR || (currentHour == CO2_OFF_HOUR && currentMin < CO2_OFF_MINUTE))
    ) {
      co2On(true);
    } else {
      co2On(false);
    }
}

void handleLights() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= MILLIS_PER_INCREMENT) {
    previousMillis = currentMillis;
  
    int currentHour = hour();
    int currentMin = minute();
    if ((currentHour > LIGHTS_ON_HOUR || (currentHour == LIGHTS_ON_HOUR && currentMin >= LIGHTS_ON_MINUTE))
        && (currentHour < LIGHTS_OFF_HOUR || (currentHour == LIGHTS_OFF_HOUR && currentMin < LIGHTS_OFF_MINUTE))
    ) {
      if (currentBrightness < 255) {
        currentBrightness++;
        analogWrite(LIGHTS_PWM_PIN, currentBrightness);
      }
    } else {
      if (currentBrightness > 0) {
        currentBrightness--;
        analogWrite(LIGHTS_PWM_PIN, currentBrightness);
      }
    }
  }
}

// Prints the date & time to the display
void updateDisplay() {
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

  lcd.setCursor(0, 3);
  if (currentBrightness == 255) {
    lcd.print("Lights: ON      ");
  } else if (currentBrightness == 0) {
    lcd.print("Lights: OFF     ");
  } else {
    lcd.print("Lights: ");
    lcd.print(currentBrightness);
    lcd.print("     ");
  }
}

// Prefixes single digits with 0
void printTwoDigits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}
