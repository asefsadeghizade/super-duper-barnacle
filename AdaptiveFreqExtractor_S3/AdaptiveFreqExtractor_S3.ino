#include <Arduino.h>
#include <arduinoFFT.h>

const uint16_t SAMPLE_RATE = 1024;    // Hz
const uint16_t FFT_SIZE    = 1024;    // Must be power of 2

const uint8_t  ADC_PIN     = 5;       // Analog input
const double   V_REF       = 3.3;     // Reference voltage

double vReal[FFT_SIZE];
double vImag[FFT_SIZE];

ArduinoFFT<double> FFT(vReal, vImag, FFT_SIZE, SAMPLE_RATE);

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);           // 0–4095
  analogSetAttenuation(ADC_11db);     // Max range (approx 3.3V)
}

void loop() {
  // === 1) Acquire samples at fixed rate ===
  const unsigned long interval = 1000000UL / SAMPLE_RATE;
  unsigned long t = micros();
  for (uint16_t i = 0; i < FFT_SIZE; i++) {
    while (micros() - t < interval);  // wait until next sample
    t += interval;
    int raw = analogRead(ADC_PIN);
    vReal[i] = ((double)raw - 2048.0) / 2048.0; // normalize ±1
    vImag[i] = 0.0;
  }

  // === 2) Perform FFT ===
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // === 3) Find fundamental ===
  uint16_t peakBin = 1;
  double peakMag = vReal[1];
  for (uint16_t i = 2; i < FFT_SIZE / 2; i++) {
    if (vReal[i] > peakMag) {
      peakMag = vReal[i];
      peakBin = i;
    }
  }

  double freqFund = peakBin * (double)SAMPLE_RATE / FFT_SIZE;
  double ampFund  = peakMag * (V_REF / 2.0);

  // === 4) Look for second harmonic ===
  uint16_t target2nd = peakBin * 2;
  uint16_t searchRange = 2;  // search ±2 bins around expected 2nd harmonic
  double mag2nd = 0.0;
  uint16_t bin2nd = 0;

  for (int i = max(peakBin + 1, target2nd - searchRange); i <= min(FFT_SIZE / 2 - 1, target2nd + searchRange); i++) {
    if (vReal[i] > mag2nd) {
      mag2nd = vReal[i];
      bin2nd = i;
    }
  }

  double freq2nd = bin2nd * (double)SAMPLE_RATE / FFT_SIZE;
  double amp2nd  = mag2nd * (V_REF / 2.0);

  // === 5) Print result ===
  Serial.printf("Fundamental: %.1f Hz  Amp: %.3f V\n", freqFund, ampFund);
  if (amp2nd > 0.05 * ampFund) {  // only print if significant (e.g. >5%)
    Serial.printf("2nd Harmonic: %.1f Hz  Amp: %.3f V\n", freq2nd, amp2nd);
  }

  delay(100);
}
