#ifndef SYSD_JAG_ENTRY_ENCODE_HPP
#define SYSD_JAG_ENTRY_ENCODE_HPP

#pragma once

#include <cstdint>
#include <locale>

#include <boost/utility/string_view.hpp>

namespace sysd::jag::detail {
std::uint32_t encode_entry_name(const boost::string_view entry) {
    std::uint32_t encoded{0};

    for (const auto &ch : entry) {
        encoded = (encoded * 61) + (std::toupper(ch, std::locale{}) - 32);
    }

    return encoded;
}
} // namespace sysd::jag::detail

#endif // SYSD_JAG_ENTRY_ENCODE_HPP
