# Wicked Serum

**A modern, source-available wavetable synthesizer built for sound designers, producers, and Logic Pro users.**

Wicked Serum turns a familiar Serum-inspired workflow into a hackable JUCE instrument: morph between wavetable frames, stack wide unison voices, draw your own waveforms, route animated modulation, shape the result with filters and effects, and turn held chords into tempo-locked patterns.

It is designed to be immediately musical while remaining approachable for developers who want to learn how a real-time software synthesizer works.

> [!IMPORTANT]
> Wicked Serum is an independent source-available project. It is not affiliated with, endorsed by, or a replacement for Xfer Records or Serum. “Serum” is referenced only to describe the workflow that inspired this project. Wicked Serum does not load Serum presets and does not copy Serum code or factory content.

## Why Wicked Serum?

- **Built for Logic Pro:** native Audio Unit instrument support with automatic local installation after a successful build.
- **Sound-design focused:** dual wavetable oscillators, frame morphing, unison, modulation, filtering, effects, and an arpeggiator in one interface.
- **Editable at every level:** draw an LFO, redraw a wavetable frame, import a WAV, or change the C++ DSP itself.
- **Automation ready:** primary controls use JUCE's parameter-state system so Logic can save and automate them.
- **Open and inspectable:** no closed binary dependency beyond Apple's toolchain; JUCE is pinned as a Git submodule.
- **Regression tested:** focused tests cover modulation, ARP timing, and frame-preserving wavetable edits.

## Highlights

### Dual wavetable oscillators

OSC A and OSC B each provide:

- Smooth **WT POS** interpolation between wavetable frames
- Up to **16 unison voices** per oscillator
- Unison detune and stereo spread
- Level, pan, semitone detune, and octave controls
- Independent enable/disable controls
- 2,048 samples per wavetable frame
- Factory **Basic**, **Analog**, and **Spectral** tables

A sub-oscillator and noise generator also exist in the synthesis engine. Their dedicated UI workflow is still being expanded.

### Visual wavetable editor

The EDITOR tab is more than a waveform display:

- Edit one frame without collapsing the rest of the wavetable
- Select the frame you want to change
- Draw a single-cycle waveform directly
- Build waveforms from harmonic amplitudes
- Import waveform data from a WAV file
- Switch among Basic, Analog, and Spectral starting tables
- Use **CLEAR EDITS** to restore the selected factory table
- Continue morphing through edited and untouched frames with WT POS

### Modulation that is visible and understandable

The MOD tab supports both a Serum-style drag workflow and an explicit routing workflow:

1. Select a source such as LFO 1 or Macro 1.
2. Choose a destination.
3. Set a bipolar amount from `-1.00` to `+1.00`.
4. Press **ADD ROUTE**.

You can also drag a source badge directly onto a supported oscillator or filter knob. Active routes display colored modulation indicators.

Current routable destinations include:

- OSC A/B wavetable position
- OSC A/B level
- OSC A/B pan
- OSC A/B detune
- OSC A/B warp amount
- Filter cutoff
- Filter resonance
- Filter drive
- Filter mix

Four independent LFOs have their own rate, depth, and editable waveform. Four macro knobs provide performance-friendly control over multiple destinations.

> **Make modulation obvious:** lower the filter cutoff first, then route LFO 1 to Filter Cutoff at about `+0.35`. At a fully open 20 kHz cutoff, positive movement has nowhere audible to go.

### Tempo-synced arpeggiator

The ARP tab transforms held notes before they enter the synth engine:

- Up, Down, Up/Down, Down/Up, Random, and played-order patterns
- Tempo-synced note divisions, including triplets
- One-to-four-octave range
- Adjustable gate length
- Swing control
- Live step visualization
- Automatic BPM tracking from Logic with a 120 BPM fallback

### Filter and tone shaping

The main interface exposes cutoff, resonance, drive, mix, and key tracking. The DSP engine contains low-pass, high-pass, band-pass, notch, ladder, comb, and formant implementations; the current UI intentionally presents a streamlined primary filter workflow while broader type selection is being refined.

### Effects rack

The FX tab exposes four production essentials:

1. **Distortion** — add grit, density, and harmonic energy
2. **Chorus** — add movement and stereo width
3. **Delay** — add echoes with time, feedback, and mix control
4. **Reverb** — place the sound in a larger space

EQ, phaser, and flanger processors are present in the internal effects chain but do not yet have complete user-facing controls. They are not advertised as finished features.

### Presets and project recall

- Save and load user presets from `~/Library/Audio/Presets/SerumSynth/`
- Logic project state is serialized through JUCE's `AudioProcessorValueTreeState`
- Primary synth, ARP, LFO, macro, and effects parameters are available to the host

## Wicked Serum and Xfer Serum

Wicked Serum borrows workflow ideas from the wavetable-synth category, but it does not claim feature parity.

| Area | Wicked Serum today | Relative to Xfer Serum |
|---|---|---|
| Dual wavetable oscillators | Implemented | Familiar core concept, smaller table ecosystem |
| WT-position morphing | Implemented | Core workflow covered |
| Unison | Up to 16 voices | Similar high-level workflow; different DSP |
| Wavetable editor | Draw, harmonics, frame selection, WAV import | Useful focused subset |
| LFO editing | Four independent drawable LFOs | Core modulation-shape workflow covered |
| Mod routing | Drag-to-knob and explicit routing to oscillator/filter targets | Smaller destination set |
| Filters | Streamlined UI with broader internal DSP | Far fewer polished/exposed models |
| Effects | Distortion, chorus, delay, reverb exposed | Smaller rack; no claim of Serum FX parity |
| Arpeggiator | Integrated and tempo synchronized | Additional Wicked Serum workflow |
| Presets | Local user presets and Logic state recall | No Serum preset compatibility |
| MPE | Not currently advertised as complete | Future work |
| Platforms | AU/VST3/Standalone build targets; tested on macOS/Logic | Narrower tested platform coverage |

## Requirements

For the supported Logic Pro workflow:

- macOS
- Logic Pro
- Git
- CMake **3.22 or newer**
- Xcode or the Xcode Command Line Tools with a C++17-capable Apple Clang compiler

Install the command-line tools if needed:

```bash
xcode-select --install
```

Install CMake with Homebrew if needed:

```bash
brew install cmake
```

## Clone the project

JUCE is a pinned Git submodule, so clone recursively:

```bash
git clone --recurse-submodules https://github.com/sabcodes/Wicked-Serum.git
cd Wicked-Serum
```

If you already cloned without submodules:

```bash
git submodule update --init --recursive
```

## Build

Configure a release build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Build every format:

```bash
cmake --build build --config Release -j4
```

Or build only the Logic Pro Audio Unit:

```bash
cmake --build build --target SerumSynth_AU --config Release -j4
```

Expected release artifacts:

```text
build/SerumSynth_artefacts/Release/AU/SerumSynth.component
build/SerumSynth_artefacts/Release/VST3/SerumSynth.vst3
build/SerumSynth_artefacts/Release/Standalone/SerumSynth.app
```

CMake's `COPY_PLUGIN_AFTER_BUILD` setting automatically installs a successful AU build to:

```text
~/Library/Audio/Plug-Ins/Components/SerumSynth.component
```

This repository intentionally excludes compiled plugins and local build directories. Build the plugin locally from source.

## Validate the Audio Unit

macOS includes `auval`, Apple's Audio Unit validation tool:

```bash
auval -v aumu Srs1 Srsn
```

A healthy build ends with:

```text
AU VALIDATION SUCCEEDED.
```

The identifiers are:

- Type: `aumu` — Audio Unit music device
- Plugin code: `Srs1`
- Manufacturer code: `Srsn`
- Bundle identifier: `com.serumsynth.plugin`

## Load Wicked Serum in Logic Pro

1. Quit and reopen Logic after building the AU.
2. Create a new **Software Instrument** track.
3. Open the channel strip's **Instrument** slot.
4. Look under **AU Instruments** for **SerumSynth**.
5. Choose the stereo instrument instance.
6. Arm the track and play a MIDI keyboard or draw notes in the Piano Roll.

The installed plugin currently appears as **SerumSynth** inside Logic because that is the Audio Unit product name.

### Force Logic to rescan

If the plugin does not appear:

1. Open **Logic Pro → Settings → Plug-in Manager**.
2. Search for `SerumSynth`.
3. Select it and choose **Reset & Rescan Selection**.
4. Confirm that `auval -v aumu Srs1 Srsn` succeeds.
5. Restart Logic once more.

## Make your first sound

### 1. Start with OSC A

- Enable OSC A.
- Set Level around 70–80%.
- Load the **Basic** wavetable.
- Play a held MIDI note.
- Turn **WT POS** slowly to hear sine, triangle, saw-like, and square-like frames blend.

### 2. Add width

- Raise Unison from 1 to 4–7 voices.
- Add a modest amount of Unison Detune.
- Keep levels conservative; stacked voices increase output energy quickly.

### 3. Shape the tone

- Lower Filter Cutoff to roughly 800 Hz–3 kHz.
- Raise Resonance slightly.
- Add Drive for more harmonic density.
- Adjust ENV 1 for amplitude shape and ENV 2 for movement.

### 4. Create audible modulation

A reliable first route:

1. Open **MOD**.
2. Click **LFO1**.
3. Choose **Filter Cutoff** as the destination.
4. Set Amount near `+0.35`.
5. Press **ADD ROUTE**.
6. Set LFO 1 Rate around 1–3 Hz.
7. Draw a shape or choose sine, triangle, saw, square, or random.

For a performance control, select **M1**, route it to Osc A Level or Filter Cutoff, then move **Macro 1**.

Use **CLEAR ROUTES** to remove all active routes.

### 5. Turn a chord into a pattern

- Open **ARP** and enable **ARP ON**.
- Hold two or more notes.
- Choose a pattern and rate.
- Increase Octaves for a wider sequence.
- Lower Gate for short plucks or raise it for connected notes.
- Add Swing for an off-grid groove.

### 6. Edit a wavetable without losing morphing

- Open **EDITOR**.
- Choose Basic, Analog, or Spectral.
- Select a frame with the frame control.
- Draw the waveform or switch to Harmonics.
- Return to the oscillator and sweep WT POS.

Only the selected frame is replaced, so neighboring positions remain available. **CLEAR EDITS** restores the selected factory table.

### 7. Finish with effects

A useful order is already built into the effects chain:

```text
Distortion → EQ → Phaser → Flanger → Chorus → Delay → Reverb
```

Start with the exposed Chorus and Reverb controls for width and space, then add Delay or Distortion as needed.

## Presets

Use the preset bar to save your current parameter state. User presets are stored at:

```text
~/Library/Audio/Presets/SerumSynth/
```

Logic also saves plugin parameter state with the project. Custom drawn LFO/wavetable data is an area still being hardened for complete cross-session recall; save important source WAVs separately.

## Run the regression checks

Build and run the focused feature test executable:

```bash
cmake --build build --target SerumSynthFeatureTests --config Release -j4
./build/SerumSynthFeatureTests_artefacts/SerumSynthFeatureTests
```

The checks currently verify:

- Macro modulation audibly changes Osc A level
- Quarter-note ARP timing is correct at 120 BPM
- Editing one wavetable frame preserves all other morph positions

## Troubleshooting

### The plugin appears but produces no sound

- Use a Software Instrument track, not an audio track.
- Confirm the track receives MIDI and is armed.
- Enable OSC A and raise Osc A Level and Master Volume.
- Disable ARP while testing a single sustained note.
- Temporarily bypass effects and open the filter cutoff.

### ARP produces no pattern

- Enable **ARP ON**.
- Hold notes long enough for the selected rate.
- Confirm Logic's transport tempo is valid.
- Start with Up, 1/8, one octave, 80% gate, and zero swing.

### WT POS does not sound different

WT POS requires a multi-frame table. Load Basic, Analog, or Spectral first. A single imported or completely flattened frame will naturally produce less movement.

### Modulation is difficult to hear

- Start with a destination away from its limit.
- Use Amount around `±0.25` to `±0.50`.
- Confirm the selected LFO Depth is above zero.
- For a macro route, move the matching Macro knob after adding the route.

### CMake cannot find JUCE

```bash
git submodule update --init --recursive
```

Then configure again.

### Logic reports that the plugin failed validation

Run:

```bash
auval -v aumu Srs1 Srsn
codesign --verify --deep --strict \
  "$HOME/Library/Audio/Plug-Ins/Components/SerumSynth.component"
```

Rebuild the AU and use Plug-in Manager's **Reset & Rescan Selection**. Development builds are ad-hoc signed locally; distributing binaries requires your own Apple signing and notarization workflow.

## Project structure

```text
Source/
├── DSP/                 Oscillators, voices, filters, LFOs, ARP and effects
├── GUI/                 Plugin panels, editors and modulation overlay
├── Presets/             User and factory preset infrastructure
├── PluginProcessor.*    Logic/JUCE audio, MIDI and parameter integration
└── PluginEditor.*       Main tabbed interface
Tests/
└── FeatureRegressionTests.cpp
JUCE/                    Pinned JUCE 8.0.6 Git submodule
CMakeLists.txt           AU, VST3, Standalone and test targets
```

## Current project status

Wicked Serum is an active early-stage synthesizer, not a finished commercial replacement for Serum. The Logic AU path and the focused feature tests are validated on macOS. VST3 and Standalone targets are configured, but broader DAW/platform validation, polished factory content, complete parameter exposure, preset-data persistence, MPE, installer packaging, and release signing remain future work.

## Contributing

Useful contribution areas include:

- Oscillator anti-aliasing and CPU optimization
- More editable filter types and effect controls
- Modulation-matrix persistence and per-route editing
- Better preset and wavetable browsing
- Universal macOS release builds, signing, and notarization
- Cross-DAW VST3 testing
- Additional deterministic DSP regression tests

Before opening a pull request, build the affected target and run `SerumSynthFeatureTests`.

## Source and licensing

The code is publicly inspectable and intended for learning and continued development. A formal license file has not yet been selected; until one is added, standard copyright rules apply. JUCE is included as a submodule and remains subject to its own licensing terms.

---

**Build a patch. Draw the motion. Make it wicked.**
