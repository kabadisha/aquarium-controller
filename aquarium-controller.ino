#include <LiquidCrystal.h> // https://www.arduino.cc/en/Reference/LiquidCrystal
#include <Streaming.h>     // http://arduiniana.org/libraries/streaming/
#include <DS1302RTC.h>     // http://playground.arduino.cc/Main/DS1302RTC
#include <TimeLib.h>       // http://playground.arduino.cc/Code/Time


// Setup RTC pins: 
//            CE, IO, CLK
DS1302RTC RTC(11, 10, 9);

/*----------------------------------------------------------------------*
 * Set the date and time by entering the following on the Arduino       *
 * serial monitor:                                                      *
 *    year,month,day,hour,minute,second,                                *
 *                                                                      *
 * Where                                                                *
 *    year can be two or four digits,                                   *
 *    month is 1-12,                                                    *
 *    day is 1-31,                                                      *
 *    hour is 0-23, and                                                 *
 *    minute and second are 0-59.                                       *
 *                                                                      *
 * Entering the final comma delimiter (after "second") will avoid a     *
 * one-second timeout and will allow the RTC to be set more accurately. *
 *                                                                      *
 * No validity checking is done, invalid values or incomplete syntax    *
 * in the input will result in an incorrect RTC setting.                *
 *                                                                      *
 *----------------------------------------------------------------------*/

// Initialize the display with the numbers of the interface pins:
//                RS EN D4 D5 D6 D7
LiquidCrystal lcd(6, 12, 7, 2, 3, 4);

const int LIGHTS_PWM_PIN = 5;
const int CO2_PIN = 8;
const int AIR_PIN = 13;

const int CO2_ON_HOUR = 7;
const int CO2_ON_MINUTE = 0;
const int CO2_OFF_HOUR = 21;
const int CO2_OFF_MINUTE = 0;
String CO2_STATE = "Off";

const int AIR_ON_HOUR = 22;
const int AIR_ON_MINUTE = 0;
const int AIR_OFF_HOUR = 7;
const int AIR_OFF_MINUTE = 0;
String AIR_STATE = "Off";

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

bool INITIALISED_SUCCESS = false;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  
  pinMode(LIGHTS_PWM_PIN, OUTPUT);
  digitalWrite(LIGHTS_PWM_PIN, LOW);
  pinMode(CO2_PIN, OUTPUT);
  digitalWrite(CO2_PIN, LOW);
  pinMode(AIR_PIN, OUTPUT);
  digitalWrite(AIR_PIN, LOW);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 4);
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

  delay(1000);
  
  // Print a message to the LCD.
  // Set the cursor to column 0, line 0
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  lcd.print("Clock Sync...");
  Serial << F("Clock Sync...");
  Serial << endl;
  // setSyncProvider() causes the Time library to synchronize with the
  // external RTC by calling RTC.get() every five minutes by default.
  setSyncProvider(RTC.get);

  if (timeStatus() == timeSet) {
    lcd.setCursor(0,1);
    lcd.print("Success");
    Serial << F("Success");
    Serial << endl;
    INITIALISED_SUCCESS = true;
  } else {
    Serial << F("Failed!");
    Serial << endl;
    lcd.setCursor(0,1);
    lcd.print("Failed");
  }
  delay(2000);
  lcd.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  readTimeFromSerial();
  
  if (INITIALISED_SUCCESS) {
    updateDisplay();
    handleCo2();
    handleAir();
    handleLights();
  } else {
    lcd.setCursor(0, 3);
    lcd.print("Init Failed!");
  }
  delay(1000);
}

void readTimeFromSerial() {
  static time_t tLast;
  time_t t;
  tmElements_t tm;

  //check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
  if (Serial.available() >= 12) {
      //note that the tmElements_t Year member is an offset from 1970,
      //but the RTC wants the last two digits of the calendar year.
      //use the convenience macros from Time.h to do the conversions.
      int y = Serial.parseInt();
      if (y >= 100 && y < 1000) {
          Serial << F("Error: Year must be two digits or four digits!") << endl;
      } else {
          if (y >= 1000)
              tm.Year = CalendarYrToTm(y);
          else    //(y < 100)
              tm.Year = y2kYearToTm(y);
          tm.Month = Serial.parseInt();
          tm.Day = Serial.parseInt();
          tm.Hour = Serial.parseInt();
          tm.Minute = Serial.parseInt();
          tm.Second = Serial.parseInt();
          t = makeTime(tm);
          //use the time_t value to ensure correct weekday is set
          if(RTC.set(t) == 0) { // Success
            setTime(t);
            Serial << F("RTC set to: ");
            printDateTime(t);
            Serial << endl;
          } else {
            Serial << F("RTC set failed!") << endl;
          }
          //dump any extraneous input
          while (Serial.available() > 0) Serial.read();
      }
  }
  
  t = now();
  if (t != tLast) {
      tLast = t;
      printDateTime(t);
      Serial << endl;
  }
}

//print date and time to Serial
void printDateTime(time_t t)
{
    printDate(t);
    Serial << ' ';
    printTime(t);
}

//print time to Serial
void printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//print date to Serial
void printDate(time_t t)
{
    printI00(day(t), 0);
    Serial << monthShortStr(month(t)) << _DEC(year(t));
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << _DEC(val);
    if (delim > 0) Serial << delim;
    return;
}

// Turn the CO2 on or off
void co2On(bool on) {
  if (on) {
    digitalWrite(CO2_PIN, HIGH);
    CO2_STATE = "On ";
  } else {
    digitalWrite(CO2_PIN, LOW);
    CO2_STATE = "Off";
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

// Turn the air on or off
void airOn(bool on) {
  if (on) {
    digitalWrite(AIR_PIN, HIGH);
    AIR_STATE = "On ";
  } else {
    digitalWrite(AIR_PIN, LOW);
    AIR_STATE = "Off";
  }
}

void handleAir() {
  int currentHour = hour();
    int currentMin = minute();
    if ((currentHour > AIR_ON_HOUR || (currentHour == AIR_ON_HOUR && currentMin >= AIR_ON_MINUTE))
        && (currentHour < AIR_OFF_HOUR || (currentHour == AIR_OFF_HOUR && currentMin < AIR_OFF_MINUTE))
    ) {
      airOn(true);
    } else {
      airOn(false);
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
  
  lcd.setCursor(0, 2);
  lcd.print("CO2:");
  lcd.print(CO2_STATE);
  lcd.setCursor(8, 2);
  lcd.print("Air:");
  lcd.print(AIR_STATE);
  
  lcd.setCursor(0, 3);
  lcd.print("Lights: ");
  if (currentBrightness == 255) {
    lcd.print("On ");
  } else if (currentBrightness == 0) {
    lcd.print("Off");
  } else {
    printBrightness(currentBrightness);
  }
}

// Prefixes single digits with 0
void printTwoDigits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}

// Suffixes with spaces to clear trailing characters on the display.
void printBrightness(int brightness) {
  lcd.print(brightness);
  if (brightness >= 0 && brightness < 10) {
    lcd.print("  ");
  } else if (brightness < 100) {
    lcd.print(" ");
  }
}

