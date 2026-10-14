# Raspberry Pi Pico Firmware

This directory contains the firmware source files for the Raspberry Pi Pico (RP2040) component of TouchAuth, a biometric FIDO2 security key project.

Files

- `user_presence.c` – Implements GPIO-based user-presence input handling.
- `usb_descriptors.c` – Defines USB device, HID, and related interface descriptors.
- `usb_descriptors.h` – Contains USB descriptor structure definitions.
- `usb.c` – Handles USB interface initialization and communication logic.
- `usb.h` – Contains USB-related declarations, event definitions, and data structures.
- `tusb_config.h` – Configures the TinyUSB device stack.
- `pico_fido.uf2` – Precompiled firmware binary, if included.

Purpose

The Pico component provides the USB device interface for the TouchAuth project. Refer to the main project README for the overall system architecture and integration details.

**Note:** These files may depend on additional source files, libraries, and build configuration from the original firmware project.
