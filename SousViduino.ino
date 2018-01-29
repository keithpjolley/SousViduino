
double targetTemp = 140.0; // °F -- the temperature we want to hold

// RELAY STUFF
#define RELAY 12 // relay is controlled from this pin

// Temp probe stuff
// Data wire is plugged into this pin on the Arduino
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 11
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// keypad/LCD stuff
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#define BUTTON_RIGHT  0   // these are not pins
#define BUTTON_UP     1
#define BUTTON_DOWN   2
#define BUTTON_LEFT   3
#define BUTTON_SELECT 4
#define BUTTON_NONE   5
#define DEGREE        1   // create a "degree" -> ° <- symbol
byte c_degree[8] = { B01110, B11011, B01110, B00000, B00000, B00000, B00000, B00000 };
LCDKeypad lcd;


// PID stuff
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
double actualTemp = 0.0;  // °F -- the actual temperature
double output;            // apparently something.

byte ATuneModeRemember = 2;
//double input=80, output=50, setpoint=180;
double kp = 2, ki = 0.5, kd = 2;

double kpmodel = 1.5, taup = 100, theta[50];
double outputStart = 5;
double aTuneStep = 50, aTuneNoise = 1, aTuneStartValue = 100;
unsigned int aTuneLookBack = 20;

boolean tuning = false;
unsigned long  modelTime, serialTime;
bool useSimulation = false;

PID_ATune aTune(&actualTemp, &output);
PID myPID(&actualTemp, &output, &targetTemp, kp, ki, kd, DIRECT); // see how that works?  Yeah, me neither.

void LCDshowTemp(float target, float current, bool onoff) {
  lcd.clear();
  lcd.print("Curr: ");
  lcd.print(current);
  lcd.write(DEGREE);
  lcd.print("F ");
  lcd.print(onoff ? "+" : "-");
  lcd.setCursor(0, 1);
  lcd.print("Targ: ");
  lcd.print(target);
  lcd.write(DEGREE);
  lcd.print("F");
  return;
}

void changeAutoTune() {
  if (!tuning) {
    //Set the output to the desired starting frequency.
    output = aTuneStartValue;
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

// Function that prints data from the server
void printData(char* data, int len) {
  // Print the data returned by the server
  // Note that the data is not null-terminated, may be broken up into smaller packets, and
  // includes the HTTP header.
  while (len-- > 0) {
    Serial.print(*(data++));
  }
}

float f2c(float f) {
  return ((5.0 * f - 160.0) / 9.0);
}

float c2f(float c) {
  return (c * 1.8 + 32.0);
}

void setup(void) {
  // start serial port
  Serial.begin(9600);
  Serial.println("hello world");
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
  //tell the PID to range between 0 and the full window size
  if (tuning) {
    tuning = false;
    changeAutoTune();
    tuning = true;
  }
  serialTime = 0;

  return;
}

void SerialSend() {
  Serial.print("targetTemp: "); Serial.print(targetTemp); Serial.print("°F, ");
  Serial.print("actualTemp: "); Serial.print(actualTemp); Serial.print("°F, ");
  Serial.print("output: ");     Serial.print(output);     Serial.print(", ");
  if (tuning) {
    Serial.println("[tuning mode]");
  } else {
    Serial.print("kp: "); Serial.print(myPID.GetKp()); Serial.print(", ");
    Serial.print("ki: "); Serial.print(myPID.GetKi()); Serial.print(", ");
    Serial.print("kd: "); Serial.print(myPID.GetKd()); Serial.print(", ");
  }
  Serial.print("relay: "); Serial.println(output > 0 ? "ON" : "OFF");
  return;
}

void SerialReceive() {
  if (Serial.available())  {
    char b = Serial.read();
    Serial.flush();
    if ((b == '1' && !tuning) || (b != '1' && tuning)) {
      changeAutoTune();
    }
  }
}

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

int waitButton() {
  int buttonPressed;
  waitReleaseButton;
  lcd.blink();
  while ((buttonPressed = lcd.button()) == KEYPAD_NONE)  {
  }
  delay(50);
  lcd.noBlink();
  return buttonPressed;
}

void loop(void) {

  // I kind of doubt that with these little changes, presumably small, would require re-autotuning.
  int buttonPressed;
  do {
    buttonPressed = lcd.button();
  } while (!(buttonPressed == KEYPAD_UP || buttonPressed == KEYPAD_DOWN || buttonPressed == KEYPAD_NONE));
  if (buttonPressed != KEYPAD_NONE) {
    waitReleaseButton();
  }
  if (buttonPressed == KEYPAD_UP) {
    Serial.println("Increasing targetTemp by 0.25°F");
    targetTemp += 0.25;
  }
  else if (buttonPressed == KEYPAD_DOWN)   {
    targetTemp -= 0.25;
    Serial.println("Decreasing targetTemp by 0.25°F");

  }


  unsigned long now = millis();

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  //  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //  Serial.print("DONE...   ");

  //  Serial.print("Temperature for Device 1 is: ");
  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  actualTemp = c2f(sensors.getTempCByIndex(0));

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
    myPID.Compute();
  }

  if (millis() > serialTime) {
    SerialReceive();
    SerialSend();
    serialTime += 500;
  }

  LCDshowTemp(targetTemp, actualTemp, output > 0.0);

  //  Serial.print("targetTemp: ");
  //  Serial.print(targetTemp);
  //  Serial.print("°F, actualTemp: ");
  //  Serial.print(actualTemp);
  //  Serial.print("°F, Relay: ");
  //  Serial.println(hilo ? "Off" : "On");
  digitalWrite(RELAY,  (output > 0.0) ? LOW : HIGH);

  return;
}
