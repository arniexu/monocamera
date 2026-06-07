# monocamera

Monocamera is an embedded photo capture and printing project for an mbed-compatible NXP LPC1768 board. It connects a serial camera module to an SD card, converts captured images into bitmap formats, and outputs the result to a thermal printer or e-paper display.

## Features

- Capture JPEG photos from a serial camera module
- Save photos to the SD card with incremental file names
- Convert captured images into BMP and 1-bit bitmap formats
- Print processed images with a thermal printer
- Display graphics and menus on an e-paper screen
- Use a joystick, button, and LEDs for basic device control and status feedback

## Main Hardware Components

- NXP LPC1768 / mbed target board
- OV528-compatible serial camera module
- SD card storage
- Electronic Assembly e-paper display
- Adafruit-compatible thermal printer
- Joystick, button, and onboard LEDs

## Default Runtime Flow

The main program performs the following steps:

1. Initialize the serial debug interface and joystick
2. Find the next available image file name on the SD card
3. Capture a photo and save it as `/sd/NNN.jpg`
4. Convert the image to `/sd/NNN.bmp`
5. Send the converted image to the printer
6. Update LEDs to show progress and wait before the next cycle

## Important Source Files

- `/tmp/workspace/arniexu/monocamera/main.cpp` - main device workflow
- `/tmp/workspace/arniexu/monocamera/command.cpp` - command handlers for capture, conversion, display, and printing
- `/tmp/workspace/arniexu/monocamera/command.h` - command declarations
- `/tmp/workspace/arniexu/monocamera/serial_camera/serial_camera.h` - serial camera protocol definitions

## Repository Structure

- `serial_camera/` - camera communication code
- `SimpleJpegDecode/` and `BaseJpegDecode/` - JPEG decoding support
- `BMPFile/` - BMP helpers
- `Thermal/` and `Adafruit_Thermal/` - thermal printer support
- `EaEpaper/` - e-paper display support
- `SDFileSystem/` - SD card filesystem support
- `menu/` - menu assets and related resources

## Build Notes

This repository contains mbed export files and Keil uVision project files such as `SerialCamera_DemoCode_OJ_OV528.uvproj`, `SerialCamera_DemoCode_OJ_OV528.uvopt`, and `mbed.bld`.

There is no standalone Makefile or automated test suite in the repository. To build the firmware, import the project into a compatible mbed/Keil environment for the LPC1768 target.
