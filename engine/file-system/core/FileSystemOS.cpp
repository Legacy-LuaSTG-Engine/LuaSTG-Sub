#include "core/FileSystemOS.hpp"
#include "core/FileSystemWindows.hpp"
#include "core/SmartReference.hpp"
#include "core/Logger.hpp"
#include "core/FileSystemCommon.hpp"
#include <cassert>
#include <fstream>

using std::string_view_literals::operator ""sv;

namespace {
	bool readFileData(std::filesystem::path const& path, core::IData** const data) {
		assert(data != nullptr);
		if (std::string correct; !win32::isFilePathCaseCorrect(path, correct)) {
			auto const name = path.u8string();
			core::Logger::error("[core] There is a difference in case between file paths '{}' and '{}'", getStringView(name), correct);
			return false;
		}
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
		core::SmartReference<core::IData> buffer;
		if (!core::IData::create(static_cast<size_t>(size), buffer.put())) {
			return false;
		}
		if (!file.read(static_cast<char*>(buffer->data()), static_cast<std::streamsize>(size))) {
			return false;
		}
		*data = buffer.detach();
		return true;
	}
	bool readFileData(std::string_view const& name, core::IData** const data) {
		assert(data != nullptr);
		std::filesystem::path const path(getUtf8StringView(name));
		return readFileData(path, data);
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
	bool FileSystemOS::readFile(std::string_view const& name, IData** const data) { return readFileData(name, data); }
	bool FileSystemOS::hasDirectory(std::string_view const& name) {
		std::error_code ec;
		return std::filesystem::is_directory(getUtf8StringView(name), ec);
	}

	bool FileSystemOS::createEnumerator(IFileSystemEnumerator** const enumerator, std::string_view const& directory, bool const recursive) {
		assert(enumerator != nullptr);
		SmartReference<IFileSystemEnumerator> temp;
		if (recursive) {
			temp.attach(new FileSystemOsRecursiveEnumerator(directory));
		}
		else {
			temp.attach(new FileSystemOsEnumerator(directory));
		}
		*enumerator = temp.detach();
		return true;
	}

	IFileSystemOS* IFileSystemOS::getInstance() {
		static FileSystemOS instance;
		return &instance;
	}
}
namespace core {
	// IFileSystemEnumerator

	bool FileSystemOsEnumerator::next() {
		std::error_code ec;
		m_path.clear();
		if (m_initialized) {
			m_iterator.increment(ec);
		}
		else {
			m_iterator = std::filesystem::directory_iterator(m_directory, ec);
			m_end = std::filesystem::end(m_iterator);
			m_initialized = !ec;
		}
		m_available = !ec && m_iterator != m_end;
		return m_available;
	}
	std::string_view FileSystemOsEnumerator::getName() {
		if (!m_available) {
			return "";
		}
		if (m_path.empty()) {
			auto const path = m_iterator->path().lexically_normal().generic_u8string();
			m_path.assign(getStringView(path));
			if (std::error_code ec; m_iterator->is_directory(ec) && !ec) {
				if (!m_path.empty() && !isPathEndsWithSeparator(m_path)) {
					m_path.push_back('/'); // zip style
				}
			}
		}
		return m_path;
	}
	FileSystemNodeType FileSystemOsEnumerator::getNodeType() {
		if (!m_available) {
			return FileSystemNodeType::unknown;
		}
		if (std::error_code ec; m_iterator->is_directory(ec) && !ec) {
			return FileSystemNodeType::directory;
		}
		if (std::error_code ec; m_iterator->is_regular_file(ec) && !ec) {
			return FileSystemNodeType::file;
		}
		return FileSystemNodeType::unknown;
	}
	size_t FileSystemOsEnumerator::getFileSize() {
		if (!m_available) {
			return 0;
		}
		std::error_code ec;
		auto const size = m_iterator->file_size(ec);
		if (size == static_cast<std::uintmax_t>(-1)) {
			return 0;
		}
		return static_cast<size_t>(size);
	}
	bool FileSystemOsEnumerator::readFile(IData** const data) {
		if (!m_available) {
			return false;
		}
		return readFileData(m_iterator->path(), data);
	}

	// FileSystemOsEnumerator

	FileSystemOsEnumerator::FileSystemOsEnumerator(std::string_view const& directory) {
		initializeDirectory(directory);
	}

	void FileSystemOsEnumerator::initializeDirectory(std::string_view const& directory) {
		std::u8string const normalized = normalizePath(directory);
		if (normalized.empty()) {
			m_directory.assign("."sv); // root directory
		}
		else {
			m_directory.assign(getStringView(normalized));
		}
	}
}
namespace core {
	// IFileSystemEnumerator

	bool FileSystemOsRecursiveEnumerator::next() {
		std::error_code ec;
		m_path.clear();
		if (m_initialized) {
			m_iterator.increment(ec);
		}
		else {
			m_iterator = std::filesystem::recursive_directory_iterator(m_directory, ec);
			m_end = std::filesystem::end(m_iterator);
			m_initialized = !ec;
		}
		m_available = !ec && m_iterator != m_end;
		return m_available;
	}
	std::string_view FileSystemOsRecursiveEnumerator::getName() {
		if (!m_available) {
			return "";
		}
		if (m_path.empty()) {
			auto const path = m_iterator->path().lexically_normal().generic_u8string();
			m_path.assign(getStringView(path));
			if (std::error_code ec; m_iterator->is_directory(ec) && !ec) {
				if (!m_path.empty() && !isPathEndsWithSeparator(m_path)) {
					m_path.push_back('/'); // zip style
				}
			}
		}
		return m_path;
	}
	FileSystemNodeType FileSystemOsRecursiveEnumerator::getNodeType() {
		if (!m_available) {
			return FileSystemNodeType::unknown;
		}
		if (std::error_code ec; m_iterator->is_directory(ec) && !ec) {
			return FileSystemNodeType::directory;
		}
		if (std::error_code ec; m_iterator->is_regular_file(ec) && !ec) {
			return FileSystemNodeType::file;
		}
		return FileSystemNodeType::unknown;
	}
	size_t FileSystemOsRecursiveEnumerator::getFileSize() {
		if (!m_available) {
			return 0;
		}
		std::error_code ec;
		auto const size = m_iterator->file_size(ec);
		if (size == static_cast<std::uintmax_t>(-1)) {
			return 0;
		}
		return static_cast<size_t>(size);
	}
	bool FileSystemOsRecursiveEnumerator::readFile(IData** const data) {
		if (!m_available) {
			return false;
		}
		return readFileData(m_iterator->path(), data);
	}

	// FileSystemOsRecursiveEnumerator

	FileSystemOsRecursiveEnumerator::FileSystemOsRecursiveEnumerator(std::string_view const& directory) {
		initializeDirectory(directory);
	}

	void FileSystemOsRecursiveEnumerator::initializeDirectory(std::string_view const& directory) {
		std::u8string const normalized = normalizePath(directory);
		if (normalized.empty()) {
			m_directory.assign("."sv); // root directory
		}
		else {
			m_directory.assign(getStringView(normalized));
		}
	}
}
