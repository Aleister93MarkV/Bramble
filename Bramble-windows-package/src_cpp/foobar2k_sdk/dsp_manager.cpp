#include "dsp_manager.hpp"

namespace fb2k {

void dsp_manager::add_dsp(std::shared_ptr<dsp> new_dsp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chain.push_back(new_dsp);
}

void dsp_manager::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chain.clear();
}

void dsp_manager::process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& dsp_node : m_chain) {
        if (dsp_node) {
            dsp_node->process_chunk(buffer, num_frames, channels, sample_rate);
        }
    }
}

} // namespace fb2k
