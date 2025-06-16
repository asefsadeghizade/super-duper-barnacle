// LOLIN S2 Mini 130Hz sine Wave generator 

const int dacPin = 17;                // DAC1 output pin on ESP32-S2 mini
const int sineTableSize = 100;        // Number of samples per sine wave period
uint8_t sineTable[sineTableSize];     // Lookup table for sine wave values (0-255)
int tableIndex = 0;                   // current index in sine table

volatile uint8_t lastVal = 0;         // last DAC value output (used for serial printing).
                                      // declare here, before onTimer

hw_timer_t* timer = NULL;             // hardware timer pointer

void IRAM_ATTR onTimer() {
  uint8_t val = sineTable[tableIndex];
  dacWrite(dacPin, val);
  tableIndex = (tableIndex + 1) % sineTableSize;

  lastVal = val;  // update global volatile variable
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < sineTableSize; i++) {
    float angle = 2 * PI * i / sineTableSize;
    sineTable[i] = (uint8_t)(127.5 + 127.5 * sin(angle));
  }

  timer = timerBegin(0, 80, true);  // Timer0, prescaler=80 → 1 tick = 1μs (since CPU clock = 80MHz)              
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 77, true); // 77μs period → timer triggers every 77 microseconds
  timerAlarmEnable(timer);
}

void loop() {
  static uint32_t lastPrintTime = 0;
  uint32_t now = millis();

  if (now - lastPrintTime >= 1) {   // every 1 ms
    lastPrintTime = now;
    Serial.println(lastVal);
  }
}
