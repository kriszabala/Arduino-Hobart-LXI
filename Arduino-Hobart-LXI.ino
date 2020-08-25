const int BAUD_RATE = 115200;

//Pins
const int washRelay = 4;
const int fillRelay = 5;
const int washButtonRelay = 6;
const int doorSwitchInput = 8;
const int waterHighInput = 9;
const int waterMedInput = 10;
const int washButtonInput = 11;

enum WaterLevel { WaterLevelLow, WaterLevelMedium, WaterLevelHigh };

//Global state vars
WaterLevel currentLevel = WaterLevelLow;
boolean doorIsOpen = false;

void setup() {
  delay(100);
  Serial.begin(BAUD_RATE);

  //Init Relays
  pinMode(washRelay, OUTPUT);
  pinMode(fillRelay, OUTPUT);
  pinMode(washButtonRelay, OUTPUT);
  digitalWrite(fillRelay, LOW);
  digitalWrite(washRelay, LOW);
  digitalWrite(washButtonRelay, LOW);

  //Door switch and wash buttons are triggered by grounding so inputs are pulled up.
  pinMode(doorSwitchInput, INPUT_PULLUP);
  pinMode(washButtonInput, INPUT_PULLUP);

  //Water Level Inputs
  pinMode(waterHighInput, INPUT);
  pinMode(waterMedInput, INPUT);
}

//Vars for debouncing wash button
unsigned long buttonLockout = 1000;
unsigned long washButtonTimestamp = millis();

void loop() {
  updateDoorState();
  updateWaterLevel();

  //Debounces button input so it can't be triggered again until 1000ms has elasped.
  if  (millis() - washButtonTimestamp > buttonLockout && digitalRead(washButtonInput) == LOW) {
    washButtonTimestamp = millis();
    wash();
  }

  delay(100);
}

//Determine if door is open or closed
void updateDoorState() {
  boolean thisBool = digitalRead(doorSwitchInput) == HIGH;
  if (thisBool != doorIsOpen) {
    doorIsOpen = thisBool;
    if (doorIsOpen) {
      Serial.println("Door open.");
    }
    else {
      Serial.println("Door closed.");
    }
  }
}

//Determine water level
void updateWaterLevel() {
  WaterLevel thisLevel = WaterLevelLow;

  if (digitalRead(waterHighInput) == HIGH) {
    thisLevel = WaterLevelHigh;
  }
  else if (digitalRead(waterMedInput) == HIGH) {
    thisLevel = WaterLevelMedium;
  }
  else {
    thisLevel = WaterLevelLow;
  }

  if (thisLevel != currentLevel) {
    currentLevel = thisLevel;
    switch (currentLevel) {
      case WaterLevelHigh:
        Serial.println("Water High");
        break;
      case WaterLevelMedium:
        Serial.println("Water Med");
        break;
      case WaterLevelLow:
        Serial.println("Water Low");
        break;
      default:
        Serial.println("Unknown Water Level");
        break;
    }
  }
}

void wash() {
  Serial.println("Pre Washing!!!!");
  digitalWrite(washRelay, HIGH);
  delay(10000);
  digitalWrite(washRelay, LOW);
  delay(250);

  Serial.println("Triggering regular wash");
  digitalWrite(washButtonRelay, HIGH);
  delay(250);
  digitalWrite(washButtonRelay, LOW);
}
