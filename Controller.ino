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