# Bramble

A cross-platform audio player with skins support, built with Qt6 and miniaudio.

## Features

- **Multiple Audio Formats**: WAV, AIFF, PCM, FLAC, ALAC, MP3, AAC, OGG, WMA, M4A, MP2, MPC, OPUS, ASF, MOD, XM, S3M, IT
- **CD Audio Playback**: Play audio CDs directly with libcdio/paranoia support
- **18-Band Equalizer**: Full parametric equalizer with visual sliders
- **Crystallizer Effect**: Psychoacoustic bass enhancement
- **Visualizer**: Real-time audio visualization
- **Skin System**: 9 built-in themes (Classic, Neon Night, Retro PC, Matrix, Synthwave, Cherry, Ocean, Sunset, Red Fox)
- **Album Art**: Automatic cover detection from music directory + MusicBrainz download
- **File Browser**: Integrated directory navigation with playlist view

## Requirements

### Linux
- Qt6 (Qt6 Widgets, Qt6 Gui, Qt6 Core, Qt6 Network)
- libsndfile
- libopenmpt
- libcdio (for CD Audio)

### Windows
- Qt6 for Windows (MinGW or MSVC)
- libsndfile for Windows
- libopenmpt for Windows

## Building

### Linux

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-network-dev \
    libsndfile1-dev libopenmpt-dev libcdio-dev libcdio-paranoia-dev

# Build with CMake
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j4
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
| Audio CD (CDDA) | libcdio |

## Usage

- **File → Open File**: Open a single audio file
- **File → Open Folder**: Open a folder and browse audio files
- **File → Play Audio CD**: Play an audio CD from CD-ROM drive

Double-click files in the browser to play them. Click tracks in the playlist to switch between them.

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