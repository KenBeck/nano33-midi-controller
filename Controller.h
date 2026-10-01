/**
 * nano33-midi-controller - Controller.h
 * 
 * Hardware: Nano 33 IoT with MIDIUSB library
 * 
 * Overview:
 * This class manages all sensor inputs, generates MIDI messages, and controls
 * LED brightness feedback based on sensor values. The design uses two multiplexers
 * to maximize the number of inputs while minimizing Arduino pins.
 * 
 * Multiplexer Strategy:
 * - Both 74HC4067 and 74HC4051 share S0/S1/S2 on D2/D3/D4
 * - Sequential reading prevents cross-talk (read one mux completely before the other)
 * - 74HC4051 (8-ch) handles: 2 ribbons + 2 FSRs + 1 direct FSR
 * - 74HC44067 (16-ch) handles: 14 buttons
 * 
 * LED Feedback System:
 * - FSR values control Blue (D12) and Green (D9) LEDs
 * - Ribbon values control Red (D10) and Yellow (D11) LEDs  
 * - Higher sensor values = brighter LEDs (0-255 scale)
 * - Sensitivity multipliers can adjust response curves
 * 
 * Calibration & Sensitivity:
 * - Runtime calibration updates min/max ADC ranges
 * - Sensitivity multipliers (1.0 = normal) adjust input response
 * - Supports recalibration via public methods
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
// Potentiometer Settings (A0-A3 - direct analog reads)
// ============================================================

#define NUM_POTS 4
const uint8_t potPins[NUM_POTS] = {A0, A1, A2, A3};
const uint8_t potCC[NUM_POTS] = {1, 2, 3, 4};  // CC numbers for each pot

// ============================================================
// Direct FSR on A6
// ============================================================

#define FSR_A6_PIN A6
#define FSR_A6_CC 5  // CC number for direct FSR

// ============================================================
// Mux 2 (74HC4051 - 8-channel for ribbons & FSRs)
// ============================================================

// Pin connections: both muxes share S0-S2 on D2-D4
#define MUX2_S0 2
#define MUX2_S1 3  
#define MUX2_S2 4
#define MUX2_SIG A5  // Signal pin
#define MUX2_ENABLE 0  // E tied to GND (always enabled)

// Channel assignments (Y0-Y5):
#define MUX2_Y0 GND  // Not used (ground reference)
#define MUX2_Y1 GND  // Not used (ground reference)
#define MUX2_Y2_RIBBON1 2   // Ribbon 1 potentiometer
#define MUX2_Y3_RIBBON2 3   // Ribbon 2 potentiometer
#define MUX2_Y4_FSR1 4      // FSR 1
#define MUX2_Y5_FSR2 5      // FSR 2

// MIDI Note assignments for ribbons (send note on messages)
#define RIBBON1_NOTE 36    // C2
#define RIBBON2_NOTE 38    // D2
const uint8_t ribbonCC[2] = {6, 7};  // Optional CC for ribbon data

// ============================================================
// Mux 1 (74HC44067 - 16-channel for buttons)
// ============================================================

#define MUX1_S0 2
#define MUX1_S1 3
#define MUX1_S2 4
#define MUX1_S3 5  // Extra select pin for 16 channels
#define MUX1_SIG A4  // Signal pin
#define MUX1_ENABLE 0  // E tied to GND (always enabled)

#define NUM_MUX_BUTTONS 14  // Using C0-C13 (channels 0-13)
const uint8_t muxButtonCC[NUM_MUX_BUTTONS] = {
  10, 11, 12, 13, 14, 15, 16, 17,  // C0-C7
  18, 19, 20, 21, 22, 23, 24, 25   // C8-C13
};

// ============================================================
// Direct Button Settings (PD6, PD7, PD8 - direct digital inputs)
// ============================================================

#define NUM_DIRECT_BUTTONS 3
const uint8_t directButtonPins[NUM_DIRECT_BUTTONS] = {6, 7, 8};  // D6, D7, D8
const uint8_t directButtonCC[NUM_DIRECT_BUTTONS] = {26, 27, 28};

// ============================================================
// LED Output Configuration
// ============================================================

#define LED_RED 10      // Controls Ribbon 1 (D10)
#define LED_YELLOW 11   // Controls Ribbon 2 (D11)
#define LED_BLUE 12     // Controls FSR 1 (D12)
#define LED_GREEN 9     // Controls FSR 2 (D9)

// LED brightness range
#define LED_MIN_BRIGHTNESS 10
#define LED_MAX_BRIGHTNESS 255

// ============================================================
// Timing & Performance
// ============================================================

#define DEBOUNCE_MS 10      // Button debounce time
#define UPDATE_INTERVAL_MS 5 // Controller loop interval
#define SMOOTH_SAMPLES 5    // Number of samples for sensor averaging

// ============================================================
// ADC & Calibration
// ============================================================

#define NUM_ADC_BITS 12     // Nano 33 IoT ADC resolution
#define ADC_MAX 4095        // 12-bit max value (2^12 - 1)

// ============================================================
// Controller Class Interface
// ============================================================

class Controller {
 public:
  Controller();
  void begin();        // Initialize hardware (pins, buttons, LEDs)
  void update();       // Main loop - read sensors, send MIDI, update LEDs
  
  // Calibration and sensitivity adjustment
  void calibrateFSR(int index);          // Calibrate specific FSR
  void calibrateRibbon(int index);       // Calibrate specific ribbon
  void setFSRSensitivity(int index, float value);    // Adjust FSR response
  void setRibbonSensitivity(int index, float value); // Adjust ribbon response
  
  // Get current settings
  float getFSRSensitivity(int index) const;
  float getRibbonSensitivity(int index) const;

 private:
  // Hardware initialization
  void initMuxPins();
  void initButtons();
  void initLEDs();
  
  // Mux control and reading
  void setMuxSelect(byte s0, byte s1, byte s2, byte s3 = 255);
  int readMuxChannel(byte muxNum, byte channel);
  
  // Sensor processing
  int readSmooth(int pin, int samples = SMOOTH_SAMPLES);
  int applyCalibration(int value, int minVal, int maxVal);
  
  // MIDI communication
  void sendControlChange(byte cc, byte value);
  void sendNoteOn(byte note, byte velocity);
  void sendNoteOff(byte note);
  
  // LED feedback
  void updateLED(uint8_t ledPin, byte brightness);
  
  // Button state management (debounce)
  bool lastMuxButtonState[NUM_MUX_BUTTONS];
  bool lastDirectButtonState[NUM_DIRECT_BUTTONS];
  unsigned long lastMuxDebounceTime[NUM_MUX_BUTTONS];
  unsigned long lastDirectDebounceTime[NUM_DIRECT_BUTTONS];
  
  // Sensor calibration ranges
  int fsrMin[3];    // 3 FSRs (A6, Y4, Y5)
  int fsrMax[3];
  int ribbonMin[2]; // 2 ribbons
  int ribbonMax[2];
  
  // Sensitivity multipliers (1.0 = normal, 2.0 = 2x sensitivity, etc.)
  float fsrSensitivity[3];
  float ribbonSensitivity[2];
  
  // Current sensor values (for change detection)
  int currentPotValue[NUM_POTS];
  int currentFSRValue[3];
  int currentRibbonValue[2];
  bool currentButtonState[NUM_MUX_BUTTONS + NUM_DIRECT_BUTTONS];
  
  // Timing control
  unsigned long lastUpdate;
};

#endif // CONTROLLER_H