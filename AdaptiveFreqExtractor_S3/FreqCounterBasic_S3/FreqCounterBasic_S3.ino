const int adcPin = 5;         // ADC input pin (change if needed)
const int sampleRate = 2000;  // ADC sample rate in Hz
const int sampleIntervalUs = 1000000 / sampleRate;

volatile int lastAdcValue = 0;
volatile unsigned long lastCrossTime = 0;
volatile unsigned long periodUs = 0;
volatile bool newFreqAvailable = false;

unsigned long lastPrintTime = 0;
float frequency = 0.0;
int amplitude = 0;

void IRAM_ATTR onTimer() {
  int adcValue = analogRead(adcPin);

  // Detect positive zero crossing: last sample < mid, current sample >= mid
  const int mid = 2048; // for 12-bit ADC (0-4095), midpoint ~2048

  if (lastAdcValue < mid && adcValue >= mid) {
    unsigned long now = micros();
    if (lastCrossTime > 0) {
      periodUs = now - lastCrossTime;
      newFreqAvailable = true;
    }
    lastCrossTime = now;
  }

  lastAdcValue = adcValue;
}

hw_timer_t* timer = NULL;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db); // Wider voltage range

  // Configure timer for sampling
  timer = timerBegin(0, 80, true); // 80 MHz clock, prescaler=80 → 1 MHz timer (1 us ticks)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, sampleIntervalUs, true);
  timerAlarmEnable(timer);
}

void loop() {
  if (newFreqAvailable) {
    noInterrupts();
    unsigned long p = periodUs;
    newFreqAvailable = false;
    interrupts();

    if (p > 0) {
      frequency = 1000000.0 / p;
    } else {
      frequency = 0;
    }
  }

  // Print every 500 ms
  if (millis() - lastPrintTime > 500) {
    lastPrintTime = millis();
    Serial.print("Frequency: ");
    Serial.print(frequency, 2);
    Serial.println(" Hz");
  }
}
