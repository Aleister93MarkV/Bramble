#!/bin/bash

# Crystal Audio Player - Build All Packages Script
# Generates AppImage, Flatpak, .deb and .rpm packages

set -e

VERSION="1.0.0"
ARCH="x86_64"
DIST="$(lsb_release -is 2>/dev/null || echo 'Unknown')"

echo "======================================"
echo "Crystal Audio Player - Package Builder"
echo "======================================"
echo "Version: $VERSION"
echo "Distribution: $DIST"
echo ""

# Check if running as root (needed for some operations)
if [ "$EUID" -eq 0 ]; then
    echo "Warning: Running as root"
fi

# Function to build AppImage
build_appimage() {
    echo "[1/4] Building AppImage..."
    if command -v appimagetool &> /dev/null; then
        ./build_appimage.sh
        echo "AppImage: Done!"
    else
        echo "AppImage tool not available. Skipping..."
    fi
}

# Function to build Flatpak
build_flatpak() {
    echo "[2/4] Building Flatpak..."
    if command -v flatpak-builder &> /dev/null; then
        flatpak-builder --user --install build-flatpak com.crystalaudio.player.json
        flatpak build-export repo-flatpak build-flatpak
        echo "Flatpak: Done!"
    else
        echo "flatpak-builder not available. Install with: sudo apt install flatpak-builder"
        echo "Skipping..."
    fi
}

# Function to build .deb
build_deb() {
    echo "[3/4] Building .deb package..."
    if command -v dpkg-buildpackage &> /dev/null; then
        # Install build dependencies
        sudo apt-get install -y \
            qt6-base-dev \
            qt6-multimedia-dev \
            libsndfile1-dev \
            libopenmpt-dev \
            debhelper
            
        dpkg-buildpackage -b -uc -us
        echo ".deb package created in parent directory!"
    else
        echo "dpkg-buildpackage not available. Skipping..."
    fi
}

# Function to build .rpm
build_rpm() {
    echo "[4/4] Building .rpm package..."
    if command -v rpmbuild &> /dev/null; then
        # Install build dependencies
        sudo dnf install -y \
            qt6-qtbase-devel \
            qt6-qtmultimedia-devel \
            libsndfile-devel \
            libopenmpt-devel \
            gcc-c++ \
            make
            
        rpmbuild -ba crystal-audio-player.spec
        echo ".rpm package created in ~/rpmbuild/RPMS/"
    else
        echo "rpmbuild not available. Install with: sudo dnf install rpm-build"
        echo "Skipping..."
    fi
}

# Parse arguments
case "${1:-all}" in
    appimage)
        build_appimage
        ;;
    flatpak)
        build_flatpak
        ;;
    deb)
        build_deb
        ;;
    rpm)
        build_rpm
        ;;
    all)
        build_appimage
        build_flatpak
        build_deb
        build_rpm
        ;;
    help)
        echo "Usage: $0 [appimage|flatpak|deb|rpm|all]"
        echo ""
        echo "Examples:"
        echo "  $0              # Build all packages"
        echo "  $0 appimage     # Build only AppImage"
        echo "  $0 deb          # Build only .deb package"
        ;;
    *)
        echo "Unknown option: $1"
        echo "Use: $0 help"
        exit 1
        ;;
esac

echo ""
echo "======================================"
echo "Build process completed!"
echo "======================================"
