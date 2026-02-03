#pragma once

#include <array>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include "rix/msg/message.hpp"

namespace rix {
namespace msg {
namespace detail {

template<typename T>
inline uint32_t size_number(const T &) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    return sizeof(T);
}

inline uint32_t size_string(const std::string &src) {
    return 4 + src.size();
}

inline uint32_t size_message(const Message &src) {
    return src.size();
}

template<typename T, size_t N>
inline uint32_t size_number_array(const std::array<T, N> &) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    return N * sizeof(T);
}

template<size_t N>
inline uint32_t size_string_array(const std::array<std::string, N> &src) {
    uint32_t size = 0;
    for (const auto &s : src)
        size += size_string(s);
    return size;
}

template<typename T, size_t N>
inline uint32_t size_message_array(const std::array<T, N> &src) {
    static_assert(std::is_base_of<Message, T>::value, "T must derive from Message");
    uint32_t size = 0;
    for (const auto &m : src)
        size += size_message(m);
    return size;
}

template<typename T>
inline uint32_t size_number_vector(const std::vector<T> &src) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    return 4 + src.size() * sizeof(T);
}

inline uint32_t size_string_vector(const std::vector<std::string> &src) {
    uint32_t size = 4;
    for (const auto &s : src)
        size += size_string(s);
    return size;
}

template<typename T>
inline uint32_t size_message_vector(const std::vector<T> &src) {
    static_assert(std::is_base_of<Message, T>::value, "T must derive from Message");
    uint32_t size = 4;
    for (const auto &m : src)
        size += size_message(m);
    return size;
}

template<typename T>
inline void serialize_number(uint8_t *dst, size_t &offset, const T &src) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    std::memcpy(dst + offset, &src, sizeof(T));
    offset += sizeof(T);
}

inline void serialize_string(uint8_t *dst, size_t &offset, const std::string &src) {
    uint32_t len = src.size();
    serialize_number(dst, offset, len);
    std::memcpy(dst + offset, src.data(), len);
    offset += len;
}

inline void serialize_message(uint8_t *dst, size_t &offset, const Message &src) {
    src.serialize(dst, offset);
    // offset is already updated by src.serialize()
}

template<typename T, size_t N>
inline void serialize_number_array(uint8_t *dst, size_t &offset, const std::array<T, N> &src) {
    for (const auto &v : src)
        serialize_number(dst, offset, v);
}

template<size_t N>
inline void serialize_string_array(uint8_t *dst, size_t &offset, const std::array<std::string, N> &src) {
    for (const auto &s : src)
        serialize_string(dst, offset, s);
}

template<typename T, size_t N>
inline void serialize_message_array(uint8_t *dst, size_t &offset, const std::array<T, N> &src) {
    static_assert(std::is_base_of<Message, T>::value, "T must derive from Message");
    for (const auto &m : src)
        serialize_message(dst, offset, m);
}

template<typename T>
inline void serialize_number_vector(uint8_t *dst, size_t &offset, const std::vector<T> &src) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    uint32_t len = src.size();
    serialize_number(dst, offset, len);
    for (const auto &v : src)
        serialize_number(dst, offset, v);
}

inline void serialize_string_vector(uint8_t *dst, size_t &offset, const std::vector<std::string> &src) {
    uint32_t len = src.size();
    serialize_number(dst, offset, len);
    for (const auto &s : src)
        serialize_string(dst, offset, s);
}

template<typename T>
inline void serialize_message_vector(uint8_t *dst, size_t &offset, const std::vector<T> &src) {
    static_assert(std::is_base_of<Message, T>::value, "T must derive from Message");
    uint32_t len = src.size();
    serialize_number(dst, offset, len);
    for (const auto &m : src)
        serialize_message(dst, offset, m);
}

template<typename T>
inline bool deserialize_number(T &dst, const uint8_t *src, size_t size, size_t &offset) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    if (offset + sizeof(T) > size)
        return false;
    std::memcpy(&dst, src + offset, sizeof(T));
    offset += sizeof(T);
    return true;
}

inline bool deserialize_string(std::string &dst, const uint8_t *src, size_t size, size_t &offset) {
    uint32_t len;
    if (!deserialize_number(len, src, size, offset))
        return false;
    if (offset + len > size)
        return false;
    dst.assign(reinterpret_cast<const char*>(src + offset), len);
    offset += len;
    return true;
}

inline bool deserialize_message(Message &dst, const uint8_t *src, size_t size, size_t &offset) {
    size_t consumed = dst.deserialize(src, size, offset);
    if (consumed == 0)
        return false;
    // offset is already updated by dst.deserialize()
    return true;
}

template<typename T, size_t N>
inline bool deserialize_number_array(std::array<T, N> &dst, const uint8_t *src, size_t size, size_t &offset) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    for (auto &v : dst)
        if (!deserialize_number(v, src, size, offset))
            return false;
    return true;
}

template<size_t N>
inline bool deserialize_string_array(std::array<std::string, N> &dst, const uint8_t *src, size_t size, size_t &offset) {
    for (auto &s : dst)
        if (!deserialize_string(s, src, size, offset))
            return false;
    return true;
}

template<typename T, size_t N>
inline bool deserialize_message_array(std::array<T, N> &dst, const uint8_t *src, size_t size, size_t &offset) {
    static_assert(std::is_base_of<Message, T>::value, "T must derive from Message");
    for (auto &m : dst) {
        if (!deserialize_message(m, src, size, offset))
            return false;
    }
    return true;
}

template<typename T>
inline bool deserialize_number_vector(std::vector<T> &dst, const uint8_t *src, size_t size, size_t &offset) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
    uint32_t len;
    if (!deserialize_number(len, src, size, offset))
        return false;
    
    dst.clear();
    dst.reserve(len);
    for (uint32_t i = 0; i < len; ++i) {
        T v;
        if (!deserialize_number(v, src, size, offset))
            return false;
        dst.push_back(v);
    }
    return true;
}

inline bool deserialize_string_vector(std::vector<std::string> &dst, const uint8_t *src, size_t size, size_t &offset) {
    uint32_t len;
    if (!deserialize_number(len, src, size, offset))
        return false;
    
    dst.clear();
    dst.reserve(len);
    for (uint32_t i = 0; i < len; ++i) {
        std::string s;
        if (!deserialize_string(s, src, size, offset))
            return false;
        dst.push_back(s);
    }
    return true;
}

template<typename T>
inline bool deserialize_message_vector(std::vector<T> &dst, const uint8_t *src, size_t size, size_t &offset) {
    static_assert(std::is_base_of<Message, T>::value, "T must derive from Message");
    uint32_t len;
    if (!deserialize_number(len, src, size, offset))
        return false;
    
    dst.clear();
    dst.reserve(len);
    for (uint32_t i = 0; i < len; ++i) {
        T msg;
        if (!deserialize_message(msg, src, size, offset))
            return false;
        dst.push_back(msg);
    }
    return true;
}

}  // namespace detail
}  // namespace msg
}  // namespace rix