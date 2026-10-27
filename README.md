# THE NOIZZ PASSAGE — by NOIZZ STUDIO

**High-Gain Technical Death Metal Amp Simulator, Intelligent Gate, Sub-Kick Boost, Dual IR Loader & Phase-Safe Widener (VST3 & AU for Windows x64 and macOS Universal)**

Inspired by the surgical guitar tones of **The Zenith Passage** (*Datalysium*, *Solipsist*, *Cosmic Dissonance*, and *Fleshbound Reliquary*).

---

## Key Features

1. **Multi-Bus Support (`Mono -> Mono`, `Stereo -> Stereo`, `Mono -> Stereo`)**
2. **`NR` (Noise Reduction Knob)**: Eliminates 50/60 Hz cable hum, static noise, and high-frequency hiss via a zero-latency adaptive downward expander and dynamic de-hiss filter.
3. **`TIGHT` (Intelligent Gate Button)**: Dual-band transient/sustain ratio discriminator that clamps staccato palm-mutes with a 3.2 ms gunshot release while automatically extending release time (~195 ms) when upper-harmonic string resonance is detected so sustained notes and solos ring out naturally.
4. **`BOOST` (Sub-Bass Kick & Backing Bass Note Knob)**: Tracks pick-attack transients and low-end palm-mute envelopes to synthesize a subtle 58 Hz kick-drum thump plus a phase-locked divide-by-2 sub-octave bass guitar note when the pick hits the strings.
5. **Zenith High-Gain Amp Simulator**: Fortin 33/Grind pre-boost + 4-stage cascaded Anti-Derivative Anti-Aliasing (ADAA) tube preamp + Dave Otero Flatline surgical post-EQ (`GAIN`, `BASS`, `MID`, `TREBLE`, `PRESENCE`, `DEPTH`).
6. **Dual Cabinet IR Loader (`CAB L` & `CAB R`)**:
   - 10 embedded 48 kHz / 24-bit minimum-phase factory IRs matching The Zenith Passage's studio and live cabinets (also included as standalone `.wav` files in `IRs/`).
   - Load any external `.wav` or `.aiff` IR file via `LOAD IR...`.
   - Dedicated `ON/OFF` switch, `VOL` knob, `PAN` knob per cab, and a master `CAB L / R MIX` slider.
7. **`SPREAD` (Phase-Safe Haas & 100%–200% Stereo Widener)**:
   - 4th-order Linkwitz-Riley crossover at 180 Hz keeps the low end strictly mono and phase-locked.
   - On a **Mono input**, generates a rich Haas stereo image using an all-pass decorrelated Side injection that sums back to mono with zero comb-filtering.
   - On a **Stereo input**, continuously expands stereo width from **100% up to 200%** while preserving a tight low end and zero phase cancellation.
8. **Futuristic 3D PCB GUI**: Resizable 16:10 aspect-ratio-locked interface in neon red (`#FF1744`), pitch black, and brushed dark gray metallic tones with intricate PCB traces, `UNDO`/`REDO`, `A`/`B` comparison, `BYPASS`, preset bar with `SAVE`/`OPEN`, and stereo/mono `INPUT`/`OUTPUT` meters and sliders.

---

## Compiling on GitHub Actions (Recommended)

1. Create a new GitHub repository and push the contents of this folder to the `main` branch:
   ```bash
   git init
   git add .
   git commit -m "Initial commit: THE NOIZZ PASSAGE by NOIZZ STUDIO"
   git branch -M main
   git remote add origin https://github.com/<your-username>/the-noizz-passage.git
   git push -u origin main
   ```
2. Open the **Actions** tab on GitHub. The workflow `.github/workflows/build.yml` will automatically fetch **JUCE 8**, compile:
   - **Windows x64**: `THE NOIZZ PASSAGE.vst3`
   - **macOS Universal (`x86_64` + `arm64`)**: `THE NOIZZ PASSAGE.vst3` and `THE NOIZZ PASSAGE.component` (AU)
3. Download the compiled plugin bundles directly from the workflow **Artifacts** section.

## Compiling Locally on Windows x64 (Visual Studio 2022 + CMake)

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
```
