Name:           bramble-player
Version:        1.0.0
Release:        1%{?dist}
Summary:        Cross-platform audio player with skins
License:        GPL-3.0-or-later
URL:            https://github.com/bramble/bramble
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtmultimedia-devel
BuildRequires:  libsndfile-devel
BuildRequires:  libopenmpt-devel
BuildRequires:  gcc-c++
BuildRequires:  make

%description
Bramble is a modern cross-platform audio player 
with built-in equalizer, crystallizer effect, visualizer, and 
customizable skins.

Supports: WAV, MP3, FLAC, OGG, MOD, XM, S3M, IT

%prep
%autosetup -p1

%build
qmake6 PREFIX=%{_prefix} Bramble.pro
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}

# Install desktop file
install -Dm644 %{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop

# Install icon
install -Dm644 icon.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/%{name}.png

%files
%{_bindir}/bramble
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png
%{_mandir}/man1/crystalplayer.1*

%changelog
* Mon Apr 27 2026 Bramble <contact@bramble.player> - 1.0.0
- Initial release
- Multi-format audio playback
- 18-band equalizer
- Crystallizer effect
- Audio visualizer
- Skin system with 9 themes
- Album art detection
