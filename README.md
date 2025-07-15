# Pico UART to HID bridge

This is based on the USB HID Keyboard for Pico RP2040 with TinyUSB https://github.com/rktrlng/pico_tusb_keyboard which is an adapted copy of the hid_composite example from [TinyUSB](https://github.com/hathach/tinyusb/tree/master/examples/device/hid_composite) showing how to build with TinyUSB when using the Raspberry Pi Pico SDK. This repository implements a simple UART to HID bridge for selected keys.

Above all, this includes proper USB descriptors so that the pico is recognized as a keyboard only, nothing else.

The logic is as it is to avoid key flooding which seems to be an issue with HID.

# Set up the environment

Use docker - anything else on Windows is a pain:

Install docker for windows
Build the container from Dockerfile
docker build -t pico-hid-dev .

# Building 

Start docker for Windows
Open terminal, navigate to the repo where you have cloned the repo, example: C:\Users\user\GIT\pico_uart_hid_bridge. Then run:

docker run --rm -it -v C:\Users\user\GIT\pico_uart_hid_bridge:/workspace -w /workspace pico-hid-dev bash

This will mount your current folder into the container and drop you into an interactive shell inside /workspace

cp /opt/pico-sdk/external/pico_sdk_import.cmake .
mkdir build
cd build
cmake .. -DPICO_SDK_PATH=/opt/pico-sdk
make -j4

# Installing

While holding BOOTSEL, connect RPi Pico to your PC and copy keyboard.uf2 from build/src to the pico. It will boot automatically.

# Using

Connect 3V3 TTL serial cable to GPIO 0 (TX) and GPIO 1 (RX). Connect GND as well.
Open up your favorite terminal e.g. PuTTY and set Terminal - Keyboard - The backspace key to Control-H instead of Control-?(127).
Then connect the Pi Pico to whatever device you wish to send keypresses to. It will show up as a HID keyboard.

Now you can type any key defined in the switch statement to the PuTTY Terminal and the device will receive keypresses as if received from a HID keyboard.