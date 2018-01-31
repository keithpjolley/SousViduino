//
// keith p jolley
// squalor heights, jamul, ca, usa
// Mon Jan 29 04:30:46 PST 2018
//
// an arduino sous vide setup. there should be a readme with this.
//

double targetTemp = 140.0; // °F -- the temperature we want to hold

// define this if you want data being sent to the serial port
#undef _SERIAL_OUT_



// Temp probe stuff
// Data wire is plugged into this pin on the Arduino
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 11             // this goes to the thermometer.
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// RELAY STUFF
#define RELAY 12 // relay is controlled from this pin

// keypad/LCD stuff
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#define DEGREE        1   // create a "degree" -> ° <- symbol
byte c_degree[8] = { B01110, B11011, B01110, B00000, B00000, B00000, B00000, B00000 };
LCDKeypad lcd;

// PID stuff -- much of it cut & paste from the PID_AutoTune example.
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
double aTuneOutputStartVal = 5.0;    // this is a number with a purpose and units. it does something.
double actualTemp = targetTemp;      // °F -- the actual temperature. set it to something reasonable.
double output = aTuneOutputStartVal; // apparently this is something and has units too. good luck!
byte ATuneModeRemember = 2;
double kp = 2, ki = 0.5, kd = 2;
double aTuneStep = 50.0, aTuneNoise = 1.0;
unsigned int aTuneLookBack = 20;     // how much history to remember.
boolean tuning = false;
unsigned long serialTime = 0;

PID_ATune aTune(&actualTemp, &output);
PID myPID(&actualTemp, &output, &targetTemp, kp, ki, kd, DIRECT); // see how that works?  Yeah, me neither.

// show something of interest on the device' LCD
void LCDshowTemp(float target, float current, bool onoff) {
  lcd.clear();
  lcd.print("Curr: ");
  lcd.print(current);
  lcd.write(DEGREE);
  lcd.print("F ");
  lcd.print(onoff ? "+" : "-");  // <- toggle this to show if the relay is on or off
  lcd.setCursor(0, 1);
  lcd.print("Targ: ");
  lcd.print(target);
  lcd.write(DEGREE);
  lcd.print("F");
  return;
}

// more cut & paste code
void changeAutoTune() {
  if (!tuning) {
    //Set the output to the desired starting frequency.
    output = aTuneOutputStartVal;
    aTune.SetNoiseBand(aTuneNoise);
    aTune.SetOutputStep(aTuneStep);
    aTune.SetLookbackSec((int)aTuneLookBack);
    AutoTuneHelper(true);
    tuning = true;
  } else {
    //cancel autotune
    aTune.Cancel();
    tuning = false;
    AutoTuneHelper(false);
  }
}

// farenheit to celsius  (not used)
float f2c(float f) {
  return ((5.0 * f - 160.0) / 9.0);
}

// the "thermometer" i'm using happens to give its readings in °C
// but i am more familiar with °F. you do what works for you.
// btw - i worked out these equations without looking them up.
// pen and paper for the win. :)
// celsius to farenheit
float c2f(float c) {
  return (c * 1.8 + 32.0);
}

void setup(void) {
  // start serial port and show we are live.
#ifdef _SERIAL_OUT_
  Serial.begin(9600);
  Serial.println("hello world");
#endif
  sensors.begin();
  lcd.createChar(DEGREE, c_degree);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Welcome to");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print("THE FUTURE!");
  digitalWrite(RELAY, LOW);
  pinMode(RELAY, OUTPUT); // ONEWIRE already init'd in globals.

  myPID.SetMode(AUTOMATIC);
  return;
}

// all this serial output is more for troubleshooting than anything. not needed.
void SerialSend() {
#ifdef _SERIAL_OUT_
  Serial.print("targetTemp: "); Serial.print(targetTemp); Serial.print("°F, ");
  Serial.print("actualTemp: "); Serial.print(actualTemp); Serial.print("°F, ");
  Serial.print("output: ");     Serial.print(output);     Serial.print(", ");
  if (tuning) {
    Serial.print("[tuning]");
  } else {
    Serial.print("kp: "); Serial.print(myPID.GetKp()); Serial.print(", ");
    Serial.print("ki: "); Serial.print(myPID.GetKi()); Serial.print(", ");
    Serial.print("kd: "); Serial.print(myPID.GetKd()); Serial.print(", ");
  }
  Serial.print("relay: "); Serial.println(output > 0 ? "ON" : "OFF");
#endif
  return;
}

//// i expect no input
void SerialReceive() {
#ifdef _SERIAL_OUT_
  if (Serial.available())  {
    char b = Serial.read();
    Serial.flush();
    if ((b == '1' && !tuning) || (b != '1' && tuning)) {
      changeAutoTune();
    }
  }
#endif
  return;
}

// this does something.
void AutoTuneHelper(boolean start) {
  if (start) {
    ATuneModeRemember = myPID.GetMode();
  } else {
    myPID.SetMode(ATuneModeRemember);
  }
}

void waitReleaseButton() {
  delay(50);
  while (lcd.button() != KEYPAD_NONE)  {
  }
  delay(50);
}

void loop(void) {

  // i kind of doubt that with these little changes, presumably small,
  // would require re-autotuning.  this button code doesn't work as
  // smoothly as i'd like but i don't think i'll use it often enough
  // to warrant improving it.  it allows you to adjust the temp "in
  // flight." a reset will bring the targetTemp back to the value
  // hardcoded above.

  int buttonPressed = lcd.button();
  if (buttonPressed == KEYPAD_UP) {
#ifdef _SERIAL_OUT_
    Serial.println("Increasing targetTemp by 0.25°F");
#endif
    targetTemp += 0.25;
    delay(200); // to prevent massive changes
  }
  else if (buttonPressed == KEYPAD_DOWN) {
    targetTemp -= 0.25;
#ifdef _SERIAL_OUT_
    Serial.println("Decreasing targetTemp by 0.25°F");
#endif
    delay(200); // to prevent massive changes
  }

  // now is the time for all good men...
  unsigned long now = millis();

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  actualTemp = c2f(sensors.getTempCByIndex(0));  // convert temp to °F

  if (tuning) {
    byte val = aTune.Runtime();
    tuning = (val == 0);
    if (!tuning) {
      kp = aTune.GetKp();
      ki = aTune.GetKi();
      kd = aTune.GetKd();
      myPID.SetTunings(kp, ki, kd);
      AutoTuneHelper(false);
    }
  } else {
    myPID.Compute();  // this updates globals.
  }

  // update the timers for the PID
  if (millis() > serialTime) {
    SerialReceive();
    SerialSend();
    serialTime += 500;
  }

  LCDshowTemp(targetTemp, actualTemp, output > 0.0); // update the LCD

  digitalWrite(RELAY, (output > 0.0) ? LOW : HIGH); // Turn on/off the relay

  return;
}
