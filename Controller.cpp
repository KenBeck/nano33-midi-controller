#include "Controller.h"

// ============================================================
// Constructor
// ============================================================

Controller::Controller() {
  // Initialize calibration ranges
  for (int i = 0; i < 3; i++) {
    fsrMin[i] = ADC_MAX;
    fsrMax[i] = 0;
  }
  for (int i = 0; i < 2; i++) {
    ribbonMin[i] = ADC_MAX;
    ribbonMax[i] = 0;
  }
  
  // Initialize sensitivity (1.0 = normal)
  for (int i = 0; i < 3; i++) {
    fsrSensitivity[i] = 1.0;
  }
  for (int i = 0; i < 2; i++) {
    ribbonSensitivity[i] = 1.0;
  }
  
  // Initialize button states
  for (int i = 0; i < NUM_MUX_BUTTONS; i++) {
    lastMuxButtonState[i] = false;
    lastMuxDebounceTime[i] = 0;
  }
  for (int i = 0; i < NUM_DIRECT_BUTTONS; i++) {
    lastDirectButtonState[i] = false;
    lastDirectDebounceTime[i] = 0;
  }
  
  lastUpdate = 0;
}

// ============================================================
// Public Methods
// ============================================================

void Controller::begin() {
  // Initialize mux select pins as outputs
  initMuxPins();
  
  // Initialize button pins (direct buttons)
  initButtons();
  
  // Initialize LED pins
  initLEDs();
}

void Controller::update() {
  unsigned long now = millis();
  
  // Only update at regular intervals
  if (now - lastUpdate < UPDATE_INTERVAL_MS) {
    return;
  }
  lastUpdate = now;
  
  // Read and send potentiometer values (A0-A3)
  for (int i = 0; i < NUM_POTS; i++) {
    int value = readSmooth(potPins[i]);
    if (value != currentPotValue[i]) {
      currentPotValue[i] = value;
      byte midiVal = map(value, 0, ADC_MAX, 0, 127);
      sendControlChange(potCC[i], midiVal);
    }
  }
  
  // Read direct FSR on A6
  int fsrA6 = readSmooth(FSR_A6_PIN);
  fsrA6 = applyCalibration(fsrA6, fsrMin[0], fsrMax[0]);
  byte fsrA6Mapped = map(fsrA6, 0, ADC_MAX, 0, 127);
  if (fsrA6Mapped != currentFSRValue[0]) {
    currentFSRValue[0] = fsrA6Mapped;
    sendControlChange(FSR_A6_CC, fsrA6Mapped);
    updateLED(LED_RED, fsrA6Mapped);  // FSR A6 controls Red LED
  }
  
  // Read Mux 2 (74HC4051) - Ribbons and FSRs
  // Select Y2 (Ribbon 1)
  setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2);
  delayMicroseconds(10);  // Allow mux to settle
  int ribbon1 = readSmooth(MUX2_SIG);
  ribbon1 = applyCalibration(ribbon1, ribbonMin[0], ribbonMax[0]);
  byte ribbon1Mapped = map(ribbon1, 0, ADC_MAX, 0, 127);
  
  // Select Y3 (Ribbon 2)
  setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2, 0);  // Only use S0-S2 for 4051
  delayMicroseconds(10);
  int ribbon2 = readSmooth(MUX2_SIG);
  ribbon2 = applyCalibration(ribbon2, ribbonMin[1], ribbonMax[1]);
  byte ribbon2Mapped = map(ribbon2, 0, ADC_MAX, 0, 127);
  
  // Select Y4 (FSR 1)
  setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2, 1);
  delayMicroseconds(10);
  int fsr1 = readSmooth(MUX2_SIG);
  fsr1 = applyCalibration(fsr1, fsrMin[1], fsrMax[1]);
  byte fsr1Mapped = map(fsr1, 0, ADC_MAX, 0, 127);
  
  // Select Y5 (FSR 2)
  setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2, 1);
  delayMicroseconds(10);
  int fsr2 = readSmooth(MUX2_SIG);
  fsr2 = applyCalibration(fsr2, fsrMin[2], fsrMax[2]);
  byte fsr2Mapped = map(fsr2, 0, ADC_MAX, 0, 127);
  
  // Send ribbon MIDI notes (pitch based on value)
  // Higher value = higher note/velocity
  if (ribbon1Mapped != currentRibbonValue[0]) {
    currentRibbonValue[0] = ribbon1Mapped;
    sendNoteOn(RIBBON1_NOTE, ribbon1Mapped);
    updateLED(LED_BLUE, ribbon1Mapped);  // Ribbon 1 controls Blue LED
  }
  
  if (ribbon2Mapped != currentRibbonValue[1]) {
    currentRibbonValue[1] = ribbon2Mapped;
    sendNoteOn(RIBBON2_NOTE, ribbon2Mapped);
    updateLED(LED_GREEN, ribbon2Mapped);  // Ribbon 2 controls Green LED
  }
  
  // Send FSR MIDI CC values
  if (fsr1Mapped != currentFSRValue[1]) {
    currentFSRValue[1] = fsr1Mapped;
    sendControlChange(ribbonCC[0], fsr1Mapped);
    updateLED(LED_BLUE, fsr1Mapped);  // FSR 1 controls Blue LED
  }
  
  if (fsr2Mapped != currentFSRValue[2]) {
    currentFSRValue[2] = fsr2Mapped;
    sendControlChange(ribbonCC[1], fsr2Mapped);
    updateLED(LED_GREEN, fsr2Mapped);  // FSR 2 controls Green LED
  }
  
  // Read Mux 1 (74HC44067) - Buttons
  for (int i = 0; i < NUM_MUX_BUTTONS; i++) {
    setMuxSelect(MUX1_S0, MUX1_S1, MUX1_S2, (i >> 3) & 0x01);
    delayMicroseconds(10);
    int buttonVal = readSmooth(MUX1_SIG);
    bool buttonPressed = (buttonVal > (ADC_MAX / 2));  // Simple threshold
    
    // Debounce and detect rising edge
    if (buttonPressed != lastMuxButtonState[i]) {
      lastMuxDebounceTime[i] = now;
    }
    
    if ((now - lastMuxDebounceTime[i] > DEBOUNCE_MS) && 
        (buttonPressed != lastMuxButtonState[i])) {
      lastMuxButtonState[i] = buttonPressed;
      if (buttonPressed) {
        sendControlChange(muxButtonCC[i], 127);
      } else {
        sendControlChange(muxButtonCC[i], 0);
      }
    }
  }
  
  // Read direct buttons (D6, D7, D8)
  for (int i = 0; i < NUM_DIRECT_BUTTONS; i++) {
    bool buttonPressed = !digitalRead(directButtonPins[i]);  // Active LOW
    
    if (buttonPressed != lastDirectButtonState[i]) {
      lastDirectDebounceTime[i] = now;
    }
    
    if ((now - lastDirectDebounceTime[i] > DEBOUNCE_MS) && 
        (buttonPressed != lastDirectButtonState[i])) {
      lastDirectButtonState[i] = buttonPressed;
      if (buttonPressed) {
        sendControlChange(directButtonCC[i], 127);
      } else {
        sendControlChange(directButtonCC[i], 0);
      }
    }
  }
}

// ============================================================
// Calibration Methods
// ============================================================

void Controller::calibrateFSR(int index) {
  if (index >= 0 && index < 3) {
    fsrMin[index] = ADC_MAX;
    fsrMax[index] = 0;
    
    // Read samples to calibrate
    for (int i = 0; i < 100; i++) {
      int val;
      if (index == 0) {
        val = analogRead(FSR_A6_PIN);
      } else if (index == 1) {
        setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2, 1);
        delayMicroseconds(10);
        val = analogRead(MUX2_SIG);
      } else {
        setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2, 1);
        delayMicroseconds(10);
        val = analogRead(MUX2_SIG);
      }
      
      if (val < fsrMin[index]) fsrMin[index] = val;
      if (val > fsrMax[index]) fsrMax[index] = val;
      delay(10);
    }
  }
}

void Controller::calibrateRibbon(int index) {
  if (index >= 0 && index < 2) {
    ribbonMin[index] = ADC_MAX;
    ribbonMax[index] = 0;
    
    // Read samples to calibrate
    for (int i = 0; i < 100; i++) {
      int val;
      if (index == 0) {
        setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2);
        delayMicroseconds(10);
        val = analogRead(MUX2_SIG);
      } else {
        setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2, 1);
        delayMicroseconds(10);
        val = analogRead(MUX2_SIG);
      }
      
      if (val < ribbonMin[index]) ribbonMin[index] = val;
      if (val > ribbonMax[index]) ribbonMax[index] = val;
      delay(10);
    }
  }
}

void Controller::setFSRSensitivity(int index, float value) {
  if (index >= 0 && index < 3 && value > 0.0) {
    fsrSensitivity[index] = value;
  }
}

float Controller::getFSRSensitivity(int index) const {
  if (index >= 0 && index < 3) {
    return fsrSensitivity[index];
  }
  return 1.0;
}

void Controller::setRibbonSensitivity(int index, float value) {
  if (index >= 0 && index < 2 && value > 0.0) {
    ribbonSensitivity[index] = value;
  }
}

float Controller::getRibbonSensitivity(int index) const {
  if (index >= 0 && index < 2) {
    return ribbonSensitivity[index];
  }
  return 1.0;
}

// ============================================================
// Private Methods
// ============================================================

void Controller::initMuxPins() {
  pinMode(MUX2_S0, OUTPUT);
  pinMode(MUX2_S1, OUTPUT);
  pinMode(MUX2_S2, OUTPUT);
  pinMode(MUX1_S0, OUTPUT);
  pinMode(MUX1_S1, OUTPUT);
  pinMode(MUX1_S2, OUTPUT);
  pinMode(MUX1_S3, OUTPUT);
}

void Controller::initButtons() {
  for (int i = 0; i < NUM_DIRECT_BUTTONS; i++) {
    pinMode(directButtonPins[i], INPUT_PULLUP);
  }
}

void Controller::initLEDs() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Turn off LEDs initially
  analogWrite(LED_RED, 0);
  analogWrite(LED_YELLOW, 0);
  analogWrite(LED_BLUE, 0);
  analogWrite(LED_GREEN, 0);
}

void Controller::setMuxSelect(byte s0, byte s1, byte s2, byte s3) {
  // Note: Both muxes share S0, S1, S2 on D2, D3, D4
  // For Mux 1 (4067), we also set S3 on D5
  
  digitalWrite(MUX2_S0, s0 & 0x01);
  digitalWrite(MUX2_S1, s1 & 0x01);
  digitalWrite(MUX2_S2, s2 & 0x01);
  
  if (s3 != 255) {
    digitalWrite(MUX1_S0, s0 & 0x01);
    digitalWrite(MUX1_S1, s1 & 0x01);
    digitalWrite(MUX1_S2, s2 & 0x01);
    digitalWrite(MUX1_S3, s3 & 0x01);
  }
}

int Controller::readMuxChannel(byte muxNum, byte channel) {
  // For Mux 2 (74HC4051) - uses 3 select lines
  if (muxNum == 2) {
    setMuxSelect(MUX2_S0, MUX2_S1, MUX2_S2);
    delayMicroseconds(10);
    return analogRead(MUX2_SIG);
  }
  
  // For Mux 1 (74HC44067) - uses 4 select lines
  if (muxNum == 1) {
    byte s0 = channel & 0x01;
    byte s1 = (channel >> 1) & 0x01;
    byte s2 = (channel >> 2) & 0x01;
    byte s3 = (channel >> 3) & 0x01;
    setMuxSelect(s0, s1, s2, s3);
    delayMicroseconds(10);
    return analogRead(MUX1_SIG);
  }
  
  return 0;
}

int Controller::readSmooth(int pin, int samples) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  return (int)(sum / samples);
}

int Controller::applyCalibration(int value, int minVal, int maxVal) {
  if (maxVal > minVal) {
    // Scale to full ADC range
    return map(constrain(value, minVal, maxVal), minVal, maxVal, 0, ADC_MAX);
  }
  return value;
}

void Controller::sendControlChange(byte cc, byte value) {
  midiEventPacket_t event;
  event.header = 0x0;
  event.byte1 = 0xB0 | MIDI_CHANNEL;  // CC message on channel
  event.byte2 = cc;
  event.byte3 = value;
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void Controller::sendNoteOn(byte note, byte velocity) {
  midiEventPacket_t event;
  event.header = 0x0;
  event.byte1 = 0x90 | MIDI_CHANNEL;  // Note On on channel
  event.byte2 = note;
  event.byte3 = velocity;
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void Controller::sendNoteOff(byte note) {
  midiEventPacket_t event;
  event.header = 0x0;
  event.byte1 = 0x80 | MIDI_CHANNEL;  // Note Off on channel
  event.byte2 = note;
  event.byte3 = 0;
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void Controller::updateLED(uint8_t ledPin, byte brightness) {
  // Apply sensitivity if needed
  int adjusted = (int)(brightness * fsrSensitivity[0]);  // Using FSR sensitivity for now
  adjusted = constrain(adjusted, LED_MIN_BRIGHTNESS, LED_MAX_BRIGHTNESS);
  analogWrite(ledPin, (byte)adjusted);
}