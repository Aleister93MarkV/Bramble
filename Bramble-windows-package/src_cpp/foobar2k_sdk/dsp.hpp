#ifndef FOOBAR2K_SDK_DSP_HPP
#define FOOBAR2K_SDK_DSP_HPP

#include "service_base.hpp"
#include <cstddef>

namespace fb2k {

/**
 * Interface for DSP effects.
 */
class dsp : public service_base {
public:
    /**
     * Process a chunk of audio data.
     * @param buffer Pointer to the audio buffer (interleaved floats).
     * @param num_frames Number of frames in the buffer.
     * @param channels Number of channels (e.g. 2 for stereo).
     * @param sample_rate Sample rate (e.g. 44100).
     */
    virtual void process_chunk(float* buffer, size_t num_frames, size_t channels, float sample_rate) = 0;
};

} // namespace fb2k

#endif // FOOBAR2K_SDK_DSP_HPP
