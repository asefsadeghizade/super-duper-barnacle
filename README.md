# Adaptive Frequency Signal Projects (ESP32 Series)

This repository includes two core Arduino projects built for real-time signal generation and frequency extraction using ESP32-based boards:

---

## 📁 Project Folders

### 1. `SignalGenerator_S2-mini`
**Board**: LOLIN S2 Mini  
**Function**: Generates a 130 Hz sine wave on GPIO17 using the DAC.  
**Key Features**:
- Timer-based sine wave output
- Lookup table with 100 samples per cycle
- 130 Hz frequency generated at 13,000 samples/sec (13 kHz DAC rate)
- Serial output of waveform values for visualizing with Serial Plotter

### 2. `AdaptiveFreqExtractor_S3`
**Board**: ESP32-S3  
**Function**: Reads an analog sinusoidal input on GPIO5 and calculates its frequency in real-time.  
**Key Features**:
- 12-bit ADC sampling at 2 kHz using a timer interrupt
- Rising zero-crossing detection for frequency estimation
- Serial output showing frequency (every 500 ms)

---

## 🧠 Project Goals

These two components are part of a larger project titled:

> **Adaptive Frequency Extraction and Generation System for Low-Frequency Analog Signals**

This system enables testing and calibration of analog sensing or signal processing algorithms in the range of **10 Hz to 130 Hz**, with clear serial visualization.

---

## 🧩 Suggested Evolution Path

### Phase 1 – Base Functionality ✅
- [x] Generate stable sine wave (S2 Mini)
- [x] Extract frequency from ADC signal (S3)

### Phase 2 – Improvements 🔧
- [ ] Add filtering to frequency extractor (e.g., moving average, bandpass)
- [ ] Display amplitude and signal quality
- [ ] Add mode for detecting second harmonic

### Phase 3 – Smart Features 🚀
- [ ] Adaptive sampling rate based on signal content
- [ ] Real-time display via web server (ESP32 WiFi)
- [ ] Auto-synchronization between generator and extractor

---

## 💡 Naming & Versioning Strategy

### Folder Naming:
Use `ComponentPurpose_Board` format:
- `SignalGenerator_S2-mini`
- `AdaptiveFreqExtractor_S3`

### Sketch/File Naming:
Use versioned and descriptive names:
- `SineGenerator130Hz_v1.0.ino`
- `FreqExtractorBasic_v1.0.ino`
- Later: `FreqExtractor_Filtered_v1.1.ino`, etc.

### Git Branches/Tags:
Keep separate **branches** for major feature additions (e.g., `filtering`, `web-interface`)  
Use **Git tags** for versioned releases:
```bash
git tag v1.0-sine-generator
git tag v1.0-frequency-extractor
