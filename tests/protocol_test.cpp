#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "meteo_protocol.h"

static meteo::Telemetry sample() {
  meteo::Telemetry p = {1, 65535, -123, 537, 7603, 4021};
  return p;
}
static bool same(const meteo::Telemetry& a, const meteo::Telemetry& b) {
  return a.node_id == b.node_id && a.sequence == b.sequence &&
         a.temperature_x10_c == b.temperature_x10_c &&
         a.humidity_x10_percent == b.humidity_x10_percent &&
         a.pressure_x10_mmhg == b.pressure_x10_mmhg &&
         a.battery_mv == b.battery_mv;
}
static std::string frame(const meteo::Telemetry& p) {
  char buffer[meteo::kMaxFrameLength];
  assert(meteo::encode(p, buffer, sizeof(buffer)));
  return std::string(buffer);
}
static bool feed(const std::string& input, meteo::StreamDecoder& decoder,
                 meteo::Telemetry& output) {
  bool got = false;
  for (size_t i = 0; i < input.size(); ++i) {
    if (decoder.feed(input[i], output)) got = true;
  }
  return got;
}
int main() {
  meteo::Telemetry decoded = {};
  const meteo::Telemetry original = sample();
  const std::string raw = frame(original);
  assert(raw[0] == '$');
  assert(raw[raw.size() - 1] == '\n');
  assert(meteo::decode(raw.substr(0, raw.size() - 1).c_str(), decoded));
  assert(same(decoded, original));
  assert(meteo::crc8("123456789", 9) == 0xF4);
  meteo::StreamDecoder decoder;
  assert(!feed(raw.substr(0, 5), decoder, decoded));
  assert(feed(raw.substr(5), decoder, decoded));
  assert(same(decoded, original));
  std::string bad = raw;
  bad[10] = bad[10] == '4' ? '3' : '4';
  meteo::StreamDecoder corrupted;
  assert(!feed(bad, corrupted, decoded));
  assert(feed(raw, corrupted, decoded));
  meteo::StreamDecoder overflow;
  assert(!feed("$" + std::string(100, '1') + "\n", overflow, decoded));
  assert(feed(raw, overflow, decoded));
  meteo::StreamDecoder noise;
  assert(feed("noise\r\nAT\r\n" + raw, noise, decoded));
  assert(same(decoded, original));
  meteo::StreamDecoder restarted;
  assert(feed("$M2,1,partial" + raw, restarted, decoded));
  meteo::StreamDecoder crlf;
  std::string windows = raw;
  windows.insert(windows.size()-1, "\r");
  assert(feed(windows, crlf, decoded));
  meteo::Telemetry invalid = original;
  invalid.battery_mv = 7000;
  char buffer[meteo::kMaxFrameLength];
  assert(!meteo::encode(invalid, buffer, sizeof(buffer)));
  assert(!meteo::encode(original, buffer, 12));
  assert(!meteo::encode(original, NULL, sizeof(buffer)));
  meteo::Telemetry high = {99, 0, 850, 1000, 9000, 6000};
  std::string highFrame = frame(high);
  assert(meteo::decode(highFrame.substr(0, highFrame.size()-1).c_str(), decoded));
  assert(same(decoded, high));
  puts("meteo protocol: all tests passed");
}
