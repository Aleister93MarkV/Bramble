#ifndef FOOBAR2K_SDK_EQ_DSP_HPP
#define FOOBAR2K_SDK_EQ_DSP_HPP

#include "dsp.hpp"
#include <array>
#include <atomic>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace fb2k {

// Biquad Filter structure for High-Quality EQ
struct Biquad {
    float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;
    
    void setPeaking(float freq, float sampleRate, float Q, float dbGain) {
        float A = std::pow(10.0f, dbGain / 40.0f);
        float omega = 2.0f * M_PI * freq / sampleRate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);
        float alpha = sn / (2.0f * Q);
        
        b0 = 1.0f + alpha * A;
        b1 = -2.0f * cs;
        b2 = 1.0f - alpha * A;
        float a0 = 1.0f + alpha / A;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha / A;
        
        // Normalize
        b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
    }
    
    inline float process(float x) {
        float out = b0 * x + b1 * z1 + b2 * z2 - a1 * z1 - a2 * z2;
        z2 = z1;
        z1 = out;
        return out;
    }
};

class EqDSP : public dsp {
public:
    EqDSP();
    
    void process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate) override;
    
    void set_band(int index, float gain);

private:
    void update_filters(float sample_rate);

    std::array<std::atomic<float>, 18> m_eqBands;
    std::array<std::array<Biquad, 18>, 2> m_filters; // 2 channels max for now
    float m_lastSampleRate = 0.0f;
    bool m_needsUpdate = true;
};

} // namespace fb2k

#endif // FOOBAR2K_SDK_EQ_DSP_HPP
