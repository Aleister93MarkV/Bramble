#ifndef FOOBAR2K_SDK_SERVICE_BASE_HPP
#define FOOBAR2K_SDK_SERVICE_BASE_HPP

#include <atomic>
#include <memory>

namespace fb2k {

/**
 * Base class for all foobar2000 SDK components.
 * In the real SDK, this provides reference counting (service_add_ref, service_release).
 * Here we use standard C++ idioms (std::shared_ptr) but keep the structural hierarchy.
 */
class service_base {
public:
    virtual ~service_base() = default;

protected:
    service_base() = default;
};

} // namespace fb2k

#endif // FOOBAR2K_SDK_SERVICE_BASE_HPP
