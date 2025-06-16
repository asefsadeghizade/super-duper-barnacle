#include <Arduino.h>
#include <arduinoFFT.h>

const uint16_t SAMPLE_RATE = 1024;    // Hz (covers up to 512 Hz)
const uint16_t FFT_SIZE    = 1024;    // power of two

const uint8_t  ADC_PIN     = 5;       // GPIO5
const double   V_REF       = 3.3;

double vReal[FFT_SIZE];
double vImag[FFT_SIZE];

// Specify template argument <double>
ArduinoFFT<double> FFT(vReal, vImag, FFT_SIZE, SAMPLE_RATE);

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  // 1) Acquire samples at SAMPLE_RATE
  unsigned long interval = 1000000UL / SAMPLE_RATE;
  unsigned long prev = micros();
  for (uint16_t i = 0; i < FFT_SIZE; i++) {
    while (micros() - prev < interval);
    prev += interval;
    int raw = analogRead(ADC_PIN);
    vReal[i] = ((double)raw - 2048.0) / 2048.0; // normalize ±1
    vImag[i] = 0.0;
  }

  // 2) Window, FFT, Magnitude
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // 3) Find peak bin (skip DC bin 0)
  uint16_t peakBin = 1;
  double   peakMag = vReal[1];
  for (uint16_t b = 2; b < FFT_SIZE/2; b++) {
    if (vReal[b] > peakMag) {
      peakMag = vReal[b];
      peakBin = b;
    }
  }

  // 4) Convert bin to frequency & amplitude
  double freq = peakBin * (double)SAMPLE_RATE / FFT_SIZE;  
  double amp  = peakMag * (V_REF/2.0);

  Serial.printf("Freq: %.1f Hz   Amp: %.3f V\n", freq, amp);

  delay(50);
}
