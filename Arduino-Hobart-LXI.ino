//Pins
const int washRelay = 4;
const int fillRelay = 5;
const int washButtonRelay = 6;
const int doorSwitchInput = 8;
const int waterHighInput = 9;
const int waterMedInput = 10;
const int washButtonInput = 11;
const int rinseInput = 12;

enum WaterLevel { WaterLevelLow, WaterLevelMedium, WaterLevelHigh };
enum CycleStage { CycleStageIdle, CycleStageFill, CycleStagePreWash, CycleStageWash, CycleStageRinse };

//Global state vars
WaterLevel currentLevel = WaterLevelLow;
CycleStage currentStage = CycleStageIdle;
boolean doorIsOpen = false;

//Config
const int BAUD_RATE = 115200;
unsigned long prewashDuration = 1000 * 10;
//Disable washButton for 3 minutes upon power up.
unsigned long initLockoutDuration = 1000 * 180;

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
  pinMode(rinseInput, INPUT);
}

//Vars for debouncing wash button
unsigned long buttonLockout = 1000;
//Lockout wash button for configured duration.
unsigned long washButtonTimestamp = millis() + initLockoutDuration;
//Tracks the time prewash was started.
unsigned long preWashTimestamp = 0;

void loop() {
  updateDoorState();
  updateWaterLevel();

  if (digitalRead(washButtonInput) == LOW) {
    if (doorIsOpen) {
      Serial.println("Wash button ignored because door is open.");
    }
    //Debounces button input so it can't be triggered again until 1000ms has elasped.
    else if (millis() - washButtonTimestamp < buttonLockout) {
      Serial.println("Wash button locked out.");
    }
    else {
      washButtonTimestamp = millis();
      washButtonPressed();
    }
  }

  switch (currentStage) {
    case CycleStageIdle:
      digitalWrite(fillRelay, LOW);
      digitalWrite(washRelay, LOW);
      break;
    case CycleStageFill:
      if (safetyCheck()) {
        //Turn on the fill relay until water level is high. Then progress to the prewash cycle.
        digitalWrite(fillRelay, HIGH);
        digitalWrite(washRelay, LOW);
        if (currentLevel == WaterLevelHigh) {
          Serial.println("Finished pre-fill. Starting pre-wash");
          startPreWash();
        }
      }
      break;
    case CycleStagePreWash:
      if (safetyCheck()) {
        //Turn on the wash relay until elapsed time > prewashDuration.
        digitalWrite(fillRelay, LOW);
        digitalWrite(washRelay, HIGH);
        if (millis() > preWashTimestamp + prewashDuration) {
          startWash();
        }
      }
      break;
    case CycleStageWash:
      //No safety checks because factory cycle enforces its own safety checks.
      digitalWrite(fillRelay, LOW);
      digitalWrite(washRelay, LOW);
      //Check to see if the rinse relay is activated to determine if wash cycle is finished.
      if (digitalRead(rinseInput) == HIGH) {
        Serial.println("Detected rinse started");
        currentStage = CycleStageRinse;
      }
      break;
    case CycleStageRinse:
      //No safety checks because factory cycle enforces its own safety checks.
      digitalWrite(fillRelay, LOW);
      digitalWrite(washRelay, LOW);
      //Check to see if rinse relay is turned off, signaling the end of the full cycle.
      if (digitalRead(rinseInput) == LOW) {
        //Delay to lockout wash button to match machine cycle reset time.
        delay(2000);
        Serial.println("Full cycle finished");
        currentStage = CycleStageIdle;
      }
      break;
    default:
      Serial.println("Error: Unknown Cycle Stage");
      break;
  }
}

//Returns true if safe to proceed. Otherwise, current cycle stage is reverted back to CycleStageIdle which should immediately cease all actions.
boolean safetyCheck() {
  //If door is opened, immediately cancel the cycle
  if (doorIsOpen) {
    Serial.println("Door was opened, reset to idle state.");
    currentStage = CycleStageIdle;
    return false;
  }
  return true;
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

void washButtonPressed() {
  //If not idle, do nothing
  if (currentStage != CycleStageIdle) {
    Serial.println("Wash button ignored because not idle.");
    return;
  }

  //Check to see if sufficient water level. If not high, first pre-fill. Otherwise, proceed with prewash.
  if (currentLevel != WaterLevelHigh) {
    Serial.println("Water level too low. Starting pre-fill");
    currentStage = CycleStageFill;
  }
  else {
    Serial.println("Starting pre-wash");
    startPreWash();
  }
}

void startPreWash() {
  preWashTimestamp = millis();
  currentStage = CycleStagePreWash;
}

void startWash() {
  Serial.println("Triggering regular wash");
  digitalWrite(washButtonRelay, HIGH);
  delay(250);
  digitalWrite(washButtonRelay, LOW);
  currentStage = CycleStageWash;
}
