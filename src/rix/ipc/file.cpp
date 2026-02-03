#include "rix/ipc/file.hpp"

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rix {
namespace ipc {

/**< TODO */
bool File::remove(const std::string &pathname) {
    // removes the file specified by pathname
    return (::unlink(pathname.c_str()) == 0);
}

File::File() : fd_(-1) {}

File::File(int fd) : fd_(fd) {}

/**< TODO */
File::File(std::string pathname, int creation_flags, mode_t mode) {
    // opens the file specified by pathname with the given creation_flags and mode
    fd_ = ::open(pathname.c_str(), creation_flags, mode);
}

/**< TODO */
File::File(const File &src) {
    // duplicates the underlying file descriptor using dup
    fd_ = (src.fd_ >= 0) ? ::dup(src.fd_) : -1;
}

/**< TODO */
File &File::operator=(const File &src) {
    // duplicates the underlying file descriptor using dup
    if (this == &src) {
        return *this;
    }

    if (fd_ >= 0) {
        ::close(fd_);
    }

    fd_ = (src.fd_ >= 0) ? ::dup(src.fd_) : -1;
    return *this;
}

File::File(File &&src) : fd_(-1) {
    std::swap(fd_, src.fd_);
}

File &File::operator=(File &&src) {
    if (this == &src) {
        return *this;
    }

    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }

    std::swap(fd_, src.fd_);
    return *this;
}

/**< TODO */
File::~File() {
    // closes the underlying file descriptor
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

/**< TODO */
ssize_t File::read(uint8_t *buffer, size_t size) const {
    // reads size bytes from the file and store them in buffer
    if (fd_ < 0) {
        return -1;
    }
    return ::read(fd_, buffer, size);
}

/**< TODO */
ssize_t File::write(const uint8_t *buffer, size_t size) const {
    // writes size bytes from buffer to the file
    if (fd_ < 0) {
        return -1;
    }
    return ::write(fd_, buffer, size);
}

int File::fd() const { return fd_; }

/**< TODO */
bool File::ok() const {
    // returns true if the file is in a valid state
    return fd_ >= 0;
}

/**< TODO */
void File::set_nonblocking(bool status) {
    // sets the file descriptor to non-blocking mode if status is true, else sets it to blocking mode
    if (fd_ < 0) {
        return;
    }

    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        return;
    }

    if (status) {
        ::fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
    } else {
        ::fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK);
    }
}

/**< TODO */
bool File::is_nonblocking() const {
    if (fd_ < 0) {
        return false;
    }
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return (flags & O_NONBLOCK) != 0;
}

/**< TODO */
bool File::wait_for_writable(const util::Duration &duration) const {
    if (fd_ < 0) {
        return false;
    }

    struct pollfd pfd;
    pfd.fd = fd_;
    pfd.events = POLLOUT;
    pfd.revents = 0;

    int timeout_ms = (duration == util::Duration::max()) ? -1 : duration.to_milliseconds();
    int ret = ::poll(&pfd, 1, timeout_ms);

    return (ret > 0) && (pfd.revents & POLLOUT);
}

/**< TODO */
bool File::wait_for_readable(const util::Duration &duration) const {
    if (fd_ < 0) {
        return false;
    }

    struct pollfd pfd;
    pfd.fd = fd_;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int timeout_ms = (duration == util::Duration::max()) ? -1 : duration.to_milliseconds();
    int ret = ::poll(&pfd, 1, timeout_ms);

    return (ret > 0) && (pfd.revents & POLLIN);
}

}  // namespace ipc
}  // namespace rix
