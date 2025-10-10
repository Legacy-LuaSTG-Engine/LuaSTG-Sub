// ReSharper disable CppUseStructuredBinding

#include <print>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <expected>
#include <format>
#include "luastg/mask.hpp"

using std::string_literals::operator ""s;
using std::string_view_literals::operator ""sv;

namespace {
    std::u8string_view getUtf8StringView(std::string_view const& s) {
        return {reinterpret_cast<char8_t const*>(s.data()), s.size()};
    }

    std::string_view getStringView(std::u8string_view const& s) {
        return {reinterpret_cast<char const*>(s.data()), s.size()};
    }

    std::vector<std::string> getCommandLineArguments();

    struct Options {
        std::string output;
        std::vector<std::string> directories;

        void print() const {
            std::println("directories:"sv);
            for (auto const& directory: directories) {
                std::println("    {}"sv, directory);
            }
            std::println("output:"sv);
            std::println("    {}"sv, output);
        }
    };

    void parseCommandLineArguments(Options& options) {
        auto const args = getCommandLineArguments();
        bool is_output{false};
        for (auto const& arg: args) {
            if (arg == "-o"sv || arg == "--output"sv) {
                is_output = true;
                continue;
            }
            if (is_output) {
                options.output = arg;
                is_output = false;
                continue;
            }
            options.directories.emplace_back(arg);
        }
    }

    std::expected<bool, std::string> verifyOptions(Options const& options) {
        if (options.output.empty()) {
            return std::unexpected("no output path provided"s);
        }
        if (options.directories.empty()) {
            return std::unexpected("no directory provided"s);
        }
        std::error_code ec;
        for (auto const& directory: options.directories) {
            std::filesystem::path const path(getUtf8StringView(directory));
            if (!std::filesystem::exists(path, ec)) {
                return std::unexpected(std::format("directory '{}' not found"sv, directory));
            }
            if (!std::filesystem::is_directory(path, ec)) {
                return std::unexpected(std::format("'{}' is not a directory"sv, directory));
            }
        }
        return true;
    }

    struct InternalLuaScriptsFileSystemEntry {
        std::string name;
        std::filesystem::path path;
        std::string variable_name;
        size_t size{};
        bool is_directory{};
    };

    bool readAllBytes(std::filesystem::path const& path, std::vector<uint8_t>& buffer) {
        auto const file_path_u8 = path.lexically_normal().generic_u8string();
        auto const file_path = getStringView(file_path_u8);
        std::ifstream file(path, std::ofstream::in | std::ofstream::binary);
        if (!file.is_open()) {
            std::println("error: open file '{}' to read failed"sv, file_path);
            return false;
        }
        if (!file.seekg(0, std::ifstream::end)) {
            std::println("error: file '{}' seek to end failed"sv, file_path);
            return false;
        }
        auto const end = file.tellg();
        if (end == static_cast<std::streampos>(-1)) {
            std::println("error: file '{}' tell failed (1)"sv, file_path);
            return false;
        }
        if (!file.seekg(0, std::ifstream::beg)) {
            std::println("error: file '{}' seek to begin failed"sv, file_path);
            return false;
        }
        auto const beg = file.tellg();
        if (beg == static_cast<std::streampos>(-1)) {
            std::println("error: file '{}' tell failed (2)"sv, file_path);
            return false;
        }
        auto const size = end - beg;
        if (size < 0) {
            std::println("error: file '{}' size invalid (size = {})"sv, file_path, size);
            return false;
        }
        if (size == 0) {
            buffer.clear();
            return true;
        }
        buffer.resize(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            std::println("error: file '{}' read failed"sv, file_path);
            return false;
        }
        if (auto const count = file.gcount(); count != size) {
            std::println("error: file '{}' read failed (read {} bytes, but file size {} bytes)"sv,
                         file_path, count, size);
            return false;
        }
        return true;
    }

    void normalizeNewLine(std::vector<uint8_t>& buffer) {
        size_t i{};
        size_t j{};
        while (i < buffer.size()) {
            if (buffer[i] == '\r') {
                if ((i + 1) < buffer.size() && buffer[i + 1] == '\n') {
                    buffer[j] = '\n';
                    i += 2;
                    j += 1;
                    continue;
                }
                if ((i + 1) <= buffer.size()) {
                    buffer[j] = '\n';
                    i += 1;
                    j += 1;
                    break;
                }
            }
            buffer[j] = buffer[i];
            i += 1;
            j += 1;
        }
        buffer.resize(j);
    }

    class InternalLuaScriptsFileSystemBuilder {
    public:
        void addFile(std::string_view const& name, std::filesystem::path const& path) {
            if (name.empty()) {
                return;
            }
            auto& entry = m_entries.emplace_back();
            entry.name = name;
            entry.path = path;
            entry.variable_name = name;
            std::ranges::replace(entry.variable_name, '/', '_'); // folder/file.lua -> folder_file.lua
            std::ranges::replace(entry.variable_name, '.', '_'); // folder_file.lua -> folder_file_lua
        }

        void addDirectory(std::string_view const& name, std::filesystem::path const& path) {
            if (name.empty()) {
                return;
            }
            auto& entry = m_entries.emplace_back();
            entry.name = name;
            entry.path = path;
            entry.is_directory = true;
            if (entry.name.back() != '/') {
                entry.name.push_back('/');
            }
        }

        bool build(std::filesystem::path const& output_path) {
            if (!prepareOutputDirectory(output_path)) {
                return false;
            }

            std::ofstream f(output_path, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
            if (!f.is_open()) {
                auto const file_path = output_path.lexically_normal().generic_u8string();
                std::println("error: open file '{}' to write failed"sv, getStringView(file_path));
                return false;
            }

            f << "#include <array>\n"sv;
            f << "#include \"scripts.hpp\"\n"sv;
            f << '\n';
            f << "using std::string_view_literals::operator \"\"sv;\n"sv;
            f << "using Node = luastg::InternalLuaScriptsFileSystemNode;\n"sv;
            f << '\n';
            f << "namespace {\n"sv;
            for (auto& entry: m_entries) {
                if (entry.is_directory) {
                    continue;
                }
                std::vector<uint8_t> buffer;
                if (!readAllBytes(entry.path, buffer)) {
                    return false;
                }
                if (entry.path.has_extension() && entry.path.extension() == ".lua"sv) {
                    normalizeNewLine(buffer);
                }
                luastg::mask(buffer.data(), buffer.size());
                entry.size = buffer.size();
                if (buffer.empty()) {
                    f << "    constexpr uint8_t "sv << entry.variable_name << "[1]{};\n"sv;
                }
                else {
                    f << "    constexpr uint8_t "sv << entry.variable_name << '[' << entry.size << "]{"sv;
                    for (auto const b: buffer) {
                        f << std::to_string(b) << ',';
                    }
                    if (!buffer.empty()) {
                        f.seekp(-1, std::ofstream::cur);
                    }
                    f << "};\n"sv; // null-terminated
                }
            }
            f << "    constexpr std::array<Node const, "sv << m_entries.size() << "> s_files{\n"sv;
            for (auto& entry: m_entries) {
                if (entry.is_directory) {
                    f << "        Node{\""sv << entry.name << "\"sv, std::span<uint8_t, 0>()},\n"sv;
                }
                else {
                    f << "        Node{\""sv << entry.name << "\"sv, std::span("sv
                            << entry.variable_name << ", "sv << entry.size << ")},\n"sv;
                }
            }
            f << "    };\n"sv;
            f << "}\n"sv;
            f << '\n';
            f << "namespace luastg::generated {\n"sv;
            f << "    std::span<InternalLuaScriptsFileSystemNode const> const files(s_files);\n"sv;
            f << "}\n"sv;

            return true;
        }

        bool build(Options const& options) {
            std::error_code ec;

            for (auto const& directory_root: options.directories) {
                for (auto const& entry: std::filesystem::recursive_directory_iterator(directory_root, ec)) {
                    add(directory_root, entry);
                }
            }
            print();

            if (!build(options.output)) {
                return false;
            }

            return true;
        }

    private:
        [[nodiscard]] static bool prepareOutputDirectory(std::filesystem::path const& output_path) {
            std::error_code ec;

            auto const output_directory = output_path.parent_path();
            if (std::filesystem::exists(output_directory, ec) && !std::filesystem::is_directory(output_directory, ec)) {
                auto const directory_path = output_directory.lexically_normal().generic_u8string();
                std::println("error: '{}' is not a directory"sv, getStringView(directory_path));
                return false;
            }
            if (!std::filesystem::is_directory(output_directory, ec)) {
                if (!std::filesystem::create_directories(output_directory, ec)) {
                    auto const directory_path = output_directory.lexically_normal().generic_u8string();
                    std::println("error: create directories '{}' failed"sv, getStringView(directory_path));
                    return false;
                }
            }

            return true;
        }

        bool add(std::filesystem::path const& directory_root, std::filesystem::directory_entry const& entry) {
            std::error_code ec;
            auto const path_str = entry.path().lexically_normal().generic_u8string();
            auto const relative_path = entry.path().lexically_relative(directory_root);
            auto const relative_path_str = relative_path.lexically_normal().generic_u8string();
            if (entry.is_directory(ec)) {
                addDirectory(getStringView(relative_path_str), entry.path());
                return true;
            }
            if (entry.is_regular_file(ec)) {
                addFile(getStringView(relative_path_str), entry.path());
                return true;
            }
            return false;
        }

        void print() const {
            std::println("entries:"sv);
            for (auto const& entry: m_entries) {
                auto const& s = entry.path.u8string();
                std::println("    {} {} ({})"sv, entry.is_directory ? "d"sv : "f"sv, entry.name, getStringView(s));
            }
        }

        std::vector<InternalLuaScriptsFileSystemEntry> m_entries;
    };
}

int main() {
    Options options;
    parseCommandLineArguments(options);
    auto const verify_result = verifyOptions(options);
    if (!verify_result.has_value()) {
        std::println("error: {}"sv, verify_result.error());
        return 1;
    }
    options.print();

    if (InternalLuaScriptsFileSystemBuilder builder; !builder.build(options)) {
        return 1;
    }

    return 0;
}

#include <windows.h>

namespace {
    int toUtf8(std::wstring_view const& input) {
        return WideCharToMultiByte(
            CP_UTF8, 0,
            input.data(), static_cast<int>(input.size()),
            nullptr, 0,
            nullptr, nullptr);
    }

    int toUtf8(std::wstring_view const& input, std::string& output) {
        return WideCharToMultiByte(
            CP_UTF8, 0,
            input.data(), static_cast<int>(input.size()),
            output.data(), static_cast<int>(output.size()),
            nullptr, nullptr);
    }

    std::vector<std::string> getCommandLineArguments() {
        std::vector<std::string> args;
        auto const cmd = GetCommandLineW();
        if (cmd == nullptr) {
            return args;
        }
        int argc{};
        auto const argv = CommandLineToArgvW(cmd, &argc);
        if (argv == nullptr) {
            return args;
        }
        if (argc <= 0) {
            return args;
        }
        args.reserve(static_cast<size_t>(argc));
        for (int i = 0; i < argc; ++i) {
            std::wstring_view const arg(argv[i]);
            if (arg.ends_with(L".exe"sv)) {
                continue;
            }
            auto const cnt = toUtf8(arg);
            if (cnt <= 0) {
                continue;
            }
            auto& str = args.emplace_back(static_cast<size_t>(cnt), '\0');
            if (auto const ret = toUtf8(arg, str); cnt != ret) {
                args.pop_back();
            }
        }
        LocalFree(argv);
        return args;
    }
}
