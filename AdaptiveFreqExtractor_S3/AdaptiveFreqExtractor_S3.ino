#include <Arduino.h>
#include <arduinoFFT.h>

const uint16_t SAMPLE_RATE = 1024;   // Hz
const uint16_t FFT_SIZE    = 1024;   // power of two

const uint8_t  ADC_PIN     = 5;      // Analog input
const double   V_REF       = 3.3;    // Reference voltage

double vReal[FFT_SIZE];
double vImag[FFT_SIZE];
ArduinoFFT<double> FFT(vReal, vImag, FFT_SIZE, SAMPLE_RATE);

// How strong a harmonic must be (as fraction of fundamental) to report it
const double HARM_THRESHOLD = 0.05;  // 5%

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);          // 0–4095
  analogSetAttenuation(ADC_11db);    // Max range (≈3.3V)
}

void loop() {
  // 1) Acquire samples at fixed rate
  const unsigned long interval = 1000000UL / SAMPLE_RATE;
  unsigned long t = micros();
  for (uint16_t i = 0; i < FFT_SIZE; i++) {
    while (micros() - t < interval);
    t += interval;
    int raw = analogRead(ADC_PIN);
    vReal[i] = ((double)raw - 2048.0) / 2048.0;  // normalize ±1
    vImag[i] = 0.0;
  }

  // 2) FFT
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // 3) Fundamental peak
  uint16_t peakBin = 1;
  double peakMag = vReal[1];
  for (uint16_t i = 2; i < FFT_SIZE/2; i++) {
    if (vReal[i] > peakMag) {
      peakMag = vReal[i];
      peakBin = i;
    }
  }
  double freqFund = peakBin * (double)SAMPLE_RATE / FFT_SIZE;
  double ampFund  = peakMag * (V_REF/2.0);

  // Print fundamental
  Serial.printf("Fundamental: %.1f Hz  Amp: %.3f V\n", freqFund, ampFund);

  // 4) Find and report harmonics 2..5
  for (int h = 2; h <= 5; h++) {
    uint16_t targetBin = peakBin * h;
    if (targetBin >= FFT_SIZE/2) break;  // beyond Nyquist

    // search ±2 bins around targetBin
    int minBin = max<int>(targetBin - 2, 1);
    int maxBin = min<int>(targetBin + 2, FFT_SIZE/2 - 1);

    double bestMag = 0;
    uint16_t bestBin = 0;
    for (int b = minBin; b <= maxBin; b++) {
      if (vReal[b] > bestMag) {
        bestMag = vReal[b];
        bestBin = b;
      }
    }

    double freqH = bestBin * (double)SAMPLE_RATE / FFT_SIZE;
    double ampH  = bestMag * (V_REF/2.0);
    if (ampH > HARM_THRESHOLD * ampFund) {
      Serial.printf("%d%s Harmonic: %.1f Hz  Amp: %.3f V\n",
                    h,
                    (h==2?"nd":h==3?"rd":"th"),
                    freqH, ampH);
    }
  }

  Serial.println();  // blank line between frames
  delay(100);
}
