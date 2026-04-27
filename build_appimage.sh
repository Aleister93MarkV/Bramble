#!/bin/bash

# Crystal Audio Player - AppImage Build Script

set -e

APP="CrystalAudioPlayer"
VERSION="1.0.0"
ARCH="x86_64"

# Create temporary build directory
BUILD_DIR=$(mktemp -d)
cd "$BUILD_DIR"

echo "Building CrystalAudioPlayer AppImage..."
echo "Build directory: $BUILD_DIR"

# Install dependencies if needed
if ! command -v qmake6 &> /dev/null; then
    echo "Installing Qt6..."
    sudo apt-get update
    sudo apt-get install -y qt6-base-dev qt6-multimedia-dev
fi

if ! pkg-config --exists sndfile; then
    echo "Installing libsndfile..."
    sudo apt-get install -y libsndfile1-dev
fi

if ! pkg-config --exists libopenmpt; then
    echo "Installing libopenmpt..."
    sudo apt-get install -y libopenmpt-dev
fi

# Copy source files
echo "Copying source files..."
cp -r /home/aleister/Documentos/Prototype\ Music\ Player/PROJETOS/PLAYER/src_cpp ./src
cp -r /home/aleister/Documentos/Prototype\ Music\ Player/PROJETOS/PLAYER/include ./include
mkdir -p src/foobar2k_sdk

# Create project file
cat > CrystalAudioPlayer.pro << 'EOF'
QT += core gui widgets

TARGET = CrystalAudioPlayer
CONFIG += c++20

INCLUDEPATH += include

LIBS += -lsndfile -lopenmpt

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/AudioEngine.cpp \
    src/EqualizerWindow.cpp \
    src/VisualizerWindow.cpp \
    src/SkinWindow.cpp \
    src/foobar2k_sdk/dsp_manager.cpp \
    src/foobar2k_sdk/eq_dsp.cpp \
    src/foobar2k_sdk/crystallizer_dsp.cpp

HEADERS += \
    src/MainWindow.hpp \
    src/AudioEngine.hpp \
    src/EqualizerWindow.hpp \
    src/VisualizerWindow.hpp \
    src/SkinWindow.hpp \
    src/SkinManager.hpp \
    src/foobar2k_sdk/service_base.hpp \
    src/foobar2k_sdk/dsp.hpp \
    src/foobar2k_sdk/dsp_manager.hpp \
    src/foobar2k_sdk/eq_dsp.hpp \
    src/foobar2k_sdk/crystallizer_dsp.hpp

QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -ffast-math
EOF

# Build
echo "Building..."
qmake6 CrystalAudioPlayer.pro
make -j$(nproc)

# Create AppDir structure
echo "Creating AppImage..."
mkdir -p "$APP.AppDir/usr/bin"
mkdir -p "$APP.AppDir/usr/lib"
mkdir -p "$APP.AppDir/usr/share/applications"
mkdir -p "$APP.AppDir/usr/share/icons/hicolor/256x256/apps"

# Copy executable
cp CrystalAudioPlayer "$APP.AppDir/usr/bin/"

# Copy libraries
cp /usr/lib/x86_64-linux-gnu/libQt6Widgets.so.6 "$APP.AppDir/usr/lib/"
cp /usr/lib/x86_64-linux-gnu/libQt6Gui.so.6 "$APP.AppDir/usr/lib/"
cp /usr/lib/x86_64-linux-gnu/libQt6Core.so.6 "$APP.AppDir/usr/lib/"
cp /usr/lib/x86_64-linux-gnu/libsndfile.so.1 "$APP.AppDir/usr/lib/"
cp /usr/lib/x86_64-linux-gnu/libopenmpt.so.0 "$APP.AppDir/usr/lib/"

# Create AppRun
cat > "$APP.AppDir/AppRun" << 'EOF'
#!/bin/bash
export LD_LIBRARY_PATH="/usr/lib:$LD_LIBRARY_PATH"
exec "/usr/bin/CrystalAudioPlayer" "$@"
EOF
chmod +x "$APP.AppDir/AppRun"

# Create desktop file
cat > "$APP.AppDir/CrystalAudioPlayer.desktop" << 'EOF'
[Desktop Entry]
Name=Crystal Audio Player
Comment=Cross-platform audio player with skins
Exec=CrystalAudioPlayer
Icon=CrystalAudioPlayer
Terminal=false
Type=Application
Categories=Audio;Music;Player;AudioVideo;
EOF

# Download AppImage tool
echo "Downloading appimage tool..."
wget -q https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O appimagetool
chmod +x appimagetool

# Build AppImage
./appimagetool "$APP.AppDir" "/home/aleister/Documentos/Prototype Music Player/PROJETOS/PLAYER/CrystalAudioPlayer-${VERSION}-linux-${ARCH}.AppImage"

echo "Done! AppImage created."
