#define MINIAUDIO_IMPLEMENTATION
#include "../include/miniaudio.h"
#include "AudioEngine.hpp"
#include <cmath>
#include <cstring>
#include <cstddef>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <sndfile.h>

#ifdef WITH_OPENMPT
#include <libopenmpt/libopenmpt.hpp>
#include <libopenmpt/libopenmpt_version.h>
#endif

#ifdef WITH_CDIO
#include <cdio/cdio.h>
#include <cdio/paranoia/cdda.h>
#include <cdio/paranoia/paranoia.h>
#endif

#define CIRCULAR_BUFFER_FRAMES 131072

struct AudioEngine::Impl {
    AudioEngine* parent;
    
    SNDFILE* sndFile = nullptr;
    SF_INFO sfInfo;
    
#ifdef WITH_OPENMPT
    std::unique_ptr<openmpt::module> openmptModule;
#endif

#ifdef WITH_CDIO
    CdIo_t* cdio = nullptr;
    cdrom_drive_t* cddaDrive = nullptr;
    cdrom_paranoia_t* cddaParanoia = nullptr;
    int cddaFirstTrack = 0;
    int cddaLastTrack = 0;
    int cddaCurrentSector = 0;
    int cddaFirstSector = 0;
    int cddaTotalSectors = 0;
#endif

    ma_device device;
    bool isDeviceInitialized = false;
    
    std::vector<float> circularBuffer;
    size_t bufferWritePos = 0;
    size_t bufferReadPos = 0;
    size_t bufferAvailable = 0;
    std::mutex bufferMutex;
    
    std::thread decodeThread;
    std::atomic<bool> decoding{false};
    std::atomic<bool> fileEnded{false};
    std::atomic<bool> threadRunning{false};
    std::atomic<sf_count_t> lastPosition{0};
    
    float peakL = 0.0f;
    float peakR = 0.0f;
    
    enum class DecoderType {
        SNDFILE,
        OPENMPT,
#ifdef WITH_CDIO
        CDDA,
#endif
        UNKNOWN
    } decoderType = DecoderType::UNKNOWN;
};

AudioEngine::AudioEngine() : pImpl(std::make_unique<Impl>()) {
    pImpl->parent = this;
    
    pImpl->circularBuffer.resize(CIRCULAR_BUFFER_FRAMES * 2);
    
    m_eqDSP = std::make_shared<fb2k::EqDSP>();
    m_crystDSP = std::make_shared<fb2k::CrystallizerDSP>();
    
    m_dspManager = std::make_shared<fb2k::dsp_manager>();
    m_dspManager->add_dsp(m_eqDSP);
    m_dspManager->add_dsp(m_crystDSP);
}

AudioEngine::~AudioEngine() {
    stop();
    cleanupDecoder();
    
    if (pImpl->isDeviceInitialized) {
        ma_device_uninit(&pImpl->device);
        pImpl->isDeviceInitialized = false;
    }
}

void AudioEngine::cleanupDecoder() {
#ifdef WITH_OPENMPT
    pImpl->openmptModule.reset();
#endif
    
#ifdef WITH_CDIO
    if (pImpl->cddaDrive) {
        cdio_cddap_close(pImpl->cddaDrive);
        pImpl->cddaDrive = nullptr;
    }
    if (pImpl->cdio) {
        cdio_destroy(pImpl->cdio);
        pImpl->cdio = nullptr;
    }
#endif
    
    if (pImpl->sndFile) {
        sf_close(pImpl->sndFile);
        pImpl->sndFile = nullptr;
    }
    
    pImpl->decoderType = Impl::DecoderType::UNKNOWN;
    pImpl->sfInfo.format = 0;
    pImpl->sfInfo.samplerate = 44100;
    pImpl->sfInfo.channels = 2;
    pImpl->sfInfo.frames = 0;
    pImpl->lastPosition.store(0, std::memory_order_release);
}

std::string toLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) c = std::tolower(c);
    return result;
}

bool AudioEngine::loadFile(const std::string& path) {
    fprintf(stderr, "CDDA: loadFile called with: %s\n", path.c_str());
    stop();
    fprintf(stderr, "CDDA: stop() done\n");
    cleanupDecoder();
    fprintf(stderr, "CDDA: cleanupDecoder() done\n");
    
    if (path.find("/dev/") == 0) {
#ifdef WITH_CDIO
        bool result = loadCDDA(path);
        fprintf(stderr, "CDDA: loadCDDA returned %d\n", result);
        return result;
#else
        return false;
#endif
    }
    
    std::string ext;
    if (path.find('.') != std::string::npos) {
        ext = path.substr(path.find_last_of('.') + 1);
        ext = toLower(ext);
    }
    
    if (ext == "mod" || ext == "xm" || ext == "s3m" || ext == "it" || ext == "669" || 
        ext == "pat" || ext == "mts" || ext == "dbm" || ext == "mptm" || ext == "rt" ||
        ext == "s3m" || ext == "ftm" || ext == "digi" || ext == "emod" || ext == "okt" ||
        ext == "st26" || ext == "mt2" || ext == "gdm" || ext == "imf" || ext == "stx") {
#ifdef WITH_OPENMPT
        return loadOpenMPT(path);
#else
        return false;
#endif
    }
    
    return loadSndFile(path);
}

bool AudioEngine::loadSndFile(const std::string& path) {
    pImpl->sfInfo.format = 0;
    pImpl->sndFile = sf_open(path.c_str(), SFM_READ, &pImpl->sfInfo);
    
    if (!pImpl->sndFile) return false;
    if (pImpl->sfInfo.channels > 2) {
        sf_close(pImpl->sndFile);
        pImpl->sndFile = nullptr;
        return false;
    }
    
    pImpl->decoderType = Impl::DecoderType::SNDFILE;
    readSndFileMetadata();
    return true;
}

#ifdef WITH_OPENMPT
bool AudioEngine::loadOpenMPT(const std::string& path) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return false;
        
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<std::byte> data(size);
        if (!file.read(reinterpret_cast<char*>(data.data()), size)) return false;
        
        pImpl->openmptModule = std::make_unique<openmpt::module>(data);
        
        if (!pImpl->openmptModule) return false;
        
        pImpl->sfInfo.samplerate = 48000;
        pImpl->sfInfo.channels = 2;
        pImpl->sfInfo.frames = (sf_count_t)(pImpl->openmptModule->get_duration_seconds() * 48000);
        
        pImpl->decoderType = Impl::DecoderType::OPENMPT;
        readOpenMPTMetadata();
        return true;
    } catch (...) {
        pImpl->openmptModule.reset();
        return false;
    }
}
#endif

#ifdef WITH_CDIO
bool AudioEngine::loadCDDA(const std::string& path) {
    fprintf(stderr, "CDDA: Trying to open %s\n", path.c_str());
    
    std::string realPath = path;
    if (path == "/dev/cdrom") {
        char buf[256] = {0};
        ssize_t len = readlink("/dev/cdrom", buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            realPath = buf;
        } else {
            realPath = "/dev/sr0";
        }
    }
    
    fprintf(stderr, "CDDA: Using path %s\n", realPath.c_str());
    
    pImpl->cddaDrive = cdio_cddap_find_a_cdrom(2, NULL);
    if (!pImpl->cddaDrive) {
        fprintf(stderr, "CDDA: cdio_cddap_find_a_cdrom failed\n");
        return false;
    }
    fprintf(stderr, "CDDA: Found drive %p\n", (void*)pImpl->cddaDrive);
    
    int r = cdio_cddap_open(pImpl->cddaDrive);
    if (r != 0) {
        fprintf(stderr, "CDDA: cdio_cddap_open failed with %d\n", r);
        cdio_cddap_close(pImpl->cddaDrive);
        pImpl->cddaDrive = nullptr;
        return false;
    }
    fprintf(stderr, "CDDA: Drive opened\n");
    
    pImpl->cdio = cdio_open(realPath.c_str(), DRIVER_UNKNOWN);
    
    pImpl->cddaFirstTrack = cdio_cddap_track_firstsector(pImpl->cddaDrive, 0);
    pImpl->cddaLastTrack = cdio_cddap_tracks(pImpl->cddaDrive);
    fprintf(stderr, "CDDA: Tracks=%d\n", pImpl->cddaLastTrack);
    
    pImpl->cddaFirstSector = cdio_get_track_lsn(pImpl->cdio, cdio_get_first_track_num(pImpl->cdio));
    
    pImpl->cddaTotalSectors = 0;
    for (int i = 0; i < pImpl->cddaLastTrack; ++i) {
        lsn_t first = cdio_get_track_lsn(pImpl->cdio, i + 1);
        lsn_t last = cdio_get_track_last_lsn(pImpl->cdio, i + 1);
        pImpl->cddaTotalSectors += (last - first + 1);
    }
    
    pImpl->sfInfo.samplerate = 44100;
    pImpl->sfInfo.channels = 2;
    pImpl->sfInfo.frames = pImpl->cddaTotalSectors;
    pImpl->cddaCurrentSector = pImpl->cddaFirstSector;
    
    pImpl->decoderType = Impl::DecoderType::CDDA;
    readCDDAMetadata();
    fprintf(stderr, "CDDA: Load complete, %d sectors\n", pImpl->cddaTotalSectors);
    return true;
}

void AudioEngine::readCDDAMetadata() {
    m_metadata.title = "Audio CD";
    m_metadata.artist = "Unknown Artist";
    m_metadata.album = "Audio CD";
    m_metadata.format = "CDDA";
    m_metadata.sampleRate = "44100";
    m_metadata.bitDepth = "16";
    m_metadata.channels = "2";
    
    char trackInfo[256];
    snprintf(trackInfo, sizeof(trackInfo), "Tracks: %d-%d (%d sectors)", 
             pImpl->cddaLastTrack, pImpl->cddaFirstTrack, pImpl->cddaTotalSectors);
    m_metadata.trackNumber = trackInfo;
}
#endif

void AudioEngine::decodeLoop() {
    fprintf(stderr, "CDDA: decodeLoop started\n");
    fflush(stderr);
    const size_t chunkSize = 4096;
    
    std::vector<float> tempBufferLeft(chunkSize);
    std::vector<float> tempBufferRight(chunkSize);
    
    pImpl->fileEnded.store(false, std::memory_order_release);
    
    while (pImpl->decoding.load(std::memory_order_acquire)) {
        if (pImpl->fileEnded.load(std::memory_order_acquire)) break;
        
        {
            std::unique_lock<std::mutex> lock(pImpl->bufferMutex);
            if (CIRCULAR_BUFFER_FRAMES - pImpl->bufferAvailable < chunkSize) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
        }
        
        sf_count_t framesRead = 0;
        
        switch (pImpl->decoderType) {
#ifdef WITH_OPENMPT
            case Impl::DecoderType::OPENMPT:
                if (pImpl->openmptModule) {
                    framesRead = pImpl->openmptModule->read(48000, chunkSize, tempBufferLeft.data(), tempBufferRight.data());
                }
                break;
#endif
#ifdef WITH_CDIO
            case Impl::DecoderType::CDDA:
                if (pImpl->cddaDrive) {
                    framesRead = 0;
                    int16_t buffer[CDIO_CD_FRAMESIZE_RAW];
                    for (int s = 0; s < 8 && framesRead < (sf_count_t)chunkSize; ++s) {
                        long n = cdio_cddap_read(pImpl->cddaDrive, buffer, pImpl->cddaCurrentSector, 1);
                        if (n > 0) {
                            for (int i = 0; i < 588 && framesRead < (sf_count_t)chunkSize; ++i) {
                                tempBufferLeft[framesRead] = buffer[i * 2] / 32768.0f;
                                tempBufferRight[framesRead] = buffer[i * 2 + 1] / 32768.0f;
                                framesRead++;
                            }
                            pImpl->cddaCurrentSector++;
                            if (pImpl->cddaCurrentSector >= pImpl->cddaFirstSector + pImpl->cddaTotalSectors) {
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                    if (framesRead == 0) {
                        pImpl->fileEnded.store(true, std::memory_order_release);
                    }
                }
                break;
#endif
            default:
                if (pImpl->sndFile) {
                    std::vector<float> tempInterleaved(chunkSize * 2);
                    framesRead = sf_readf_float(pImpl->sndFile, tempInterleaved.data(), chunkSize);
                    for (sf_count_t i = 0; i < framesRead; ++i) {
                        tempBufferLeft[i] = tempInterleaved[i * 2];
                        tempBufferRight[i] = tempInterleaved[i * 2 + 1];
                    }
                }
                break;
        }
        
        if (framesRead <= 0) {
            pImpl->fileEnded.store(true, std::memory_order_release);
            break;
        }
        
        std::lock_guard<std::mutex> lock(pImpl->bufferMutex);
        
        for (sf_count_t i = 0; i < framesRead; ++i) {
            size_t idx = (pImpl->bufferWritePos + i) % CIRCULAR_BUFFER_FRAMES;
            pImpl->circularBuffer[idx * 2] = tempBufferLeft[i];
            pImpl->circularBuffer[idx * 2 + 1] = tempBufferRight[i];
        }
        
        pImpl->bufferWritePos = (pImpl->bufferWritePos + framesRead) % CIRCULAR_BUFFER_FRAMES;
        pImpl->bufferAvailable += framesRead;
    }
    
    pImpl->threadRunning.store(false, std::memory_order_release);
}

void AudioEngine::play() {
    fprintf(stderr, "CDDA: play() called, decoderType=%d\n", (int)pImpl->decoderType);
    if (!hasValidDecoder()) return;
    
    if (m_isPlaying.load(std::memory_order_acquire)) return;
    
    if (pImpl->isDeviceInitialized) {
        if (ma_device_is_started(&pImpl->device)) ma_device_stop(&pImpl->device);
        ma_device_uninit(&pImpl->device);
        pImpl->isDeviceInitialized = false;
    }
    
    pImpl->decoding.store(false, std::memory_order_release);
    if (pImpl->threadRunning.load(std::memory_order_acquire) && pImpl->decodeThread.joinable()) {
        pImpl->decodeThread.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pImpl->threadRunning.store(false, std::memory_order_release);
    
    {
        std::lock_guard<std::mutex> lock(pImpl->bufferMutex);
        pImpl->bufferWritePos = 0;
        pImpl->bufferReadPos = 0;
        pImpl->bufferAvailable = 0;
    }
    pImpl->fileEnded.store(false, std::memory_order_release);
    
    bool wasAtEnd = pImpl->lastPosition.load(std::memory_order_acquire) >= (sf_count_t)pImpl->sfInfo.frames;
    if (wasAtEnd) {
        pImpl->lastPosition.store(0, std::memory_order_release);
    }
    
    sf_count_t startPos = pImpl->lastPosition.load(std::memory_order_acquire);
    
    switch (pImpl->decoderType) {
#ifdef WITH_OPENMPT
        case Impl::DecoderType::OPENMPT:
            if (pImpl->openmptModule) {
                pImpl->openmptModule->set_position_seconds(startPos / (double)pImpl->sfInfo.samplerate);
            }
            break;
#endif
#ifdef WITH_CDIO
        case Impl::DecoderType::CDDA:
            if (pImpl->cddaParanoia) {
                paranoia_seek(pImpl->cddaParanoia, pImpl->cddaCurrentSector, SEEK_SET);
            }
            break;
#endif
        default:
            if (pImpl->sndFile) sf_seek(pImpl->sndFile, startPos, SEEK_SET);
            break;
    }
    
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = pImpl->sfInfo.samplerate;
    config.dataCallback = audioCallback;
    config.pUserData = this;
    config.periodSizeInFrames = 1024;
    
    if (ma_device_init(NULL, &config, &pImpl->device) != MA_SUCCESS) return;
    pImpl->isDeviceInitialized = true;
    
    pImpl->decoding.store(true, std::memory_order_release);
    pImpl->threadRunning.store(true, std::memory_order_release);
    pImpl->decodeThread = std::thread(&AudioEngine::decodeLoop, this);
    
    if (ma_device_start(&pImpl->device) != MA_SUCCESS) {
        pImpl->decoding.store(false, std::memory_order_release);
        return;
    }
    
    m_isPlaying.store(true, std::memory_order_release);
}

void AudioEngine::pause() {
    if (!m_isPlaying.exchange(false)) return;
    
    if (pImpl->isDeviceInitialized && ma_device_is_started(&pImpl->device)) {
        ma_device_stop(&pImpl->device);
    }
}

void AudioEngine::stop() {
    bool wasPlaying = m_isPlaying.load(std::memory_order_acquire);
    m_isPlaying.store(false, std::memory_order_release);
    
    if (pImpl->isDeviceInitialized && ma_device_is_started(&pImpl->device)) {
        ma_device_stop(&pImpl->device);
    }
    
    pImpl->decoding.store(false, std::memory_order_release);
    
    if (pImpl->threadRunning.load(std::memory_order_acquire) && pImpl->decodeThread.joinable()) {
        pImpl->decodeThread.join();
    }
    pImpl->threadRunning.store(false, std::memory_order_release);
    
    {
        std::lock_guard<std::mutex> lock(pImpl->bufferMutex);
        pImpl->bufferWritePos = 0;
        pImpl->bufferReadPos = 0;
        pImpl->bufferAvailable = 0;
    }
    pImpl->fileEnded.store(false, std::memory_order_release);
    pImpl->lastPosition.store(0, std::memory_order_release);
    
    if (pImpl->decoderType == Impl::DecoderType::SNDFILE && pImpl->sndFile) {
        sf_seek(pImpl->sndFile, 0, SEEK_SET);
    }
    
#ifdef WITH_OPENMPT
    if (pImpl->decoderType == Impl::DecoderType::OPENMPT && pImpl->openmptModule) {
        pImpl->openmptModule->set_position_seconds(0);
    }
#endif
}

float AudioEngine::getPosition() const {
    if (pImpl->sfInfo.samplerate == 0) return 0.0f;
    return pImpl->lastPosition.load(std::memory_order_acquire) / pImpl->sfInfo.samplerate;
}

void AudioEngine::setPosition(float seconds) {
    if (!hasValidDecoder()) return;
    
    bool wasPlaying = m_isPlaying.load(std::memory_order_acquire);
    if (wasPlaying) pause();
    
    pImpl->decoding.store(false, std::memory_order_release);
    if (pImpl->threadRunning.load(std::memory_order_acquire) && pImpl->decodeThread.joinable()) {
        pImpl->decodeThread.join();
    }
    pImpl->threadRunning.store(false, std::memory_order_release);
    
    sf_count_t frame = (sf_count_t)(seconds * pImpl->sfInfo.samplerate);
    
    switch (pImpl->decoderType) {
#ifdef WITH_OPENMPT
        case Impl::DecoderType::OPENMPT:
            if (pImpl->openmptModule) {
                pImpl->openmptModule->set_position_seconds(frame / (double)pImpl->sfInfo.samplerate);
            }
            break;
#endif
#ifdef WITH_CDIO
        case Impl::DecoderType::CDDA:
            if (pImpl->cddaParanoia && seconds < pImpl->sfInfo.frames / 44100.0) {
                lsn_t targetSector = (lsn_t)(seconds * 75);
                if (targetSector >= pImpl->cddaFirstSector && 
                    targetSector < pImpl->cddaFirstSector + pImpl->cddaTotalSectors) {
                    pImpl->cddaCurrentSector = targetSector;
                    paranoia_seek(pImpl->cddaParanoia, targetSector, SEEK_SET);
                }
            }
            break;
#endif
        default:
            if (pImpl->sndFile) sf_seek(pImpl->sndFile, frame, SEEK_SET);
            break;
    }
    
    {
        std::lock_guard<std::mutex> lock(pImpl->bufferMutex);
        pImpl->bufferReadPos = 0;
        pImpl->bufferWritePos = 0;
        pImpl->bufferAvailable = 0;
    }
    pImpl->lastPosition.store(frame, std::memory_order_release);
    pImpl->fileEnded.store(false, std::memory_order_release);
    
    if (wasPlaying) play();
}

float AudioEngine::getDuration() const {
    return (float)pImpl->sfInfo.frames / pImpl->sfInfo.samplerate;
}

bool AudioEngine::hasValidDecoder() const {
    switch (pImpl->decoderType) {
#ifdef WITH_OPENMPT
        case Impl::DecoderType::OPENMPT:
            return (bool)pImpl->openmptModule;
#endif
        default:
            return pImpl->sndFile != nullptr;
    }
}

void AudioEngine::setEQBand(int index, float gain) {
    if (m_eqDSP) m_eqDSP->set_band(index, gain);
}

void AudioEngine::setCrystallizer(float amount) {
    if (m_crystDSP) m_crystDSP->set_amount(amount);
}

void AudioEngine::getLevels(float& left, float& right) {
    left = pImpl->peakL;
    right = pImpl->peakR;
}

void AudioEngine::getSpectrum(std::vector<float>& outSpectrum) {
    outSpectrum.assign(128, 0.0f);
    for (int i = 0; i < 128; ++i) {
        outSpectrum[i] = std::abs(m_lastSamples[i * 4]) * 0.5f;
    }
}

void AudioEngine::getWaveform(std::vector<float>& outWaveform) {
    outWaveform.assign(512, 0.0f);
    for (int i = 0; i < 512; ++i) outWaveform[i] = m_lastSamples[i];
}

void AudioEngine::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    (void)pInput;
    AudioEngine* engine = (AudioEngine*)pDevice->pUserData;
    engine->processAudioFrames((float*)pOutput, frameCount);
}

void AudioEngine::processAudioFrames(float* output, int frameCount) {
    Impl* impl = pImpl.get();
    int channels = 2;
    
    if (!m_isPlaying.load(std::memory_order_acquire)) {
        std::memset(output, 0, frameCount * channels * sizeof(float));
        return;
    }
    
    std::unique_lock<std::mutex> lock(impl->bufferMutex);
    
    if (impl->bufferAvailable == 0) {
        lock.unlock();
        
        if (impl->fileEnded.load(std::memory_order_acquire)) {
            m_isPlaying.store(false, std::memory_order_release);
        }
        std::memset(output, 0, frameCount * channels * sizeof(float));
        return;
    }
    
    int framesToRead = (impl->bufferAvailable >= (size_t)frameCount) ? frameCount : (int)impl->bufferAvailable;
    
    for (int i = 0; i < framesToRead; ++i) {
        size_t idx = (impl->bufferReadPos + i) % CIRCULAR_BUFFER_FRAMES;
        output[i * 2] = impl->circularBuffer[idx * 2];
        output[i * 2 + 1] = impl->circularBuffer[idx * 2 + 1];
    }
    
    impl->bufferReadPos = (impl->bufferReadPos + framesToRead) % CIRCULAR_BUFFER_FRAMES;
    impl->bufferAvailable -= framesToRead;
    
    static int frameCounter = 0;
    frameCounter += framesToRead;
    if (frameCounter >= 4800) {
        frameCounter = 0;
        impl->lastPosition.fetch_add(framesToRead, std::memory_order_release);
    }
    
    lock.unlock();
    
    if (m_dspManager) {
        m_dspManager->process_chunk(output, framesToRead, channels, impl->sfInfo.samplerate);
    }
    
    float vol = m_volume.load(std::memory_order_acquire);
    
    for (int i = 0; i < framesToRead; ++i) {
        for (int ch = 0; ch < channels; ++ch) {
            float sample = output[i * channels + ch] * vol;
            output[i * channels + ch] = sample;
            
            float absSample = std::abs(sample);
            if (ch == 0) {
                if (absSample > impl->peakL) impl->peakL = absSample;
            } else if (ch == 1) {
                if (absSample > impl->peakR) impl->peakR = absSample;
            }
            
            if (ch == 0 && (i % 4) == 0) {
                int idx = m_sampleIdx.fetch_add(1) % 512;
                m_lastSamples[idx] = sample;
            }
        }
    }
    
    impl->peakL *= 0.95f;
    impl->peakR *= 0.95f;
    
            if (framesToRead < frameCount) {
        std::memset(&output[framesToRead * channels], 0, (frameCount - framesToRead) * channels * sizeof(float));
        
        if (impl->fileEnded.load(std::memory_order_acquire)) {
            m_isPlaying.store(false, std::memory_order_release);
            impl->lastPosition.store(0, std::memory_order_release);
        }
    }
}

void AudioEngine::readSndFileMetadata() {
    m_metadata = AudioMetadata();
    if (!pImpl->sndFile) return;
    
    const char* title = sf_get_string(pImpl->sndFile, SF_STR_TITLE);
    const char* artist = sf_get_string(pImpl->sndFile, SF_STR_ARTIST);
    const char* album = sf_get_string(pImpl->sndFile, SF_STR_ALBUM);
    const char* genre = sf_get_string(pImpl->sndFile, SF_STR_GENRE);
    const char* date = sf_get_string(pImpl->sndFile, SF_STR_DATE);
    const char* track = sf_get_string(pImpl->sndFile, SF_STR_TRACKNUMBER);
    
    if (title) m_metadata.title = title;
    if (artist) m_metadata.artist = artist;
    if (album) m_metadata.album = album;
    if (genre) m_metadata.genre = genre;
    if (date) m_metadata.year = date;
    if (track) m_metadata.trackNumber = track;
    
    // Technical info
    m_metadata.channels = std::to_string(pImpl->sfInfo.channels) + " ch";
    m_metadata.sampleRate = std::to_string(pImpl->sfInfo.samplerate / 1000.0) + " kHz";
    
    int format = pImpl->sfInfo.format;
    int subformat = format & SF_FORMAT_SUBMASK;
    switch (subformat) {
        case SF_FORMAT_PCM_S8: case SF_FORMAT_PCM_U8: m_metadata.bitDepth = "8-bit"; break;
        case SF_FORMAT_PCM_16: m_metadata.bitDepth = "16-bit"; break;
        case SF_FORMAT_PCM_24: m_metadata.bitDepth = "24-bit"; break;
        case SF_FORMAT_PCM_32: m_metadata.bitDepth = "32-bit"; break;
        case SF_FORMAT_FLOAT: m_metadata.bitDepth = "32-bit float"; break;
        default: m_metadata.bitDepth = "Unknown";
    }
    
    switch (format & SF_FORMAT_TYPEMASK) {
        case SF_FORMAT_WAV: m_metadata.format = "WAV"; break;
        case SF_FORMAT_FLAC: m_metadata.format = "FLAC"; break;
        case SF_FORMAT_AIFF: m_metadata.format = "AIFF"; break;
        case SF_FORMAT_OGG: m_metadata.format = "OGG"; break;
        default: m_metadata.format = "Unknown";
    }
}

#ifdef WITH_OPENMPT
void AudioEngine::readOpenMPTMetadata() {
    m_metadata = AudioMetadata();
    if (!pImpl->openmptModule) return;
    
    try {
        // Try multiple metadata keys for each field
        auto getMeta = [this](const std::vector<std::string>& keys) -> std::string {
            for (const auto& key : keys) {
                try {
                    std::string val = pImpl->openmptModule->get_metadata(key);
                    if (!val.empty()) return val;
                } catch (...) {}
            }
            return "";
        };
        
        std::string title = getMeta({"title", "TITL", "NAME", "name", "song_name"});
        std::string artist = getMeta({"artist", "AUTH", "AUTHOR", "composer", "COMM"});
        std::string album = getMeta({"album", "ALBM", "collection"});
        std::string genre = getMeta({"genre", "GENRE", "style", "type"});
        std::string year = getMeta({"date", "YEAR", "year", "time", "date"});
        std::string track = getMeta({"track", "TRACK", "tracknumber", "TRK"});
        std::string comment = getMeta({"comment", "COMMENT", "msg", "message"});
        
        if (!title.empty()) m_metadata.title = title;
        if (!artist.empty()) m_metadata.artist = artist;
        if (!album.empty()) m_metadata.album = album;
        if (!genre.empty()) m_metadata.genre = genre;
        if (!year.empty()) m_metadata.year = year;
        if (!track.empty()) m_metadata.trackNumber = track;
        
        // Get additional module info
        try {
            std::string type = pImpl->openmptModule->get_metadata("type");
            if (!type.empty()) m_metadata.format = type;
            else m_metadata.format = "Module";
        } catch (...) {
            m_metadata.format = "Module";
        }
        
        // Technical info for modules
        m_metadata.channels = "4-8 ch";  // Typical Amiga module channels
        m_metadata.sampleRate = "48 kHz";
        m_metadata.bitDepth = "8-16 bit";
        
        // Add comment if available
        if (!comment.empty() && m_metadata.title.empty()) {
            m_metadata.title = comment.substr(0, 50);
        }
    } catch (...) {}
}
#endif

std::string AudioEngine::getFormattedMetadata() const {
    std::string result;
    if (!m_metadata.title.empty()) result += "Título: " + m_metadata.title + "\n";
    if (!m_metadata.artist.empty()) result += "Artista: " + m_metadata.artist + "\n";
    if (!m_metadata.album.empty()) result += "Álbum: " + m_metadata.album + "\n";
    if (!m_metadata.genre.empty()) result += "Gênero: " + m_metadata.genre + "\n";
    if (!m_metadata.year.empty()) result += "Ano: " + m_metadata.year + "\n";
    if (!m_metadata.trackNumber.empty()) result += "Faixa: " + m_metadata.trackNumber + "\n";
    
    // Technical info
    std::string techInfo;
    if (!m_metadata.format.empty()) techInfo += m_metadata.format;
    if (!m_metadata.channels.empty()) techInfo += " | " + m_metadata.channels;
    if (!m_metadata.sampleRate.empty()) techInfo += " | " + m_metadata.sampleRate;
    if (!m_metadata.bitDepth.empty()) techInfo += " | " + m_metadata.bitDepth;
    if (!techInfo.empty()) result += techInfo;
    
    if (result.empty()) {
        return "Informações não disponíveis";
    }
    return result;
}

#ifdef WITH_CDIO
int AudioEngine::getCDTrackCount() const {
    if (pImpl->decoderType != Impl::DecoderType::CDDA) return 0;
    return pImpl->cddaLastTrack - pImpl->cddaFirstTrack + 1;
}

std::string AudioEngine::getCDTrackInfo(int track) const {
    if (pImpl->decoderType != Impl::DecoderType::CDDA) return "";
    if (track < 0 || track >= getCDTrackCount()) return "";
    
    int trackNum = pImpl->cddaFirstTrack + track;
    lsn_t firstLsn = cdio_get_track_lsn(pImpl->cdio, trackNum);
    lsn_t lastLsn = cdio_get_track_last_lsn(pImpl->cdio, trackNum);
    int sectors = lastLsn - firstLsn + 1;
    int seconds = sectors / 75;
    
    return "Track " + std::to_string(trackNum) + ": " + std::to_string(seconds / 60) + ":" + std::to_string(seconds % 60);
}

void AudioEngine::setCDTrack(int trackIndex) {
    if (pImpl->decoderType != Impl::DecoderType::CDDA) return;
    
    int trackCount = getCDTrackCount();
    if (trackIndex < 0 || trackIndex >= trackCount) return;
    
    int trackNum = pImpl->cddaFirstTrack + trackIndex;
    pImpl->cddaCurrentSector = cdio_get_track_lsn(pImpl->cdio, trackNum);
    
    bool wasPlaying = m_isPlaying.load(std::memory_order_acquire);
    stop();
    
    if (wasPlaying) {
        play();
    }
}
#endif