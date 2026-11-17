# Meteo 2.0 hardware notes

## Battery measurement

The original code used the INTERNAL 1.1 V ADC reference and assumed an ADC full-scale equivalent of 4075 mV at battery input. Meteo 2.0 retains this as a configurable parameter, with averaging of eight samples. **Verify the real resistor divider and calibrate against a DMM.** If the actual divider saturates above 4.075 V, software cannot measure a full 4.2 V Li-ion cell correctly. Do not connect the cell directly to A3; always ensure A3 stays below the actual ADC reference.

## MQ-135 limitations

MQ-135 is a broad mixed-gas sensor, **not a selective CO2 detector**. Old startup calibration and absolute ppm display were misleading. New firmware displays only an uncalibrated relative 0..1023 ADC value after five minutes of warm-up suppression. This delay does not guarantee full sensor warm-up or calibration, and the reading is not an air quality index or safety metric. For genuine CO2 concentration use an NDIR sensor such as SCD40/SCD41.

## HC-12 and energy

The radio implementation assumes HC-12 because original firmware uses SET and AT+SLEEP. SET LOW/HIGH transitions now have 100/120 ms guard times, with command acknowledgements and wake/sleep handling. Increase METEO_HC12_TX_SETTLE_MS in slow HC-12 modes. Measure actual deep-sleep power consumption of the **entire outdoor board**, including the regulator, LEDs, DHT, BMP280 and radio. Verify voltage levels on Uno-to-radio UART.

## Field checklist

- Compare displayed battery voltage against a DMM from low voltage to fully charged; verify no saturation.
- Test HC-12 recovery from sleep and after power cycling.
- Disconnect each DHT/BMP280 sensor and verify recovery on reconnection.
- Block radio messages for 180 seconds; verify OUT LINK LOST.
- Measure whole-system transmit and sleep current.
