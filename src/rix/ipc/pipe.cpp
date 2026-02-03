#include "rix/ipc/pipe.hpp"

#include <unistd.h>

namespace rix {
namespace ipc {

/**< TODO */
std::array<Pipe, 2> Pipe::create() {
    int fds[2];
    if (::pipe(fds) < 0) {
        return {};
    }

    // fds[0] = read end, fds[1] = write end
    return {Pipe(fds[0], true), Pipe(fds[1], false)};
}

Pipe::Pipe() : File(), read_end_(false) {}

/**< TODO */
Pipe::Pipe(const Pipe &other) {
    if (other.fd_ >= 0) {
        fd_ = ::dup(other.fd_);
        read_end_ = other.read_end_;
    } else {
        fd_ = -1;
        read_end_ = false;
    }
}

/**< TODO */
Pipe &Pipe::operator=(const Pipe &other) {
    if (this == &other) {
        return *this;
    }

    if (fd_ >= 0) {
        ::close(fd_);
    }

    if (other.fd_ >= 0) {
        fd_ = ::dup(other.fd_);
        read_end_ = other.read_end_;
    } else {
        fd_ = -1;
        read_end_ = false;
    }

    return *this;
}

Pipe::Pipe(Pipe &&other)
    : File(std::move(other)),
      read_end_(other.read_end_) {
    other.read_end_ = false;
}

Pipe &Pipe::operator=(Pipe &&other) {
    if (this == &other) {
        return *this;
    }

    if (fd_ >= 0) {
        ::close(fd_);
    }

    fd_ = other.fd_;
    read_end_ = other.read_end_;

    other.fd_ = -1;
    other.read_end_ = false;

    return *this;
}

Pipe::~Pipe() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool Pipe::is_read_end() const {
    return read_end_;
}

bool Pipe::is_write_end() const {
    return !read_end_;
}

Pipe::Pipe(int fd, bool read_end)
    : File(fd), read_end_(read_end) {}

}  // namespace ipc
}  // namespace rix
