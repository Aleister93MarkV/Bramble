#ifndef FOOBAR2K_SDK_CRYSTALLIZER_DSP_HPP
#define FOOBAR2K_SDK_CRYSTALLIZER_DSP_HPP

#include "dsp.hpp"
#include <array>
#include <atomic>

namespace fb2k {

class CrystallizerDSP : public dsp {
public:
    CrystallizerDSP();

    void process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate) override;
    
    void set_amount(float amount);

private:
    std::atomic<float> m_amount;
    std::array<float, 2> m_lp_state = {0.0f, 0.0f};
};

} // namespace fb2k

#endif // FOOBAR2K_SDK_CRYSTALLIZER_DSP_HPP
