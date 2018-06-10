#ifndef SYSD_JAG_DECOMPRESSOR_HPP
#define SYSD_JAG_DECOMPRESSOR_HPP

#pragma once

#include <vector>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace sysd::jag::detail {
using container_type = std::vector<char>;

auto decompress(const container_type &buffer, const std::size_t &offset,
                const std::size_t &decomp_len) {
    container_type compressed = {'B', 'Z', 'h', '1'};

    auto in_begin = std::cbegin(buffer);
    auto in_end = std::cend(buffer);

    // advance the input iterator by the buffer's read caret
    std::advance(in_begin, offset);

    // pre-allocate the copied buffer's size, it can be easily calculated
    compressed.reserve(std::distance(in_begin, in_end) + 4);

    std::copy(in_begin, in_end, std::back_inserter(compressed));

    // create a vector to which we will write decompressed data to
    // resize() must be used instead of reserve, so the vector
    // will know how many elements it contains
    container_type decompressed{};
    decompressed.resize(decomp_len);

    // create our boost in/out streams
    boost::iostreams::array_source src{compressed.data(), compressed.size()};
    boost::iostreams::array dest{decompressed.data(), decomp_len};

    boost::iostreams::filtering_istream in{};
    boost::iostreams::filtering_ostream out{};

    // specify we need to bunzip2 the input
    in.push(boost::iostreams::bzip2_decompressor{});
    in.push(src);

    out.push(dest);

    boost::iostreams::copy(in, out);

    return decompressed;
}
} // namespace sysd::jag::detail

#endif // SYSD_JAG_DECOMPRESSOR_HPP
