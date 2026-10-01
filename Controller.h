/**
 * Controller.h - Header file for Nano 33 IoT MIDI Controller
 * 
 * Defines the Controller class that manages all sensor inputs,
 * MIDI message generation, and LED output control.
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <MIDIUSB.h>

// ============================================================
// Configuration - Adjustable Parameters
// ============================================================

// MIDI Channel (1-16, 0 = Omni)
#define MIDI_CHANNEL 1

// Potentiometer settings
#define NUM_POTS 4
const int potPins[NUM_POTS] = {A0, A1, A2, A3};

// Potentiometer CC numbers (one per pot)
#define POT0_CC 1
#define POT1_CC 2
#define POT2_CC 3
#define POT3_CC 4

// FSR settings
#define NUM_FSRs 2
const int fsrPins[NUM_FSRs] = {A6, A6}; // A6 and shared A6 from mux

// FSR CC numbers
#define FSR0_CC 5
#define FSR1_CC 6

// Button settings
#define NUM_BUTTONS 14  // From 74HC44067 mux channels C0-C15
// Buttons on direct pins
#define NUM_DIRECT_BUTTONS 3
const int directButtonPins[NUM_DIRECT_BUTTONS] = {6, 7, 8};  // D6, D7, D8

// Button CC numbers - direct buttons start after mux buttons
#define BUTTON_CC_BASE 10

// Ribbon potentiometer settings
#define NUM_RIBBONS 2
const int ribbonPins[NUM_RIBBONS] = {A6, A6}; // Y2 and Y3 from mux - will use different analog reads

// Ribbon MIDI Note numbers
#define RIBBON0_NOTE 36  // C2
#define RIBBON1_NOTE 38  // D2

// ============================================================
// LED Settings
// ============================================================

// LED pins
#define LED_RED 10
#define LED_YELLOW 11
#define LED_BLUE 12
#define LED_GREEN 9

// LED brightness control
// FSR LED mapping:
// - FSR 0 → Red LED (D10)
// - FSR 1 → Yellow LED (D11)
// - Ribbon 1 → Blue LED (D12)
// - Ribbon 2 → Green LED (D9)

// LED sensitivity thresholds
#define LED_MIN_BRIGHTNESS 0
#define LED_MAX_BRIGHTNESS 255

// ============================================================
// Class Definition
// ============================================================

class Controller {
 public:
  Controller();
  void begin();
  void update();

 private:
  // Sensor reading helpers
  int readPot(int index);
  int readFSR(int index);
  int readMuxButton(byte muxChannel);
  int readDirectButton(int index);
  
  // MIDI message generation
  void sendPotMIDI(int index, int value);
  void sendFSRMIDI(int index, int value);
  void sendButtonMIDI(int index, bool pressed);
  void sendRibbonMIDI(int index, int value);
  
  // LED control
  void updateLED(int ledPin, int brightness);
  
  // Calibration
  void calibrateSensors();
  
  // Raw sensor values for calibration
  int potMin[NUM_POTS];
  int potMax[NUM_POTS];
  int fsrMin[NUM_FSRs];
  int fsrMax[NUM_FSRs];
  int ribbonMin[NUM_RIBBONS];
  int ribbonMax[NUM_RIBBONS];
};

#endif // CONTROLLER_H