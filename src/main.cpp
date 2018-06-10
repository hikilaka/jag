#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include <spdlog/spdlog.h>
#include <sysd/buffer.hpp>
#include <sysd/jag/archive.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>

boost::optional<sysd::buffer> read_file(const std::string name) {
    std::ifstream file(name, std::ios::binary);

    if (!file.is_open() && file.good()) {
        return boost::none;
    }

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

void write_file(const boost::filesystem::path &location,
                const sysd::buffer &buffer) {
    std::ofstream out{location.string()};
    std::ostream_iterator<char> out_itr{out};

    std::copy(std::cbegin(buffer.data()), std::cend(buffer.data()), out_itr);
}

void write_archive(const boost::filesystem::path &file,
                   const sysd::jag::archive &archive) {}

namespace opts = boost::program_options;

opts::options_description generic_opts{"generic options"};
opts::options_description op_opts{"archive opterations"};
opts::options_description visible_opts{"allowed options"};
opts::options_description all_opts{"all options"};
opts::positional_options_description hidden_opts{};

void init_program_options() {
    using str_val = std::string;
    using vec_val = std::vector<std::string>;

    generic_opts.add_options()("help,h", "prints this help message")(
        "verbose,v", "enables debug information");

    op_opts.add_options()(
        "extract,e",
        opts::value<vec_val>()->composing()->default_value({}, "{}"),
        "extracts an archive's entry")(
        "insert,i",
        opts::value<vec_val>()->composing()->default_value({}, "{}"),
        "inserts a file into an archive")(
        "output,o", opts::value<str_val>(),
        "specifies the output directory/file");

    hidden_opts.add("input", -1);

    visible_opts.add(generic_opts).add(op_opts);

    all_opts.add(visible_opts);
    all_opts.add_options()("input", opts::value<str_val>());
}

void display_help() {
    // TODO implement a libfmt printer for
    // options_description in the future
    std::stringstream ss{};
    ss << visible_opts;

    fmt::print("{}\n", ss.str());
}

opts::variables_map parse_arguments(int argc, char **argv) {
    opts::variables_map map{};

    opts::store(opts::command_line_parser{argc, argv}
                    .options(all_opts)
                    .positional(hidden_opts)
                    .run(),
                map);
    opts::notify(map);

    return map;
}

int main(int argc, char **argv) {
    init_program_options();
    const auto args = parse_arguments(argc, argv);

    if (args.count("help")) {
        display_help();
        return 0;
    }

    if (args.count("verbose")) {
        spdlog::set_level(spdlog::level::debug);
    }

    auto log = spdlog::stdout_color_mt("jag");

    if (!args.count("input")) {
        log->warn("no input files\n");
        display_help();
        return 1;
    }

    auto out_path = boost::filesystem::current_path();

    if (args.count("output")) {
        out_path = boost::filesystem::path{args["output"].as<std::string>()};

        if (!boost::filesystem::is_directory(out_path)) {
            log->critical("supplied output directory isn't a directory");
            return 1;
        }
    }

    try {
        const auto archive_name = args["input"].as<std::string>();
        const auto req_extract = args["extract"].as<std::vector<std::string>>();
        const auto req_insert = args["insert"].as<std::vector<std::string>>();

        if (req_extract.size() == 0 && req_insert.size() == 0) {
            log->warn("no archive operation specified");
            display_help();
            return 1;
        }

        if (auto buffer = read_file(archive_name); buffer) {
            sysd::jag::archive archive{buffer.value()};

            for (const auto &file : req_extract) {
                if (auto data = archive.get(file); data) {
                    write_file(out_path / file, data->data());
                } else {
                    log->warn("couldn't find {} in {}", file, archive_name);
                }
            }

            for (const auto &file : req_insert) {
                if (auto data = read_file(file); data) {
                    archive.put(file, data.value());
                } else {
                    log->warn("couldn't find {} while inserting", file);
                }
            }

            if (req_insert.size() > 0) {
                // write archive
            }
        } else {
            log->critical("unable to open {}", archive_name);
            return 1;
        }
    } catch (std::exception &e) {
        auto trace = boost::stacktrace::stacktrace();
        log->critical("uncaught exception: {}", e.what());
        std::stringstream ss{};
        ss << trace;
        fmt::print("{}", ss.str());
    }
    return 0;
}
