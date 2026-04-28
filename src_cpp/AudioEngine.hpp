#ifndef AUDIOENGINE_HPP
#define AUDIOENGINE_HPP

#include <vector>
#include <string>
#include <memory>
#include <atomic>

#include <miniaudio.h>

#include "foobar2k_sdk/eq_dsp.hpp"
#include "foobar2k_sdk/crystallizer_dsp.hpp"
#include "foobar2k_sdk/dsp_manager.hpp"

#define WITH_OPENMPT

struct AudioMetadata {
    std::string title;
    std::string artist;
    std::string album;
    std::string genre;
    std::string year;
    std::string trackNumber;
    std::string format;
};

class AudioEngine {
public:
    bool hasValidDecoder() const;

    AudioEngine();
    ~AudioEngine();

    bool loadFile(const std::string& path);
    void play();
    void pause();
    void stop();
    
    bool isPlaying() const { return m_isPlaying.load(std::memory_order_acquire); }
    float getPosition() const;
    void setPosition(float seconds);
    float getDuration() const;

    void setVolume(float volume) { m_volume.store(volume, std::memory_order_release); }
    void setEQBand(int index, float gain);
    void setCrystallizer(float amount);

    void getSpectrum(std::vector<float>& outSpectrum);
    void getWaveform(std::vector<float>& outWaveform);
    void getLevels(float& left, float& right);

    AudioMetadata getMetadata() const { return m_metadata; }
    std::string getFormattedMetadata() const;
    
    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

private:
    void processAudioFrames(float* output, int frameCount);
    void decodeLoop();
    void cleanupDecoder();
    
    bool loadSndFile(const std::string& path);
    void readSndFileMetadata();
    
#ifdef WITH_OPENMPT
    bool loadOpenMPT(const std::string& path);
    void readOpenMPTMetadata();
#endif

    struct Impl;
    std::unique_ptr<Impl> pImpl;

    std::atomic<float> m_volume{1.0f};
    std::atomic<bool> m_isPlaying{false};

    std::shared_ptr<fb2k::EqDSP> m_eqDSP;
    std::shared_ptr<fb2k::CrystallizerDSP> m_crystDSP;
    std::shared_ptr<fb2k::dsp_manager> m_dspManager;
    
    float m_lastSamples[512] = {0};
    std::atomic<int> m_sampleIdx{0};
    float m_peakL = 0.0f;
    float m_peakR = 0.0f;

    AudioMetadata m_metadata;
};

#endif // AUDIOENGINE_HPP