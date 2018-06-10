#ifndef SYSD_BUFFER_HPP
#define SYSD_BUFFER_HPP

#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <vector>

#include <spdlog/spdlog.h>

namespace sysd {
template <typename T, typename C = std::vector<std::decay_t<T>>>
struct basic_buffer {
    using type = std::decay_t<T>;
    using container_type = C;

    basic_buffer() = default;
    basic_buffer(const std::initializer_list<type> &data) : buf{data} {}
    basic_buffer(const container_type &data) : buf{data} {}

    basic_buffer(const basic_buffer &other) = default;
    basic_buffer(basic_buffer &&other) = default;
    basic_buffer &operator=(const basic_buffer &other) = default;
    basic_buffer &operator=(basic_buffer &&other) = default;

    // TODO impl constexpr if on N to automatically determine
    // the best result type
    template <std::size_t N, typename R> constexpr R read() {
        R result{0};

        for (std::size_t i = 0; i < N; i++) {
            result |= (buf[caret + (N - (i + 1))] & 0xff) << (i * 8);
        }

        caret += N;
        return result;
    }

    template <std::size_t N, typename R> constexpr void write(const R &value) {
        for (std::size_t i = N; i-- > 0;) {
            buf.emplace_back((value >> (i * 8)) & 0xff);
        }
    }

    constexpr void write(const basic_buffer &other) {
        std::copy(std::cbegin(other.data()), std::cend(other.data()),
                  std::back_inserter(buf));
    }

    const container_type &data() const { return buf; }
    const std::size_t &position() const { return caret; }

  private:
    container_type buf = {};
    std::size_t caret = 0;
};

using buffer = basic_buffer<char>;
} // namespace sysd

#endif // SYSD_BUFFER_HPP
