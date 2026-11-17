#ifndef METEO_PROTOCOL_H
#define METEO_PROTOCOL_H

// Meteo v2 wire protocol. Header-only, Arduino-independent, no heap allocation.
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace meteo {

static const size_t kMaxFrameLength = 64;

struct Telemetry {
  uint8_t node_id;
  uint16_t sequence;
  int16_t temperature_x10_c;
  uint16_t humidity_x10_percent;
  uint16_t pressure_x10_mmhg;
  uint16_t battery_mv;
};

inline bool valid(const Telemetry& p) {
  return p.node_id >= 1 && p.node_id <= 99 &&
         p.temperature_x10_c >= -400 && p.temperature_x10_c <= 850 &&
         p.humidity_x10_percent <= 1000 &&
         p.pressure_x10_mmhg >= 3000 && p.pressure_x10_mmhg <= 9000 &&
         p.battery_mv >= 1800 && p.battery_mv <= 6000;
}

inline uint8_t crc8(const char* data, size_t length) {
  uint8_t crc = 0;
  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07)
                         : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

// $M2,node,sequence,temp_x10,rh_x10,pressure_x10,battery_mV*CRC8\n
inline bool encode(const Telemetry& p, char* out, size_t capacity) {
  if (!out || !valid(p) || capacity < 32) return false;
  const int length = snprintf(out, capacity, "$M2,%u,%u,%d,%u,%u,%u",
                              static_cast<unsigned>(p.node_id),
                              static_cast<unsigned>(p.sequence),
                              static_cast<int>(p.temperature_x10_c),
                              static_cast<unsigned>(p.humidity_x10_percent),
                              static_cast<unsigned>(p.pressure_x10_mmhg),
                              static_cast<unsigned>(p.battery_mv));
  if (length < 0 || static_cast<size_t>(length) + 5 > capacity) return false;
  const uint8_t checksum = crc8(out + 1, static_cast<size_t>(length) - 1);
  return snprintf(out + length, capacity - static_cast<size_t>(length),
                  "*%02X\n", static_cast<unsigned>(checksum)) == 4;
}

inline int hexNibble(char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  return -1;
}

inline bool decode(const char* frame, Telemetry& out) {
  if (!frame || strncmp(frame, "$M2,", 4) != 0) return false;
  const size_t length = strlen(frame);
  if (length < 20 || length >= kMaxFrameLength) return false;
  const char* asterisk = strchr(frame + 4, '*');
  if (!asterisk || asterisk[1] == '\0' || asterisk[2] == '\0' ||
      asterisk[3] != '\0') return false;
  const int hi = hexNibble(asterisk[1]);
  const int lo = hexNibble(asterisk[2]);
  if (hi < 0 || lo < 0) return false;
  if (crc8(frame + 1, static_cast<size_t>(asterisk - frame - 1)) !=
      static_cast<uint8_t>((hi << 4) | lo)) return false;

  long values[6];
  const char* cursor = frame + 4;
  for (uint8_t i = 0; i < 6; ++i) {
    if (*cursor == '\0' || *cursor == ',' || *cursor == '*') return false;
    char* end = NULL;
    values[i] = strtol(cursor, &end, 10);
    if (end == cursor) return false;
    if (i < 5) {
      if (*end != ',') return false;
      cursor = end + 1;
    } else if (end != asterisk) {
      return false;
    }
  }
  if (values[0] < 1 || values[0] > 99 ||
      values[1] < 0 || values[1] > 65535 ||
      values[2] < -400 || values[2] > 850 ||
      values[3] < 0 || values[3] > 1000 ||
      values[4] < 3000 || values[4] > 9000 ||
      values[5] < 1800 || values[5] > 6000) return false;

  Telemetry parsed;
  parsed.node_id = static_cast<uint8_t>(values[0]);
  parsed.sequence = static_cast<uint16_t>(values[1]);
  parsed.temperature_x10_c = static_cast<int16_t>(values[2]);
  parsed.humidity_x10_percent = static_cast<uint16_t>(values[3]);
  parsed.pressure_x10_mmhg = static_cast<uint16_t>(values[4]);
  parsed.battery_mv = static_cast<uint16_t>(values[5]);
  out = parsed;  // Publish only after checksum and all fields have passed validation.
  return true;
}

class StreamDecoder {
 public:
  StreamDecoder() : size_(0), collecting_(false) { buffer_[0] = '\0'; }

  bool feed(char byte, Telemetry& decoded) {
    if (byte == '$') {
      collecting_ = true;
      size_ = 0;
      buffer_[size_++] = byte;
      return false;
    }
    if (!collecting_) return false;
    if (byte == '\r') return false;
    if (byte == '\n') {
      buffer_[size_] = '\0';
      collecting_ = false;
      return decode(buffer_, decoded);
    }
    if (byte < ' ' || byte > '~' || size_ >= kMaxFrameLength - 1) {
      collecting_ = false;  // Drop oversized/corrupted packet, resync at next '$'.
      size_ = 0;
      return false;
    }
    buffer_[size_++] = byte;
    return false;
  }

 private:
  char buffer_[kMaxFrameLength];
  size_t size_;
  bool collecting_;
};

}  // namespace meteo

#endif
