#ifndef SYSD_JAG_SERIALIZE_HPP
#define SYSD_JAG_SERIALIZE_HPP

#pragma once

#include <tuple>
#include <utility>

#include <sysd/buffer.hpp>
#include <sysd/jag/archive.hpp>
#include <sysd/jag/detail/compressor.hpp>

namespace sysd::jag {
namespace {
template <typename T> auto compute_data_block(const T &entries) {
    sysd::buffer buffer{};

    for (const auto &entry : entries) {
        const auto &buf = std::get<sysd::buffer>(entry);

        buffer.write(buf);
    }

    return buffer;
}
template <typename T> auto compute_info_block(const T &entries) {
    sysd::buffer buffer{};

    buffer.write<2, std::size_t>(entries.size());

    for (const auto &entry : entries) {
        const auto &[name, buf] = entry;

        buffer.write<4, std::size_t>(name);
        buffer.write<3, std::size_t>(buf.data().size());
        buffer.write<3, std::size_t>(buf.data().size());
    }

    return buffer;
}
} // namespace

const sysd::buffer serialize(const archive &arc, std::size_t threshold) {
    const auto &entries = arc.get_entries();
    const auto info_block = compute_info_block(entries);
    const auto data_block = compute_data_block(entries);

    sysd::buffer body{};

    body.write(std::move(info_block));
    body.write(std::move(data_block));

    // The jag format consists of ia 6 byte header, the first
    // 3 bytes are the archive's decompressed size, and the
    // other 3 bytes are the archive's compressed size. If the two
    // sizes don't equal eachother, the engine will attempt to decompress
    // the remaining data.
    auto decompressed_size = body.data().size();
    auto compressed_size = decompressed_size;

    if (decompressed_size >= threshold) {
        body = sysd::buffer{detail::compress(body.data())};
        compressed_size = body.data().size();
    }

    sysd::buffer buffer{};

    buffer.write<3, std::size_t>(decompressed_size);
    buffer.write<3, std::size_t>(compressed_size);
    buffer.write(body);

    return buffer;
}
} // namespace sysd::jag

#endif // SYSD_JAG_SERIALIZE_HPP
