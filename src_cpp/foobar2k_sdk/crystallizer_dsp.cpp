#include "crystallizer_dsp.hpp"

namespace fb2k {

// Fast rational approximation of tanh for audio distortion
inline float fast_tanh(float x) {
    if (x < -3.0f) return -1.0f;
    if (x > 3.0f) return 1.0f;
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

CrystallizerDSP::CrystallizerDSP() {
    m_amount.store(0.0f);
}

void CrystallizerDSP::set_amount(float amount) {
    m_amount.store(amount, std::memory_order_release);
}

void CrystallizerDSP::process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate) {
    (void)sample_rate;
    float crystNorm = m_amount.load(std::memory_order_acquire) / 20.0f;
    
    if (crystNorm <= 0.0f) return;

    for (size_t i = 0; i < num_frames; ++i) {
        for (size_t ch = 0; ch < channels; ++ch) {
            size_t chIdx = (ch < 2) ? ch : 0;
            float sample = buffer[i * channels + ch];

            float highPass = sample - m_lp_state[chIdx];
            m_lp_state[chIdx] += 0.1f * highPass; 

            float excited = fast_tanh(highPass * 3.0f);
            sample += excited * crystNorm;

            buffer[i * channels + ch] = sample;
        }
    }
}

} // namespace fb2k
