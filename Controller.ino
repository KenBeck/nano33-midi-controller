#include "Controller.h"

// Create controller instance
Controller controller;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  
  // Initialize the controller
  controller.begin();
  
  Serial.println(F("Nano 33 IoT MIDI Controller initialized"));
  Serial.println(F("Ready for MIDI communication"));
}

void loop() {
  // Update controller state
  controller.update();
  
  // Small delay to prevent overwhelming the USB
  delay(1);
}