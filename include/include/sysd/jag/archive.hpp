#ifndef SYSD_JAG_ARCHIVE_HPP
#define SYSD_JAG_ARCHIVE_HPP

#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <tuple>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <sysd/buffer.hpp>
#include <sysd/jag/detail/decompressor.hpp>
#include <sysd/jag/detail/entry_encode.hpp>

#include <spdlog/spdlog.h>

namespace sysd::jag {
struct archive {
    using entry_type = std::tuple<std::uint32_t, sysd::buffer>;

    archive(sysd::buffer &b) : entries{} { read_headers(b); }

    boost::optional<const sysd::buffer &> get(const boost::string_view file) {
        const auto encoded = detail::encode_entry_name(file);

        for (const auto &entry : entries) {
            if (std::get<std::uint32_t>(entry) == encoded) {
                return std::get<sysd::buffer>(entry);
            }
        }

        return boost::none;
    }

    void put(const boost::string_view file, sysd::buffer buffer) {
        auto log = spdlog::get("jag");

        const auto encoded = detail::encode_entry_name(file);

        for (auto &entry : entries) {
            if (std::get<std::uint32_t>(entry) == encoded) {
                log->warn("replaced {} in archive", file.to_string());
                std::get<sysd::buffer>(entry) = std::move(buffer);
                return;
            }
        }

        log->debug("added new archive entry {}", file.to_string());
        entries.emplace_back(encoded, std::move(buffer));
    }

  private:
    std::vector<entry_type> entries;

    void read_headers(sysd::buffer &buffer) {
        auto log = spdlog::get("jag");

        const auto decomp_len = buffer.read<3, std::size_t>();
        const auto comp_len = buffer.read<3, std::size_t>();

        sysd::buffer &decompressed = buffer;

        log->debug("decompressed len={}, compressed len={}", decomp_len,
                   comp_len);

        if (decomp_len != comp_len) {
            log->debug("decompressing archive");

            decompressed = sysd::buffer{detail::decompress(
                buffer.data(), buffer.position(), decomp_len)};
        }

        unpack_files(decompressed);
    }
    void unpack_files(sysd::buffer &buffer) {
        auto log = spdlog::get("jag");

        // The decompressed file format contains a header of 2 bytes for number
        // of files within the archive. Then follows the entry table, for each
        // file in the archive, 10 bytes are written in this table. 4 bytes for
        // the encoded entry name, 3 bytes for the entry's decompressed size,
        // and 3 bytes for the entry's compressed size. Finally, after the
        // entry table, is the data table which consists of each entry's data.
        const auto file_count = buffer.read<2, std::size_t>();
        std::size_t ptr_offset{buffer.position() + (file_count * 10)};

        log->debug("found {} files in archive", file_count);

        entries.reserve(file_count);

        for (std::size_t i = 0; i < file_count; i++) {
            auto name = buffer.read<4, std::uint32_t>();
            auto decomp_len = buffer.read<3, std::size_t>();
            auto comp_len = buffer.read<3, std::size_t>();

            sysd::buffer::container_type entry_data{};
            entry_data.reserve(comp_len);

            auto buff_begin = std::begin(buffer.data());

            std::advance(buff_begin, ptr_offset);
            std::copy_n(buff_begin, comp_len, std::back_inserter(entry_data));

            log->debug("\tname={}, offset={}, size={}", name, ptr_offset,
                       entry_data.size());

            if (decomp_len != comp_len) {
                log->debug("\t\tdecompressing entry");

                auto decompressed =
                    detail::decompress(entry_data, 0, decomp_len);

                entries.emplace_back(name,
                                     sysd::buffer{std::move(decompressed)});
            } else {
                entries.emplace_back(name, sysd::buffer{std::move(entry_data)});
            }

            ptr_offset += comp_len;
        }
    }
};
} // namespace sysd::jag

#endif // SYSD_JAG_ARCHIVE_HPP
