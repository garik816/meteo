#ifndef METEO_CONFIG_H
#define METEO_CONFIG_H

// Change only after checking actual wiring / radio settings.
#define METEO_NODE_ID 1
#define METEO_RADIO_BAUD 9600
#define METEO_DEBUG 0
#define METEO_OUTDOOR_PERIOD_MS 60000UL
#define METEO_OUTDOOR_STALE_MS 180000UL
// Legacy code assumed A3 reads 4075 mV at ADC 1023 with INTERNAL (1.1 V).
// This is a scale for the *physical resistor divider*, not a software gain.
// Verify A3 voltage <= 1.1 V at maximum battery voltage and calibrate it.
#define METEO_BATTERY_ADC_FULL_SCALE_MV 4075UL
// HC-12 FU3 normal, 300 ms settling after sending a packet.
// Increase to >= 1200 for FU4 slow mode.
#define METEO_HC12_TX_SETTLE_MS 300UL
// Heater must already be physically powered. MQ135 here is a qualitative raw ADC.
#define METEO_MQ135_WARMUP_MS 300000UL

#endif
