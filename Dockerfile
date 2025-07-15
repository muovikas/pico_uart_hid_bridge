FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install toolchain and required build tools
RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  git \
  python3 \
  gcc-arm-none-eabi \
  libnewlib-arm-none-eabi \
  ninja-build \
  wget \
 && apt-get clean

WORKDIR /opt

# Clone SDK and TinyUSB
RUN git clone -b master https://github.com/raspberrypi/pico-sdk.git \
 && cd pico-sdk && git submodule update --init

RUN git clone https://github.com/hathach/tinyusb.git

ENV PICO_SDK_PATH=/opt/pico-sdk
ENV TINYUSB_PATH=/opt/tinyusb