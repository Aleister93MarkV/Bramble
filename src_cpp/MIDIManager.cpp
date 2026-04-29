#include "MIDIManager.hpp"
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>

MIDIManager::MIDIManager() {
    m_settings = new_fluid_settings();
    
    const char* drivers[] = {"pipewire", "pulseaudio", "alsa", "sdl", "jack", "file"};
    for (const char* drv : drivers) {
        if (fluid_settings_setstr(m_settings, "audio.driver", drv) == FLUID_OK) {
            std::cout << "MIDI: Using audio driver: " << drv << std::endl;
            break;
        }
    }
    
    m_synth = new_fluid_synth(m_settings);
    fluid_synth_set_polyphony(m_synth, 64);
    
    m_player = new_fluid_player(m_synth);
}

MIDIManager::~MIDIManager() {
    stop();
    
    if (m_player) {
        delete_fluid_player(m_player);
    }
    if (m_synth) {
        delete_fluid_synth(m_synth);
    }
    if (m_settings) {
        delete_fluid_settings(m_settings);
    }
}

bool MIDIManager::loadSoundFont(const QString& sfPath) {
    if (!m_synth) return false;
    
    int id = fluid_synth_sfload(m_synth, sfPath.toUtf8().constData(), 1);
    if (id < 0) {
        std::cerr << "Failed to load soundfont: " << sfPath.toStdString() << std::endl;
        return false;
    }
    
    std::cout << "MIDI: Loaded soundfont ID: " << id << std::endl;
    return true;
}

bool MIDIManager::loadMIDI(const std::string& midiPath) {
    stop();
    
    if (m_player) {
        fluid_player_stop(m_player);
        fluid_player_join(m_player);
    }
    
    if (fluid_player_add(m_player, midiPath.c_str()) != FLUID_OK) {
        std::cerr << "Failed to load MIDI: " << midiPath << std::endl;
        return false;
    }
    
    m_duration = 180.0;
    m_position.store(0.0);
    m_currentMIDI = midiPath;
    m_loaded.store(true);
    
    std::cout << "MIDI: Loaded: " << midiPath << std::endl;
    return true;
}

void MIDIManager::play() {
    if (!m_loaded.load() || !m_player) return;
    
    fluid_player_play(m_player);
    m_playing.store(true);
    
    std::thread([this]() {
        while (m_playing.load()) {
            if (fluid_player_get_status(m_player) == FLUID_PLAYER_PLAYING) {
                m_position.store(m_position.load() + 0.01);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }).detach();
}

void MIDIManager::stop() {
    m_playing.store(false);
    if (m_player) {
        fluid_player_stop(m_player);
        fluid_player_seek(m_player, 0);
    }
    m_position.store(0.0);
}

void MIDIManager::seek(double seconds) {
    if (m_player) {
        int ticks = static_cast<int>(seconds * 480);
        fluid_player_seek(m_player, ticks);
        m_position.store(seconds);
    }
}

void MIDIManager::setPlaying(bool playing) {
    m_playing.store(playing);
}

void MIDIManager::getAudio(float* buffer, int frameCount) {
    if (!m_loaded.load() || !m_synth) {
        std::memset(buffer, 0, frameCount * 2 * sizeof(float));
        return;
    }
    
    if (m_playing.load()) {
        fluid_synth_write_float(m_synth, frameCount, buffer, 0, 2, buffer, 1, 2);
    } else {
        std::memset(buffer, 0, frameCount * 2 * sizeof(float));
    }
}