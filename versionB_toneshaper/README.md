# Sherman Filterbank VSTi

This project scaffolds a JUCE-based VST3/Standalone plugin named Sherman Filterbank with a more expressive filterbank-style tone engine.

## What changed

- A richer 4-band filterbank core with tuned band frequencies and Q values
- Preset switching for Clean, Warm, Bright, Punchy, and Dark tones
- A more polished Live-friendly editor with rotary controls and a preset selector

## Build

1. Install CMake and a C++ compiler (Visual Studio Build Tools or LLVM/Clang).
2. Configure the project:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   ```
3. Build:
   ```bash
   cmake --build build --config Release
   ```
