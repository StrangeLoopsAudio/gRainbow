# gRainbow   ![GitHub release (release name instead of tag name)](https://img.shields.io/github/v/release/StrangeLoopsAudio/gRainbow) ![build (with event)](https://img.shields.io/github/actions/workflow/status/StrangeLoopsAudio/gRainbow/ci.yml) ![GitHub all releases](https://img.shields.io/github/downloads/StrangeLoopsAudio/gRainbow/total) ![contributors](https://img.shields.io/github/contributors/StrangeLoopsAudio/gRainbow) ![GitHub repo size](https://img.shields.io/github/repo-size/StrangeLoopsAudio/gRainbow)

A synthesizer that uses pitch detection to choose candidates for granular synthesis or sampling.

Note: This synth has recently been redesigned! This page reflects the most recent information about the project.

![gRainbow](Source/Resources/logo.png)

## Why?

gRainbow was created to overcome a few shortcomings of traditional granular synths.

1. Pitch variations in the input clip can produce inharmonic tones, which isn't always wanted. This leads users to often use short single-pitch clips, restricting the synth to a single timbre.
2. Manual pitch matching to the input clip is often required to produce the correct notes with MIDI input, which can be difficult and repetitive.
3. Pitch shifting is commonly done with timestretching, which can create unwanted artifacts when shifting multiple octaves in either direction.

Instead, gRainbow prefers longer, pitch-diverse audio clips (1), automatically produces harmonics matched for MIDI input (2) and avoids too much timestretching by generating harmonics that are already near their target pitch (3). Voila!

## Features

- Full MIDI range, polyphonic voicing
- Quick and reliable pitch detection using Spotify's BasicPitch machine learning model
- Up to 100 grains playing at once
- Ultra-customizable parameters that can be overriden at the per-note and per-generator level
    - Grain envelope shape, tilt, rate and duration parameters
    - Pitch, pan and grain position spray and adjustment parameters
    - Play grains in forward or reverse
    - Option to sync rate parameters to the host BPM
    - Option to play a reference tone for each note to help dial in pitches
    - and more!
- Full internal modulation and mapping system
    - Amplitude envelope and 2 additional envelope modulators
    - 3 LFO modulators
    - 4 macros
- Working preset loading and saving system that bundles the audio sample into the preset for you
- Cute clouds that sing!

## Some terminology

- notes: one of 12 notes in the chromatic scale. gRainbow can now play the full range of MIDI notes, but its note parameters are set on a 12-tone basis.
- generators: create grains for a particular note at a particular position in the audio clip.
- candidates: a collection of viable positions in the audio clip where the pitch is harmonic with a particular note.
- parameters: just like usual ones, but organized a bit differently. Parameters use their global value by default, but can be overridden at both the note and generator level. The lowest-level parameter that is different from its default value is used, in the order of global->note->generator. In this way, you'll only have to change the lower-level parameters when you need to without losing any flexibility. You can always double click on a knob to reset it to a higher level too!

## Plugin flow

When an audio file is loaded into gRainbow, a pitch detection analysis is ran to segment the clip into discrete pitches over time. Next, gRainbow finds up to 6 pitch-matched position candidates for each note, guaranteeing harmonically agreeable grains. Finally, the user customizes each note and its generators (i.e. changing parameters and candidates) to their liking, and can save everything into a preset to avoid processing things again next time.

![gRainbow gui](docs/gRainbow1_0_0.png)

Is anything important missing or needed? Open an [Issue](github.com/bboettcher3/gRainbow/issues) and let us know!
