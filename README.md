# MIDI Controller for Nano 33 IoT

A USB MIDI controller using the MIDIUSB library with configurable sensors, buttons, and LED feedback.

## Hardware Configuration

### Pin Connections
- **4 Pots**: A0, A1, A2, A3 (direct analog input)
- **16-channel Mux (74HC44067)**: 14 buttons on mux channel C0-C15
- **3 Buttons**: D6 (PD6), D7 (PD7), D8 (PD8) - direct digital input
- **FSR**: A6 (single FSR on analog input)
- **8-channel Mux (74HC4051)**: 2 FSRs + 2 ribbon pots

#### 74HC4051 Mux (Mix 2) Connections
- Y0, Y1 = GND (reference)
- Y2 = Ribbon 1 potentiometer
- Y3 = Ribbon 2 potentiometer
- Y4 = FSR 1
- Y5 = FSR 2

### LED Outputs
- **Red LED**: D10
- **Yellow LED**: D11
- **Blue LED**: D12
- **Green LED**: D9

### LED Control Mapping
- Ribbon 1 (Y2) → Red LED (D10)
- Ribbon 2 (Y3) → Yellow LED (D11)
- FSR 1 (Y4) → Blue LED (D12)
- FSR 2 (Y5) → Green LED (D9)

## MIDI Message Types
- **Pots (A0-A3)**: MIDI Control Change (CC) messages
- **FSRs (Y4-Y5)**: MIDI CC messages
- **Buttons**: MIDI CC messages
- **Ribbons (Y2-Y3)**: MIDI Note messages

## LED Brightness Control
- FSRs: Higher CC value → brighter LED
- Ribbons: Higher pitch/note → brighter LED

## Features
- Software-configurable MIDI channels, CC numbers, and note numbers
- Adjustable sensitivity/thresholds for FSRs and ribbon sensors
- USB MIDI support via MIDIUSB library