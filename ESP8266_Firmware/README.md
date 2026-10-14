# ESP8266 Fingerprint Firmware

This directory contains the ESP8266 firmware for the TouchAuth Biometric FIDO2 Security Key project. It integrates an R307 fingerprint sensor with a web-based control panel for fingerprint enrollment and management.

## Features

- Fingerprint enrollment and verification using the R307 sensor.
- Wi-Fi access point for the fingerprint control interface.
- Web-based fingerprint enrollment and deletion.
- Display of registered fingerprint IDs and system status.
- Local fingerprint template storage on the sensor.

## Hardware Components

- ESP8266 microcontroller
- R307 fingerprint sensor

## How It Works

The ESP8266 communicates with the fingerprint sensor over serial communication and hosts a local Wi-Fi access point. Users can access the web interface to enroll, delete, and manage fingerprints. During verification, the firmware checks the fingerprint and controls the relay according to the configured button logic.

## Configuration

Update the Wi-Fi access point settings, GPIO assignments, and sensor configuration in the source code according to your hardware setup. Do not publish real passwords or sensitive credentials.

## Hardware Integration

The TouchAuth prototype integrates an ESP8266, an R307 fingerprint sensor, and a Raspberry Pi Pico (RP2040) using jumper-wire connections. The ESP8266 handles fingerprint verification, while the Pico provides the USB interface. The inter-board signaling and complete authentication workflow depend on the implemented firmware and must be validated through testing.
