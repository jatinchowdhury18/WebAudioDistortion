# Web Audio Distortion

A proof of concept for porting audio processing desktop applications made in c++ to web audio applications. `myeffect.cpp` is a simple distortion effect with a low pass filter. Using the [Emscripten](https://github.com/kripken/emscripten), the effect was compiled into web assembly and javascript, then wrapped as an Audio Worklet Module, and connected to a monophonic sine wave MIDI synth.

### More information:
  - See the [completed Web Audio application](https://ccrma.stanford.edu/~jatin/220a/fp/) (need Google Chrome)
  - For more information on [porting C++ audio applications to web audio](https://developers.google.com/web/updates/2018/06/audio-worklet-design-pattern)
  - This project was completed as a final project for [Music 220a](https://ccrma.stanford.edu/courses/220a/) at Stanford University (CCRMA)