#ifndef MIDIMANAGER_HPP
#define MIDIMANAGER_HPP

#include <fluidsynth.h>
#include <QString>
#include <atomic>
#include <string>

class MIDIManager {
public:
    MIDIManager();
    ~MIDIManager();
    
    bool loadSoundFont(const QString& sfPath);
    bool loadMIDI(const std::string& midiPath);
    void play();
    void stop();
    void seek(double seconds);
    bool isPlaying() const { return m_playing.load(); }
    bool isLoaded() const { return m_loaded.load(); }
    double getDuration() const { return m_duration; }
    double getPosition() const { return m_position.load(); }
    void getAudio(float* buffer, int frameCount);
    void setPlaying(bool playing);

private:
    fluid_settings_t* m_settings = nullptr;
    fluid_synth_t* m_synth = nullptr;
    fluid_player_t* m_player = nullptr;
    
    std::atomic<bool> m_playing{false};
    std::atomic<bool> m_loaded{false};
    std::atomic<double> m_position{0.0};
    
    double m_duration = 0.0;
    std::string m_currentMIDI;
};

class MIDIManagerSingleton {
public:
    static MIDIManager& instance() {
        static MIDIManager inst;
        return inst;
    }
private:
    MIDIManagerSingleton() = default;
};

#endif // MIDIMANAGER_HPP