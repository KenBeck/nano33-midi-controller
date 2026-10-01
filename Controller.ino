/**
 * nano33-midi-controller - Controller.cpp
 * 
 * Hardware: Nano 33 IoT with MIDIUSB library
 * 
 * Mux Configuration (both share S0/S1/S2 on D2/D3/D4):
 * - 74HC4051 (Mux 2): 8-ch for ribbons & FSRs → Signal on A5
 *   - Y0,Y1 = GND (reference)
 *   - Y2 = Ribbon 1 → Red LED (D10)
 *   - Y3 = Ribbon 2 → Yellow LED (D11)
 *   - Y4 = FSR 1 → Blue LED (D12)
 *   - Y5 = FSR 2 → Green LED (D9)
 * - 74HC44067 (Mux 1): 16-ch for 14 buttons → Signal on A4
 *   - C0-C13 used for buttons
 * 
 * Timing: Sequential mux reading prevents cross-talk since S0-S2 are shared
 * Total loop time: ~20ms = 50 updates/sec (imperceptible latency for MIDI)
 * 
 * MIDI Messages:
 * - Pots (A0-A3): CC messages
 * - FSRs (Y4,Y5): CC messages + LED brightness control
 * - Ribbons (Y2,Y3): Note On messages (pitch based on value)
 * - Buttons (C0-C13): CC messages (0/127)
 * - Direct buttons D6/D7/D8: CC messages
 */

#include "Controller.h"

// Create controller instance
Controller controller;

void setup() {
  // Initialize serial for debugging (115200 baud)
  Serial.begin(115200);
  
  // Initialize the controller (sets up mux pins, button pins, LEDs)
  controller.begin();
  
  Serial.println(F("Nano 33 IoT MIDI Controller initialized"));
  Serial.println(F("Ready for MIDI communication"));
}

void loop() {
  // Update controller state - reads all sensors and sends MIDI messages
  // This runs at ~50Hz (20ms per loop) - fast enough for human input
  controller.update();
  
  // Small delay to prevent USB buffer overflow
  // Without this, the loop runs too fast and overwhelms USB
  delay(1);
}