#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include <spdlog/spdlog.h>
#include <sysd/buffer.hpp>
#include <sysd/jag/archive.hpp>

#include <boost/program_options.hpp>

sysd::buffer read_file(const std::string name) {
    std::ifstream file(name, std::ios::binary);
    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);

    auto file_size = file.tellg();

    file.seekg(0, std::ios::beg);

    sysd::buffer::container_type data{};
    data.reserve(file_size);

    data.insert(data.begin(), std::istream_iterator<sysd::buffer::type>(file),
                std::istream_iterator<sysd::buffer::type>());

    return sysd::buffer{data};
}

void write_file(const std::string name, const sysd::buffer &buffer) {
    std::ofstream out{name};
    std::ostream_iterator<char> out_itr{out};

    std::copy(std::cbegin(buffer.data()), std::cend(buffer.data()), out_itr);
}

// TODO: clean up the program options portion.. it's REALLY messy
int main(int argc, char **argv) {
    namespace opts = boost::program_options;

    opts::options_description desc{"options"};
    desc.add_options()("help,h", "prints this help message")(
        "verbose,v", "enables debug information")(
        "extract,e", opts::value<std::vector<std::string>>(),
        "extracts a given entry")("out,o", opts::value<std::string>(),
                                  "specify output location")(
        "input", opts::value<std::string>(), "input archive to operate on");

    opts::positional_options_description pos_opts;
    pos_opts.add("input", -1);

    opts::variables_map var_map{};
    opts::store(opts::command_line_parser{argc, argv}
                    .options(desc)
                    .positional(pos_opts)
                    .run(),
                var_map);
    opts::notify(var_map);

    if (var_map.count("help")) {
        std::stringstream ss{}; // really?
        ss << desc;
        fmt::print("{}\n", ss.str());
        return 0;
    }

    if (var_map.count("verbose")) {
        spdlog::set_level(spdlog::level::debug);
    }

    if (!var_map.count("input")) {
        fmt::print("error: no input file\n\n");
        std::stringstream ss{};
        ss << desc;
        fmt::print("{}\n", ss.str());
        return 1;
    }

    if (!var_map.count("extract")) { // TODO add future operations here
        fmt::print("no archive operation specified, at least one required");
        return 1;
    }

    auto log = spdlog::stdout_color_mt("console");

    try {
        const auto archive_name = var_map["input"].as<std::string>();

        auto buffer = read_file(archive_name);
        sysd::jag::archive archive{buffer};

        if (var_map.count("extract")) {
            const auto files =
                var_map["extract"].as<std::vector<std::string>>();

            for (const auto &file : files) {
                if (auto data = archive.get(file); data) {
                    write_file(fmt::format("{}", file), data.value());
                } else {
                    log->warn("couldn't find {} in archive", file);
                }
            }
        }
    } catch (std::exception &e) {
        log->critical("uncaught exception: {}", e.what());
    }
    return 0;
}
