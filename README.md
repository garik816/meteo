# Meteo 2.0 — Arduino weather station

Two AVR firmwares in one branch. The original client and server branches are untouched.

## Hardware and pins

| Module | Board | Sensor / component | Pin |
| --- | --- | --- | --- |
| Outdoor | Pro Mini 3.3 V / 8 MHz | DHT22 | D6 |
| Outdoor | Pro Mini | HC-12 SET, RX, TX | D7, D8, D9 |
| Outdoor | Pro Mini | BMP280 I2C | A4/A5 |
| Outdoor | Pro Mini | Battery divider | A3 |
| Indoor | Uno 5 V | DHT11 | D7 |
| Indoor | Uno | MQ-135 analog | A3 |
| Indoor | Uno | HC-12 RX, TX | D4, D3 |
| Indoor | Uno | LCD 1602 I2C (0x27) | A4/A5 |

SoftwareSerial constructor pin order is RX, TX. Verify voltage compatibility of Uno 5 V TX with HC-12 RX; both HC-12 modules must share baud/channel/mode. Firmware assumes HC-12 9600 bps, typical FU3.

## Build

Install Python and PlatformIO, then:

    python -m pip install platformio
    pio run -e indoor_uno
    pio run -e outdoor_pro8MHz

Upload via working board bootloaders (replace COM ports):

    pio run -e indoor_uno -t upload --upload-port COM12
    pio run -e outdoor_pro8MHz -t upload --upload-port COM8

Optional PICkit2 ISP environment: outdoor_pickit2. Check programmer/fuses/bootloader before using. **Upgrade both units together**: legacy unchecksummed radio packets are intentionally incompatible.

## Improvements

- Shared bounded, checksummed CRC-8 protocol, node ID, sequence counter, fixed-point measurements.
- Nonblocking byte-oriented UART packet decoder; rejects truncated, oversized, malformed and corrupted packets.
- Receiver reports OUT LINK LOST after 180 s without valid frames.
- No heap-allocated Arduino String construction.
- LCD updates in-place rather than clearing each time.
- DHT and BMP280 validation, BMP280 retry after failure rather than infinite blocking loop.
- HC-12 mode timing, ack checking and power-down / wake sequence.
- MQ-135 shown as raw ADC, not absolute CO2 ppm.
- Native C++ protocol tests and GitHub Actions builds producing AVR firmware artifacts.

Configuration: include/meteo_config.h. See docs/hardware.md for crucial battery-divider and sensor warnings, and docs/protocol.md for wire format.

Host-side tests:

    g++ -std=c++11 -Wall -Wextra -Werror -pedantic -Iinclude tests/protocol_test.cpp -o meteo_test
    ./meteo_test

Verify actual HC-12 model, battery divider, ADC saturation and power consumption on hardware before deployment. Reflash both devices; check fresh readings, sensor failures and link-loss display after 180 seconds.
