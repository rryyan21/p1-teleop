#include "rix/ipc/fifo.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

namespace rix {
namespace ipc {

Fifo::Fifo(const std::string &pathname, Mode mode, bool nonblocking)
    : pathname_(pathname), mode_(mode) {

    // Create FIFO if it does not exist
    if (mkfifo(pathname.c_str(), 0666) < 0) {
        if (errno != EEXIST) {
            fd_ = -1;
            return;
        }
    }

    int flags = 0;
    if (mode == Mode::READ) {
        flags |= O_RDONLY;
    } else {
        flags |= O_WRONLY;
    }

    if (nonblocking) {
        flags |= O_NONBLOCK;
    }

    fd_ = ::open(pathname.c_str(), flags);
}

Fifo::Fifo() {
    fd_ = -1;
    pathname_.clear();
    mode_ = Mode::READ;
}

Fifo::Fifo(const Fifo &other) {
    fd_ = (other.fd_ >= 0) ? ::dup(other.fd_) : -1;
    pathname_ = other.pathname_;
    mode_ = other.mode_;
}

Fifo &Fifo::operator=(const Fifo &other) {
    if (this == &other) {
        return *this;
    }

    if (fd_ >= 0) {
        ::close(fd_);
    }

    fd_ = (other.fd_ >= 0) ? ::dup(other.fd_) : -1;
    pathname_ = other.pathname_;
    mode_ = other.mode_;

    return *this;
}

Fifo::Fifo(Fifo &&other)
    : File(std::move(other)),
      pathname_(std::move(other.pathname_)),
      mode_(other.mode_) {
}

Fifo &Fifo::operator=(Fifo &&other) {
    if (this == &other) {
        return *this;
    }

    if (fd_ >= 0) {
        ::close(fd_);
    }

    fd_ = other.fd_;
    pathname_ = std::move(other.pathname_);
    mode_ = other.mode_;

    other.fd_ = -1;
    other.pathname_.clear();

    return *this;
}

Fifo::~Fifo() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

std::string Fifo::pathname() const {
    return pathname_;
}

Fifo::Mode Fifo::mode() const {
    return mode_;
}

}  // namespace ipc
}  // namespace rix
