# Morph

Clean JUCE 8 + CMake starter for a morphing distortion plugin.

## Windows build

From PowerShell in this folder:

```powershell
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue
cmake -B build -S . -DJUCE_DIR="C:/path/to/JUCE/extras/Build/CMake"
cmake --build build --config Debug
```

For Windows Milestone 1, build formats are VST3 and Standalone.
AU/AUv3 are macOS-only. AAX requires the Avid AAX SDK.
