#ifndef FOOBAR2K_SDK_DSP_MANAGER_HPP
#define FOOBAR2K_SDK_DSP_MANAGER_HPP

#include "dsp.hpp"
#include <vector>
#include <memory>
#include <mutex>

namespace fb2k {

/**
 * Manages a chain of DSP effects.
 */
class dsp_manager : public service_base {
public:
    dsp_manager() = default;

    void add_dsp(std::shared_ptr<dsp> new_dsp);
    void clear();

    void process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate);

private:
    std::vector<std::shared_ptr<dsp>> m_chain;
    std::mutex m_mutex;
};

} // namespace fb2k

#endif // FOOBAR2K_SDK_DSP_MANAGER_HPP
