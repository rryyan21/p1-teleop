#include "rix/ipc/signal.hpp"

#include <stdexcept>

namespace rix {
namespace ipc {

std::array<Signal::SignalNotifier, 32> Signal::notifier = {};

/**< TODO */
Signal::Signal(int signum) : signum_(signum - 1) {
    if (signum < 1 || signum > 32) {
        throw std::invalid_argument("Signal number must be between 1 and 32");
    }

    if (notifier[signum_].is_init) {
        throw std::invalid_argument("Signal already registered");
    }

    notifier[signum_].pipe = Pipe::create();
    notifier[signum_].is_init = true;

    // IMPORTANT: read end must be non-blocking
    notifier[signum_].pipe[0].set_nonblocking(true);

    ::signal(signum, Signal::handler);
}

/**< TODO */
Signal::~Signal() {
    if (signum_ >= 0 && signum_ < 32 && notifier[signum_].is_init) {
        notifier[signum_].is_init = false;
        notifier[signum_].pipe = {};
        ::signal(signum_ + 1, SIG_DFL);
    }
}

Signal::Signal(Signal &&other) : signum_(-1) {
    std::swap(signum_, other.signum_);
}

Signal &Signal::operator=(Signal &&other) {
    if (this == &other) {
        return *this;
    }

    if (signum_ >= 0 && signum_ < 32 && notifier[signum_].is_init) {
        notifier[signum_].is_init = false;
        notifier[signum_].pipe = {};
        ::signal(signum_ + 1, SIG_DFL);
    }

    signum_ = -1;
    std::swap(signum_, other.signum_);
    return *this;
}

/**< TODO */
bool Signal::raise() const {
    if (signum_ < 0 || signum_ >= 32) {
        return false;
    }
    return (::raise(signum_ + 1) == 0);
}

/**< TODO */
bool Signal::kill(pid_t pid) const {
    if (signum_ < 0 || signum_ >= 32) {
        return false;
    }
    return (::kill(pid, signum_ + 1) == 0);
}

int Signal::signum() const {
    return (signum_ >= 0 && signum_ < 32) ? signum_ + 1 : -1;
}

/**< TODO */
bool Signal::wait(const rix::util::Duration &d) const {
    if (signum_ < 0 || signum_ >= 32) {
        return false;
    }

    if (!notifier[signum_].is_init) {
        return false;
    }

    // First, wait until readable
    if (!notifier[signum_].pipe[0].wait_for_readable(d)) {
        return false;
    }

    // Now actually read a byte to confirm signal delivery
    uint8_t byte;
    ssize_t n = notifier[signum_].pipe[0].read(&byte, 1);

    return (n > 0);
}

/**< TODO */
void Signal::handler(int signum) {
    int index = signum - 1;
    if (index < 0 || index >= 32) {
        return;
    }

    if (!notifier[index].is_init) {
        return;
    }

    uint8_t byte = 1;
    notifier[index].pipe[1].write(&byte, 1);
}

}  // namespace ipc
}  // namespace rix
