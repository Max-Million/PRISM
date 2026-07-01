# PRISM v1.0.0 Release Notes

PRISM is an assignable vector distortion plugin built with JUCE.

## Main Features

- Four-corner vector morphing distortion
- Assignable algorithms per corner
- Eight distortion algorithms:
  - Tube
  - Hard Clip
  - Foldback
  - Fuzz
  - Amp Drive
  - Bitcrush
  - Wavefolder
  - Rectifier
- Global Drive, Tone, Mix, Input, and Output controls
- Stereo Output Modes:
  - Stereo
  - Mono
  - Mid Only
  - Side Only
- Factory preset browser
- Randomize button
- Randomize lock buttons for:
  - Drive
  - Tone
  - Mix
  - Vector
  - Corners
- 2x oversampling
- DC blocking / silence protection to reduce pops and offset artifacts
- VST3 and Standalone builds

## Release Check

Verified:

- Debug build
- Release build
- Standalone launch
- VST3 loads in DAW
- Factory presets load correctly
- Randomize and locks work correctly
- Bypass stays clean
- Silence remains silent
- Side Only output works correctly
- No obvious startup or close pop

## Notes

This is the first stable v1 feature set. Future versions may add user preset saving, quality modes, deeper modulation, and additional polish.
