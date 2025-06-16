// LOLIN S2 Mini – Sine + 2nd Harmonic Generator with Independent Amplitudes

#include <Arduino.h>

const int dacPin         = 17;           // DAC1 on ESP32-S2
const int sineTableSize = 100;           // samples per cycle
uint8_t sineTable[sineTableSize];        // 0–255 lookup
volatile uint8_t lastVal = 0;            // for Serial plot

hw_timer_t* timer = NULL;

// --- User parameters ---
const float outputFrequency      = 15.0;  // fundamental in Hz (10–130)
const float ampFundamental       = 1.0;   // relative amplitude of fundamental (0.0–1.0)
const float ampSecondHarmonic    = 0.0;   // relative amplitude of 2nd harmonic (0.0–1.0)

// --- Phase accumulators (in table‐index units) ---
volatile int phaseFund = 0;
volatile int phase2nd  = 0;

// Calculate timer interval in microseconds for given frequency
uint32_t calcTimerInterval() {
  float periodPerCycle = 1e6 / outputFrequency;     // µs per sine cycle
  return uint32_t(periodPerCycle / sineTableSize + 0.5);
}

void IRAM_ATTR onTimer() {
  // 1) Lookup table values
  int idx1 = phaseFund;
  int idx2 = phase2nd;
  uint8_t v1 = sineTable[idx1];
  uint8_t v2 = sineTable[idx2];

  // 2) Convert to -1.0→+1.0 floats
  float n1 = (v1 - 127.5f) / 127.5f;
  float n2 = (v2 - 127.5f) / 127.5f;

  // 3) Weight & sum
  float sum = ampFundamental * n1 + ampSecondHarmonic * n2;
  float norm = ampFundamental + ampSecondHarmonic;
  if (norm > 0) sum /= norm;     // re-normalize to -1..1

  // 4) Convert back to 0–255
  uint8_t out = uint8_t(127.5f + 127.5f * sum);
  dacWrite(dacPin, out);
  lastVal = out;

  // 5) Advance phases
  phaseFund = (phaseFund + 1) % sineTableSize;
  phase2nd  = (phase2nd  + 2) % sineTableSize;  // 2× speed for 2nd harmonic
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // Build sine lookup table
  for (int i = 0; i < sineTableSize; i++) {
    float angle = 2 * PI * i / sineTableSize;
    sineTable[i] = uint8_t(127.5f + 127.5f * sinf(angle));
  }

  // Configure timer
  timer = timerBegin(0, 80, true);  // prescaler=80 → 1 tick=1µs
  timerAttachInterrupt(timer, &onTimer, true);
  uint32_t tUs = calcTimerInterval();
  timerAlarmWrite(timer, tUs, true);
  timerAlarmEnable(timer);

  Serial.printf("Gen: %.1fHz fund @amp %.2f + 2nd @amp %.2f\n",
                outputFrequency, ampFundamental, ampSecondHarmonic);
  Serial.printf("Timer interval: %u µs\n", tUs);
}

void loop() {
  // Print last DAC value for plot
  static uint32_t last = 0;
  if (millis() - last >= 20) {
    last = millis();
    Serial.println(lastVal);
  }
}
