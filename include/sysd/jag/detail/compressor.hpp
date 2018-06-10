#ifndef SYSD_JAG_COMPRESSOR_HPP
#define SYSD_JAG_COMPRESSOR_HPP

#pragma once

#include <iterator>
#include <vector>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace sysd::jag::detail {
using container_type = std::vector<char>;

auto compress(const container_type &buffer) {
    container_type compressed{};

    boost::iostreams::filtering_ostream out{};

    // must use a block size of 1
    out.push(boost::iostreams::bzip2_compressor(1));
    out.push(std::back_inserter(compressed));

    boost::iostreams::copy(boost::make_iterator_range(buffer), out);

    // remove the BZh1 header
    auto begin = std::begin(compressed);
    auto end = std::begin(compressed);

    std::advance(end, 4);
    compressed.erase(begin, end);

    return compressed;
}
} // namespace sysd::jag::detail

#endif // SYSD_JAG_COMPRESSOR_HPP
