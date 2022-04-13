// Interactive Dog Sculpture by Minh Pham
// 
// Music self-transcribed to Arduino
// Ode to joy, Symphony no9 by Beethoven self-recreated
// Marriage of Figaro by Mozart, score from https://musescore.com/user/8095736/scores/5516259
// Notes frequency from https://github.com/robsoncouto/arduino-songs

#include <NewPing.h>

#define E3  165
#define F3  175
#define FS3 185
#define G3  196
#define GS3 208
#define A3  220
#define AS3 233
#define B3  247
#define C4  262
#define CS4 277
#define D4  294
#define DS4 311
#define E4  330
#define F4  349
#define FS4 370
#define G4  392
#define GS4 415
#define A4  440
#define AS4 466
#define B4  494
#define C5  523
#define CS5 554
#define D5  587
#define DS5 622
#define E5  659
#define F5  698
#define FS5 740
#define G5  784
#define GS5 831
#define A5  880
#define AS5 932
#define B5  988
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define REST 0

// pins for ultrasonic sensors
int tp0 = 22; // tp for trigger pin
int ep0 = 23; // ep for echo pin
int tp1 = 26;
int ep1 = 27;
int tp2 = 30;
int ep2 = 31;
int maxDist = 20;

int const numSonars = 3; // number of ultra sonic sensors
long sensors[numSonars]; // array to store distance detected just now
long lastsensors[numSonars] = {0, 0, 0}; // array to store distance detected the iteration before this

// initialize ultrasonic sensors using the NewPing library
NewPing sonar[numSonars] = {
  NewPing(tp0, ep0, maxDist),
  NewPing(tp1, ep1, maxDist),
  NewPing(tp2, ep2, maxDist)
};

// ultrasonic sensors work every 100 millisecs
int ultraSenseInterval = 100;

// pin for sound sensor
int soundSensor = A4;

// pin for buzzer
int buzzer = 4;

// pins for shift registers
int latchPin = 8;
int clockPin = 12;
int dataPin = 11;

// creates continuous interactions of blinking and sensors
int currentMillis;
int lastBlink = 0;
int lastUltraSense = 0;

// sound sensor works every 100 millisecs
int soundSenseInterval = 100;

// NEUTRAL STATE
int blinkNeutralInterval = 1000;
int lightsStateNeutral = 0;

// general state
int state = 0;
// 0 for neutral
// 1 for happy 1
// 2 for happy 2
// 3 for aggressive 1
// 4 for aggressive 2

// HAPPY STATE
// length of happy period, and keep track of when to stop with continuous interaction
int happyPeriod = 4900;
int lastHappy = 0;

// count increases through each positive interaction from human
int count = 0;

// as count reachs a certain threshold, dog is happy
// threshold starts with a number, 
// reduces by 1 for each time dog turns happy
// increases by 1 for each time dog is scared
// this allows evolution
int thresholdHappy = 7;

// this allows continuous interaction between lights and everything else
int blinkHappyInterval1 = 100;
int blinkHappyInterval2 = 80;
int lightsStateHappy = 0;

// this allows continuous interaction between buzzer and everything else
int buzzHappyInterval = 150;
int isBuzzingHappy = 0;
int lastBuzzed = 0;

// this state updates to allow buzzer make different frequency
int buzzHappyState = 0;
int maxBuzzHappyState = 34;

// AGRESSIVE STATE
int aggressivePeriod = 4000;
int lastAggressive = 0;

// this allows continuous interaction between lights and everything else
int blinkAggressiveIntervalOn1 = 1000;
int blinkAggressiveIntervalOff1 = 300;
int blinkAggressiveIntervalConsistent = 400;
int lightsOn = 0;

// this allows continuous interaction between buzzer and everything else
int buzzAggressiveInterval = 400;
int isBuzzingAggressive = 0;

void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.begin(9600);
}


void loop() {
  currentMillis = millis();
  if (state == 0) {
    // resets a few variables
    buzzHappyState = 0;
    lastBuzzed = 0;
    noTone(buzzer);

    if (count > thresholdHappy) {
      isBuzzingHappy = 0;
      count = 0;
      if (thresholdHappy > 2) { // dog becomes more friendly
        thresholdHappy--;
      }
      state = random(1, 3);
      lastHappy = currentMillis;
      for (int i = 0; i < numSonars; i++) {
        lastsensors[i] = 0;
      }
      lightsStateHappy = 0;
    }
    if (currentMillis - lastUltraSense > ultraSenseInterval) {
      lastUltraSense = currentMillis;
      for (uint8_t i = 0; i < numSonars; i++) { // Loop through each sensor
        delay(30);
        sensors[i] = sonar[i].ping_cm(); 
        if (sensors[i] > 0 && sensors[i] < maxDist) {
          // happy if the touch is gentle
          if (lastsensors[i] - sensors[i] >= -1 && lastsensors[i] - sensors[i] < 8 && sensors[i] < 10) {
            count++;
          } 
          // aggressive if the touch is sudden
          else if (lastsensors[i] - sensors[i] >= 8) {
            state = random(3, 5);
            lastAggressive = currentMillis;
            isBuzzingAggressive = 0;
            count = 0;
            if (thresholdHappy < 11) { // dog becomes more aggresive
              thresholdHappy++;
            }
            for (int i = 0; i < numSonars; i++) {
              lastsensors[i] = 0;
            }
            return;
          } else {
            count = 0;
          }
          lastsensors[i] = sensors[i];
          return;
        }
        
      }
      lastUltraSense = currentMillis;
    }
    if (analogRead(soundSensor) > 700) {
      state = random(3, 5);
      lightsOn = 1;
      isBuzzingAggressive = 0;
      lastAggressive = currentMillis;
      if (thresholdHappy < 11) { // dog becomes more aggresive
        thresholdHappy++;
      }
      return;
    }
    neutral();
  }

  if (state == 1) { // happy version 1
    if (currentMillis - lastHappy < happyPeriod) {
      happy1();
    } else {
      state = 0;
      return;
    }
  }

  if (state == 2) { // happy version 2
    if (currentMillis - lastHappy < happyPeriod) {
      happy2();
    } else {
      state = 0;
      return;
    }
  }

  if (state == 3) { // aggressive version 1
    if (currentMillis - lastAggressive < aggressivePeriod) {
      aggressive1();
    } else {
      state = 0;
      return;
    }
  }

  if (state == 4) { // aggressive version 2
    if (currentMillis - lastAggressive < aggressivePeriod) {
      aggressive2();
    } else {
      state = 0;
      return;
    }
  }
}

// neutral state
void neutral() {
  noTone(buzzer);
  currentMillis = millis();
  if (currentMillis - lastBlink > blinkNeutralInterval) {
    lastBlink = currentMillis;
    if (lightsStateNeutral == 0) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10010010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10010010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10010010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10010010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10010010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10010010);
      digitalWrite(latchPin, HIGH);
      lightsStateNeutral = 1;
    } else if (lightsStateNeutral == 1) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01001001);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01001001);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01001001);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01001001);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01001001);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01001001);
      digitalWrite(latchPin, HIGH);
      lightsStateNeutral = 2;
    } else if (lightsStateNeutral == 2) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10100100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10100100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10100100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10100100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10100100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10100100);
      digitalWrite(latchPin, HIGH);
      lightsStateNeutral = 0;
    }
  }
}

// happy state version 1
void happy1() {
  currentMillis = millis();
  // sound
  if (currentMillis - lastBuzzed > buzzHappyInterval) {
    lastBuzzed = currentMillis;
    buzzHappyState++;
    if (buzzHappyState == maxBuzzHappyState) { // to wrap circular
      buzzHappyState = 0;
    }
  }
  if (buzzHappyState == 0) {
    noTone(buzzer);
  } else if (buzzHappyState == 1) {
    tone(buzzer, E5, 100);
  } else if (buzzHappyState == 2) {
    noTone(buzzer);
  } else if (buzzHappyState == 3) {
    tone(buzzer, E5, 100);
  } else if (buzzHappyState == 4) {
    noTone(buzzer);
  } else if (buzzHappyState == 5) {
    tone(buzzer, F5, 100);
  } else if (buzzHappyState == 6) {
    noTone(buzzer);
  } else if (buzzHappyState == 7) {
    tone(buzzer, G5, 100);
  } else if (buzzHappyState == 8) {
    noTone(buzzer);
  } else if (buzzHappyState == 9) {
    tone(buzzer, G5, 100);
  } else if (buzzHappyState == 10) {
    noTone(buzzer);
  } else if (buzzHappyState == 11) {
    tone(buzzer, F5, 100);
  } else if (buzzHappyState == 12) {
    noTone(buzzer);
  } else if (buzzHappyState == 13) {
    tone(buzzer, E5, 100);
  } else if (buzzHappyState == 14) {
    noTone(buzzer);
  } else if (buzzHappyState == 15) {
    tone(buzzer, D5, 100);
  } else if (buzzHappyState == 16) {
    noTone(buzzer);
  } else if (buzzHappyState == 17) {
    tone(buzzer, C5, 100);
  } else if (buzzHappyState == 18) {
    noTone(buzzer);
  } else if (buzzHappyState == 19) {
    tone(buzzer, C5, 100);
  } else if (buzzHappyState == 20) {
    noTone(buzzer);
  } else if (buzzHappyState == 21) {
    tone(buzzer, D5, 100);
  } else if (buzzHappyState == 22) {
    noTone(buzzer);
  } else if (buzzHappyState == 23) {
    tone(buzzer, E5, 100);
  } else if (buzzHappyState == 24) {
    noTone(buzzer);
  } else if (buzzHappyState == 25) {
    tone(buzzer, D5, 100);
  } else if (buzzHappyState == 26) {
    tone(buzzer, D5, 100);
  } else if (buzzHappyState == 27) {
    tone(buzzer, D5, 50);
  } else if (buzzHappyState == 28) {
    noTone(buzzer);
  } else if (buzzHappyState == 29) {
    tone(buzzer, C5, 50);
  } else if (buzzHappyState == 30) {
    noTone(buzzer);
  } else if (buzzHappyState == 31) {
    tone(buzzer, C5, 50);
  } 
  // lights
  if (currentMillis - lastBlink > blinkHappyInterval1) {
    lastBlink = currentMillis;
    if (lightsStateHappy == 0) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 1;
    } else if (lightsStateHappy == 1) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11110000);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 2;
    } else if (lightsStateHappy == 2) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001111);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 3;
    } else if (lightsStateHappy == 3) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 0;
    }
  }
}

// happy state version 2
void happy2() {
  currentMillis = millis();
  // sound
  if (currentMillis - lastBuzzed > buzzHappyInterval) {
    lastBuzzed = currentMillis;
    buzzHappyState++;
    if (buzzHappyState == maxBuzzHappyState) { // to wrap circular
      buzzHappyState = 0;
    }
  }
  if (buzzHappyState == 0) {
    tone(buzzer, D6, 100);
  } else if (buzzHappyState == 1) {
    tone(buzzer, D6, 100);
  } else if (buzzHappyState == 2) {
    tone(buzzer, D6, 100);
  } else if (buzzHappyState == 3) {
    tone(buzzer, D6, 100);
  } else if (buzzHappyState == 4) {
    tone(buzzer, D6, 100);
  } else if (buzzHappyState == 5) {
    noTone(buzzer);
  } else if (buzzHappyState == 6) {
    tone(buzzer, CS6, 100);
  } else if (buzzHappyState == 7) {
    tone(buzzer, CS6, 100);
  } else if (buzzHappyState == 8) {
    tone(buzzer, CS6, 100);
  } else if (buzzHappyState == 9) {
    noTone(buzzer);
  } else if (buzzHappyState == 10) {
    tone(buzzer, B5, 100);
  } else if (buzzHappyState == 11) {
    noTone(buzzer);
  } else if (buzzHappyState == 12) {
    tone(buzzer, A5, 100);
  } else if (buzzHappyState == 13) {
    noTone(buzzer);
  } else if (buzzHappyState == 14) {
    tone(buzzer, D6, 100);
  } else if (buzzHappyState == 15) {
    noTone(buzzer);
  } else if (buzzHappyState == 16) {
    tone(buzzer, CS6, 100);
  } else if (buzzHappyState == 17) {
    noTone(buzzer);
  } else if (buzzHappyState == 18) {
    tone(buzzer, B5, 100);
  } else if (buzzHappyState == 19) {
    noTone(buzzer);
  } else if (buzzHappyState == 20) {
    tone(buzzer, A5, 100);
  } else if (buzzHappyState == 21) {
    tone(buzzer, A5, 100);
  } else if (buzzHappyState == 22) {
    noTone(buzzer);
  } else if (buzzHappyState == 23) {
    tone(buzzer, G5, 100);
  } else if (buzzHappyState == 24) {
    tone(buzzer, G5, 100);
  } else if (buzzHappyState == 25) {
    noTone(buzzer);
  } else if (buzzHappyState == 26) {
    tone(buzzer, FS5, 100);
  } else if (buzzHappyState == 27) {
    tone(buzzer, FS5, 100);
  } else if (buzzHappyState == 28) {
    noTone(buzzer);
  } else if (buzzHappyState == 29) {
    tone(buzzer, E5, 100);
  } else if (buzzHappyState == 30) {
    tone(buzzer, E5, 100);
  } else if (buzzHappyState == 31) {
    noTone(buzzer);
  } else if (buzzHappyState == 32) {
    tone(buzzer, D5, 100);
  } else if (buzzHappyState == 33) {
    tone(buzzer, D5, 100);
  }
  // lights
  if (currentMillis - lastBlink > blinkHappyInterval2) {
    lastBlink = currentMillis;
    if (lightsStateHappy == 0) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11000000);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 1;
    } else if (lightsStateHappy == 1) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00110000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00110000);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 2;
    } else if (lightsStateHappy == 2) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 3;
    } else if (lightsStateHappy == 3) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001100);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00001100);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 4;
    } else if (lightsStateHappy == 4) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000011);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000011);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000011);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000011);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000011);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000011);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 5;
    } else if (lightsStateHappy == 5) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 6;
    } else if (lightsStateHappy == 6) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 7;
    } else if (lightsStateHappy == 7) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 8;
    } else if (lightsStateHappy == 8) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      digitalWrite(latchPin, HIGH);
      lightsStateHappy = 0;
    }
  } 
}

// aggressive state version 1
void aggressive1() {
  currentMillis = millis();
  // sound
  tone(buzzer, 150, 100);
  // lights
  if (lightsOn == 1) {
    if (currentMillis - lastBlink < blinkAggressiveIntervalOn1) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      digitalWrite(latchPin, HIGH);
    } else {
      lastBlink = currentMillis;
      lightsOn = 0;
    }
  } else {
    if (currentMillis - lastBlink < blinkAggressiveIntervalOff1) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      digitalWrite(latchPin, HIGH);
    } else {
      lastBlink = currentMillis;
      lightsOn = 1;
    }
  }
}

// aggressive state version 1
void aggressive2() {
  currentMillis = millis();
  // sound
  if (currentMillis - lastBuzzed > buzzAggressiveInterval) {
    lastBuzzed = currentMillis;
    isBuzzingAggressive = 1 - isBuzzingAggressive; // if 1, become 0; if 0, become 1
  }
  if (isBuzzingAggressive) {
    tone(buzzer, 1100, 100);
  } else {
    noTone(buzzer);
  }
  // lights
  if (lightsOn == 1) {
    if (currentMillis - lastBlink < blinkAggressiveIntervalConsistent) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b11111111);
      digitalWrite(latchPin, HIGH);
    } else {
      lastBlink = currentMillis;
      lightsOn = 0;
    }
  } else {
    if (currentMillis - lastBlink < blinkAggressiveIntervalConsistent) {
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
      digitalWrite(latchPin, HIGH);
    } else {
      lastBlink = currentMillis;
      lightsOn = 1;
    }
  }
}
