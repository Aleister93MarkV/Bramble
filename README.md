# Bramble

A cross-platform audio player with skins support, built with Qt6 and miniaudio.

## Features

- **Multiple Audio Formats**: WAV, AIFF, PCM, FLAC, ALAC, MP3, AAC, OGG, WMA, M4A, MP2, MPC, OPUS, ASF, MOD, XM, S3M, IT
- **18-Band Equalizer**: Full parametric equalizer with visual sliders
- **Crystallizer Effect**: Psychoacoustic bass enhancement
- **Visualizer**: Real-time audio visualization
- **Skin System**: 9 built-in themes (Classic, Neon Night, Retro PC, Matrix, Synthwave, Cherry, Ocean, Sunset, Red Fox)
- **Album Art**: Automatic cover detection from music directory

## Requirements

### Linux
- Qt6 (Qt6 Widgets, Qt6 Gui, Qt6 Core)
- libsndfile
- libopenmpt

### Windows
- Qt6 for Windows (MinGW or MSVC)
- libsndfile for Windows
- libopenmpt for Windows

## Building

### Linux

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install qt6-base-dev qt6-multimedia-dev libsndfile1-dev libopenmpt-dev

# Build
make clean && make -j4
```

### Windows

1. Install Qt6 for Windows: https://www.qt.io/download
2. Install dependencies:
   - libsndfile: https://github.com/libsndfile/libsndfile/releases
   - libopenmpt: https://lib.openmpt.org/svn/tags/

3. Build using the provided script:
```cmd
build_windows.bat
```

Or using CMake:
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Supported Formats

| Format | Decoder |
|--------|---------|
| WAV, AIFF, PCM, FLAC, ALAC, MP3, AAC, OGG, WMA, M4A, MP2, MPC, OPUS, ASF | libsndfile |
| MOD, XM, S3M, IT, etc. | libopenmpt |

## Themes

- Classic - Default blue/cyan theme
- Neon Night - Purple/magenta dark theme
- Retro PC - Windows 95 teal style
- Matrix - Green monochrome
- Synthwave - 80s neon pink/purple
- Cherry - Dark red accents
- Ocean - Deep blue theme
- Sunset - Orange/brown warm tones
- Red Fox - Firefox orange theme

## License

GNU General Public License v3.0
