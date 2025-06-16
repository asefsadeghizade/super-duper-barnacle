// LOLIN S2 Mini – Function Generator with up to 5 Harmonics

#include <Arduino.h>

const int dacPin         = 17;       // DAC1 on ESP32-S2
const int sineTableSize = 100;       // samples per sine cycle
uint8_t sineTable[sineTableSize];    // 0–255 lookup
volatile uint8_t lastVal = 0;        // for Serial Plotter

hw_timer_t* timer = NULL;

// === User parameters ===
// Fundamental frequency (10–130 Hz)
const float outputFrequency = 15.0;

// Relative amplitudes of the first 5 harmonics (fundamental = harmonic 1)
const float ampHarm[5] = {
  1.0,   // 1× (fundamental)
  0.5,   // 2×
  0.3,   // 3×
  0.1,   // 4×
  0.09   // 5×
};

// === Phase accumulator (table‐index units) ===
volatile int phaseFund = 0;

// Compute timer interval (µs) for given fundamental
uint32_t calcTimerInterval() {
  float periodUs = 1e6 / outputFrequency;           // µs per full sine cycle
  return uint32_t(periodUs / sineTableSize + 0.5);  // µs per table step
}

void IRAM_ATTR onTimer() {
  // Sum weighted normalized samples for each harmonic
  float totalAmp = 0;
  float sum = 0;

  for (int h = 0; h < 5; h++) {
    float amp = ampHarm[h];
    if (amp <= 0) continue;
    totalAmp += amp;

    // index = fundamental phase × (h+1), wrapped by table size
    int idx = (phaseFund * (h+1)) % sineTableSize;
    uint8_t raw = sineTable[idx];
    // normalize 0–255 → –1…+1
    float norm = (raw - 127.5f) / 127.5f;
    sum += amp * norm;
  }

  // Normalize sum back to –1…+1
  if (totalAmp > 0) sum /= totalAmp;

  // Convert back to 0–255 and output
  uint8_t out = uint8_t(127.5f + 127.5f * sum);
  dacWrite(dacPin, out);
  lastVal = out;

  // Advance fundamental phase
  phaseFund = (phaseFund + 1) % sineTableSize;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // Build sine lookup table
  for (int i = 0; i < sineTableSize; i++) {
    float angle = 2 * PI * i / sineTableSize;
    sineTable[i] = uint8_t(127.5f + 127.5f * sinf(angle));
  }

  // Setup timer
  timer = timerBegin(0, 80, true);              // prescaler=80 → 1 tick = 1µs
  timerAttachInterrupt(timer, &onTimer, true);
  uint32_t tUs = calcTimerInterval();
  timerAlarmWrite(timer, tUs, true);
  timerAlarmEnable(timer);

  // Debug info
  Serial.printf("Fundamental: %.1f Hz\n", outputFrequency);
  for (int h = 0; h < 5; h++)
    Serial.printf(" Harmonic %d amp: %.2f\n", h+1, ampHarm[h]);
  Serial.printf(" Timer interval: %u µs\n", tUs);
}

void loop() {
  // Print last DAC output for Serial Plotter
  static uint32_t last = 0;
  if (millis() - last >= 20) {
    last = millis();
    Serial.println(lastVal);
  }
}
