#include "eq_dsp.hpp"

namespace fb2k {

EqDSP::EqDSP() {
    for (int i = 0; i < 18; ++i) {
        m_eqBands[i].store(0.0f);
    }
}

void EqDSP::set_band(int index, float gain) {
    if (index >= 0 && index < 18) {
        m_eqBands[index].store(gain, std::memory_order_release);
        m_needsUpdate = true;
    }
}

void EqDSP::update_filters(float sample_rate) {
    static const float freqs[] = {
        20, 31, 50, 80, 125, 200, 315, 500, 800, 
        1250, 2000, 3150, 5000, 8000, 12500, 16000, 18000, 20000
    };
    for(int i=0; i<18; ++i) {
        float gain = m_eqBands[i].load(std::memory_order_acquire);
        m_filters[0][i].setPeaking(freqs[i], sample_rate, 1.414f, gain);
        m_filters[1][i].setPeaking(freqs[i], sample_rate, 1.414f, gain);
    }
    m_lastSampleRate = sample_rate;
    m_needsUpdate = false;
}

void EqDSP::process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate) {
    if (sample_rate != m_lastSampleRate || m_needsUpdate) {
        update_filters(sample_rate);
    }

    for (size_t i = 0; i < num_frames; ++i) {
        for (size_t ch = 0; ch < channels; ++ch) {
            size_t chIdx = (ch < 2) ? ch : 0;
            float sample = buffer[i * channels + ch];

            for(auto& f : m_filters[chIdx]) {
                sample = f.process(sample);
            }

            buffer[i * channels + ch] = sample;
        }
    }
}

} // namespace fb2k
