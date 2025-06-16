// LOLIN S2 Mini Configurable Sine Wave Generator

const int dacPin = 17;                 // DAC1 output pin on ESP32-S2 mini
const int sineTableSize = 100;         // Number of samples per sine wave period
uint8_t sineTable[sineTableSize];      // Lookup table for sine wave values (0–255)
int tableIndex = 0;

volatile uint8_t lastVal = 0;          // Last DAC value output (used for serial print)
hw_timer_t* timer = NULL;

// === Change this variable to adjust output frequency ===
const float outputFrequency = 15.0;   // in Hz

void IRAM_ATTR onTimer() {
  uint8_t val = sineTable[tableIndex];
  dacWrite(dacPin, val);
  tableIndex = (tableIndex + 1) % sineTableSize;
  lastVal = val;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Populate sine table
  for (int i = 0; i < sineTableSize; i++) {
    float angle = 2 * PI * i / sineTableSize;
    sineTable[i] = (uint8_t)(127.5 + 127.5 * sin(angle));
  }

  // Calculate timer period from desired output frequency
  float periodMicroseconds = (1e6 / outputFrequency) / sineTableSize;

  timer = timerBegin(0, 80, true); // prescaler = 80 → 1 tick = 1 μs
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, (uint32_t)periodMicroseconds, true); // trigger every x μs
  timerAlarmEnable(timer);
}

void loop() {
  static uint32_t lastPrintTime = 0;
  uint32_t now = millis();

  if (now - lastPrintTime >= 1) {
    lastPrintTime = now;
    Serial.println(lastVal);  // for Serial Plotter
  }
}