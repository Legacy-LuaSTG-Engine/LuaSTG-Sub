#include "core/FileSystemOS.hpp"
#include "core/SmartReference.hpp"
#include <cassert>
#include <fstream>
#include <filesystem>

namespace {
	std::u8string_view getUtf8StringView(std::string_view const& s) {
		return { reinterpret_cast<char8_t const*>(s.data()), s.size() };
	}
}

namespace core {
	bool FileSystemOS::hasNode(std::string_view const& name) {
		return hasFile(name) || hasDirectory(name);
	}
	FileSystemNodeType FileSystemOS::getNodeType(std::string_view const& name) {
		if (hasFile(name)) {
			return FileSystemNodeType::file;
		}
		if (hasDirectory(name)) {
			return FileSystemNodeType::directory;
		}
		return FileSystemNodeType::unknown;
	}
	bool FileSystemOS::hasFile(std::string_view const& name) {
		std::error_code ec;
		return std::filesystem::is_regular_file(getUtf8StringView(name), ec);
	}
	size_t FileSystemOS::getFileSize(std::string_view const& name) {
		std::error_code ec;
		auto const size = std::filesystem::file_size(getUtf8StringView(name), ec);
		if (size == static_cast<std::uintmax_t>(-1)) {
			return 0;
		}
		return static_cast<size_t>(size);
	}
	bool FileSystemOS::readFile(std::string_view const& name, IData** const data) {
		assert(data != nullptr);
		std::filesystem::path const path(getUtf8StringView(name));
		std::ifstream file(path, std::ifstream::in | std::ifstream::binary);
		if (!file.is_open()) {
			return false;
		}
		if (!file.seekg(0, std::ifstream::end)) {
			return false;
		}
		auto const end = file.tellg();
		if (end == static_cast<std::ifstream::pos_type>(-1)) {
			return false;
		}
		if (!file.seekg(0, std::ifstream::beg)) {
			return false;
		}
		auto const begin = file.tellg();
		if (begin == static_cast<std::ifstream::pos_type>(-1)) {
			return false;
		}
		auto const size = end - begin;
		SmartReference<IData> buffer;
		if (!IData::create(static_cast<size_t>(size), buffer.put())) {
			return false;
		}
		if (!file.read(static_cast<char*>(buffer->data()), static_cast<std::streamsize>(size))) {
			return false;
		}
		*data = buffer.detach();
		return true;
	}
	bool FileSystemOS::hasDirectory(std::string_view const& name) {
		std::error_code ec;
		return std::filesystem::is_directory(getUtf8StringView(name), ec);
	}

	bool FileSystemOS::createEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory) {
		return false;
	}
	bool FileSystemOS::createRecursiveEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory) {
		return false;
	}

	IFileSystem* FileSystemOS::getInstance() {
		static FileSystemOS instance;
		return &instance;
	}
}
