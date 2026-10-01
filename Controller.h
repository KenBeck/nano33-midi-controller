/**
 * Controller.h - Header file for Nano 33 IoT MIDI Controller
 * 
 * Hardware Configuration (based on Sensor and Mux pin files):
 * - Potentiometers: A0, A1, A2, A3 (direct analog read)
 * - 74HC4051 (8-ch) Mux 2 (ribbons & FSRs):
 *   - S0=D2, S1=D3, S2=D4, Z=A5, E=GND (always enabled)
 *   - Y0=GND, Y1=GND, Y2=Ribbon1, Y3=Ribbon2, Y4=FSR1, Y5=FSR2
 * - 74HC44067 (16-ch) Mux 1 (buttons):  
 *   - S0=D2, S1=D3, S2=D4, S3=D5, Signal=A4
 *   - C0-C13 = 14 buttons
 * - FSR on A6 (direct read)
 * - 3 Direct buttons on D6, D7, D8 (PD6, PD7, PD8)
 * 
 * LED Mapping:
 * - Ribbon 1 (Y2) -> Red LED (D10)
 * - Ribbon 2 (Y3) -> Yellow LED (D11)
 * - FSR 1 (Y4) -> Blue LED (D12)
 * - FSR 2 (Y5) -> Green LED (D9)
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <MIDIUSB.h>

// ============================================================
// MIDI Configuration
// ============================================================

#define MIDI_CHANNEL 0  // MIDI channel (0 = channel 1)

// ============================================================
// Potentiometer Settings (A0-A3 direct)
// ============================================================

#define NUM_POTS 4
const uint8_t potPins[NUM_POTS] = {A0, A1, A2, A3};
const uint8_t potCC[NUM_POTS] = {1, 2, 3, 4};

// ============================================================
// Direct FSR on A6
// ============================================================

#define FSR_A6_PIN A6
#define FSR_A6_CC 5

// ============================================================
// Mux 2 (74HC4051 - 8-ch for ribbons and FSRs)
// ============================================================

#define MUX2_S0 2
#define MUX2_S1 3  
#define MUX2_S2 4
#define MUX2_SIG A5
#define MUX2_ENABLE 0  // E tied to GND (always enabled)

// Mux channel assignments (Y0-Y5)
#define MUX2_Y2_RIBBON1 2
#define MUX2_Y3_RIBBON2 3
#define MUX2_Y4_FSR1 4
#define MUX2_Y5_FSR2 5

// Ribbon settings - send MIDI Note messages
#define RIBBON1_NOTE 36
#define RIBBON2_NOTE 38
const uint8_t ribbonCC[2] = {6, 7};  // Optional CC for additional data

// ============================================================
// Mux 1 (74HC44067 - 16-ch for buttons)
// ============================================================

#define MUX1_S0 2
#define MUX1_S1 3
#define MUX1_S2 4
#define MUX1_S3 5
#define MUX1_SIG A4
#define MUX1_ENABLE 0  // E tied to GND (always enabled)

#define NUM_MUX_BUTTONS 14
const uint8_t muxButtonCC[NUM_MUX_BUTTONS] = {
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
};

// ============================================================
// Direct Button Settings (PD6, PD7, PD8)
// ============================================================

#define NUM_DIRECT_BUTTONS 3
const uint8_t directButtonPins[NUM_DIRECT_BUTTONS] = {6, 7, 8};  // D6, D7, D8
const uint8_t directButtonCC[NUM_DIRECT_BUTTONS] = {24, 25, 26};

// ============================================================
// LED Settings
// ============================================================

#define LED_RED 10
#define LED_YELLOW 11
#define LED_BLUE 12
#define LED_GREEN 9

// ============================================================
// Timing and Debounce
// ============================================================

#define DEBOUNCE_MS 10
#define UPDATE_INTERVAL_MS 5

// ============================================================
// Calibration Settings
// ============================================================

#define NUM_ADC_BITS 12  // Nano 33 IoT has 12-bit ADC
#define ADC_MAX 4095     // 12-bit max value

// ============================================================
// Class Definition
// ============================================================

class Controller {
 public:
  Controller();
  void begin();
  void update();
  
  // Public methods for calibration and sensitivity adjustment
  void calibrateFSR(int index);
  void setFSRSensitivity(int index, float value);
  float getFSRSensitivity(int index) const;
  
  void calibrateRibbon(int index);
  void setRibbonSensitivity(int index, float value);
  float getRibbonSensitivity(int index) const;

 private:
  // Hardware initialization
  void initMuxPins();
  void initButtons();
  void initLEDs();
  
  // Mux select line control
  void setMuxSelect(byte s0, byte s1, byte s2, byte s3 = 255);
  int readMuxChannel(byte muxNum, byte channel);
  
  // Sensor reading helpers
  int readSmooth(int pin, int samples = 5);
  int applyCalibration(int value, int minVal, int maxVal);
  
  // MIDI message helpers
  void sendControlChange(byte cc, byte value);
  void sendNoteOn(byte note, byte velocity);
  void sendNoteOff(byte note);
  
  // LED control
  void updateLED(uint8_t ledPin, byte brightness);
  
  // Button state tracking
  bool lastMuxButtonState[NUM_MUX_BUTTONS];
  bool lastDirectButtonState[NUM_DIRECT_BUTTONS];
  unsigned long lastMuxDebounceTime[NUM_MUX_BUTTONS];
  unsigned long lastDirectDebounceTime[NUM_DIRECT_BUTTONS];
  
  // Calibration storage
  int fsrMin[3];    // 3 FSRs (A6, Y4, Y5)
  int fsrMax[3];
  int ribbonMin[2]; // 2 ribbons
  int ribbonMax[2];
  
  // Sensitivity multipliers (1.0 = normal)
  float fsrSensitivity[3];
  float ribbonSensitivity[2];
  
  // Current state
  int currentPotValue[NUM_POTS];
  int currentFSRValue[3];
  int currentRibbonValue[2];
  bool currentButtonState[NUM_MUX_BUTTONS + NUM_DIRECT_BUTTONS];
  
  unsigned long lastUpdate;
};

#endif // CONTROLLER_H