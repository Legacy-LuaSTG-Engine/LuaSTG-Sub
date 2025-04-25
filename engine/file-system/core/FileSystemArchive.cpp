#include "FileSystemArchive.hpp"
#include "core/SmartReference.hpp"
#include <cassert>
#include <array>
#include <memory_resource>
#include <algorithm>
#include <ranges>
#include <filesystem>
#include "mz.h"
#include "mz_strm.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

#define MEMORY_RESOURCE() \
	std::array<std::byte, 256> memory_resource_stack_buffer{}; \
	std::pmr::monotonic_buffer_resource memory_resource(memory_resource_stack_buffer.data(), memory_resource_stack_buffer.size(), std::pmr::get_default_resource())

#define MEMORY_RESOURCE_STRING(NAME, SOURCE) std::pmr::string const NAME ((SOURCE), &memory_resource)

using std::string_view_literals::operator ""sv;

namespace {
	bool isSeparator(char const c) {
		return c == '/' || c == '\\';
	}
	bool isPathEndsWithSeparator(std::string_view const& path) {
		return !path.empty() && isSeparator(path.back());
	}
	bool isPathEquals(std::string_view const& a, std::string_view const& b) {
		if (a.size() != b.size()) {
			return false;
		}
		for (size_t i = 0; i < a.size(); ++i) {
			if (isSeparator(a[i]) && isSeparator(b[i])) {
				continue;
			}
			if (a[i] != b[i]) {
				return false;
			}
		}
		return true;
	}
	bool isPathStartsWith(std::string_view const& a, std::string_view const& b) {
		if (a.size() < b.size()) {
			return false;
		}
		for (size_t i = 0; i < b.size(); ++i) {
			if (isSeparator(a[i]) && isSeparator(b[i])) {
				continue;
			}
			if (a[i] != b[i]) {
				return false;
			}
		}
		return true;
	}
	bool isPatternMatched(std::string_view const& path, std::string const& pattern) {
		if (pattern.empty()) {
			return true;
		}
		if (pattern.ends_with("**"sv)) {
			auto const head = pattern.substr(0, pattern.size() - 2);
			if (isPathEndsWithSeparator(head) && isPathEquals(path, head)) {
				return false; // exclude parent level directory
			}
			return isPathStartsWith(path, head);
		}
		if (pattern.ends_with("*"sv)) {
			auto const head = pattern.substr(0, pattern.size() - 1);
			if (isPathEndsWithSeparator(head) && isPathEquals(path, head)) {
				return false; // exclude parent level directory
			}
			if (!isPathStartsWith(path, head)) {
				return false;
			}
			auto const tail = path.substr(head.size());
			auto const separator_count = std::ranges::count_if(tail, &isSeparator);
			if (separator_count == 0) {
				return true; // current level file
			}
			if (separator_count == 1 && isSeparator(tail.back())) {
				return true; // current level directory
			}
			return false; // in child directory
		}
		return isPathEquals(path, pattern);
	}
	bool isPathMatched(std::string_view const& path, std::string const& directory, bool const recursive) {
		if (!isPathStartsWith(path, directory)) {
			return false;
		}
		if (path.size() == directory.size() /* isPathEquals(path, directory) */) {
			return false; // exclude 'directory'
		}
		if (recursive) {
			return true;
		}
		auto const leaf = path.substr(directory.size());
		auto const separator_count = std::ranges::count_if(leaf, &isSeparator);
		if (separator_count == 0) {
			return true; // current level file
		}
		if (separator_count == 1 && isPathEndsWithSeparator(leaf)) {
			return true; // current level directory
		}
		return false; // in child directory
	}
	std::u8string normalizePath(std::string_view const& path) {
		std::u8string_view const directory_u8(reinterpret_cast<char8_t const*>(path.data()), path.size()); // as utf-8
		std::filesystem::path const directory_path(directory_u8);
		std::u8string normalized = directory_path.lexically_normal().generic_u8string();
		if (normalized == u8"."sv || normalized == u8"/"sv || normalized.find(u8".."sv) != std::u8string::npos) {
			normalized.clear();
		}
		return normalized;
	}
	std::string_view getStringView(std::u8string const& s) {
		return { reinterpret_cast<char const*>(s.data()), s.size() };
	}
}

namespace core {
	// IFileSystem

	bool FileSystemArchive::hasNode(std::string_view const& name) {
		if (!m_archive) {
			return false;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(name_z, name);
		if (MZ_OK != mz_zip_reader_locate_entry(m_archive, name_z.c_str(), false)) {
			return false;
		}
		return true;
	}
	FileSystemNodeType FileSystemArchive::getNodeType(std::string_view const& name) {
		if (!m_archive) {
			return FileSystemNodeType::unknown;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(name_z, name);
		if (MZ_OK != mz_zip_reader_locate_entry(m_archive, name_z.c_str(), false)) {
			return FileSystemNodeType::unknown;
		}
		if (MZ_OK == mz_zip_reader_entry_is_dir(m_archive)) {
			return FileSystemNodeType::directory;
		}
		return FileSystemNodeType::file;
	}
	bool FileSystemArchive::hasFile(std::string_view const& name) {
		if (!m_archive) {
			return false;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(name_z, name);
		if (MZ_OK != mz_zip_reader_locate_entry(m_archive, name_z.c_str(), false)) {
			return false;
		}
		return MZ_OK != mz_zip_reader_entry_is_dir(m_archive);
	}
	size_t FileSystemArchive::getFileSize(std::string_view const& name) {
		if (!m_archive) {
			return 0;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(name_z, name);
		if (MZ_OK != mz_zip_reader_locate_entry(m_archive, name_z.c_str(), false)) {
			return 0;
		}
		auto const size = mz_zip_reader_entry_save_buffer_length(m_archive);
		if (size < 0) {
			return 0;
		}
		return size;
	}
	bool FileSystemArchive::readFile(std::string_view const& name, IData** const data) {
		if (!data) {
			return false;
		}
		if (!m_archive) {
			return false;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(name_z, name);
		if (MZ_OK != mz_zip_reader_locate_entry(m_archive, name_z.c_str(), false)) {
			return false;
		}
		auto const size = mz_zip_reader_entry_save_buffer_length(m_archive);
		if (size < 0) {
			return false;
		}
		SmartReference<IData> buffer;
		if (!IData::create(static_cast<size_t>(size), buffer.put())) {
			return false;
		}
		if (MZ_OK != mz_zip_reader_entry_save_buffer(m_archive, buffer->data(), size)) {
			return false;
		}
		*data = buffer.detach();
		return true;
	}
	bool FileSystemArchive::hasDirectory(std::string_view const& name) {
		if (!m_archive) {
			return false;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(name_z, name);
		if (MZ_OK != mz_zip_reader_locate_entry(m_archive, name_z.c_str(), false)) {
			return false;
		}
		return MZ_OK == mz_zip_reader_entry_is_dir(m_archive);
	}

	bool FileSystemArchive::createEnumerator(IFileSystemEnumerator** const enumerator, std::string_view const& directory, bool const recursive) {
		if (!m_archive) {
			return false;
		}
		*enumerator = new FileSystemArchiveEnumerator(this, directory, recursive);
		return true;
	}

	// IFileSystemArchive

	bool FileSystemArchive::setPassword(std::string_view const& password) {
		if (!m_archive) {
			return false;
		}
		MEMORY_RESOURCE();
		MEMORY_RESOURCE_STRING(password_z, password);
		mz_zip_reader_set_password(m_archive, password_z.c_str());
		return true;
	}

	// FileSystemArchive

	FileSystemArchive::~FileSystemArchive() {
		if (m_archive) {
			mz_zip_reader_close(m_archive);
			mz_zip_reader_delete(&m_archive);
			m_archive = nullptr;
		}
	}

	bool FileSystemArchive::open(std::string_view const& path) {
		m_archive = mz_zip_reader_create();
		if (m_archive == nullptr) {
			return false;
		}
		m_name = path;
		if (MZ_OK != mz_zip_reader_open_file(m_archive, m_name.c_str())) {
			mz_zip_reader_delete(&m_archive);
			m_name.clear();
			m_archive = nullptr;
			return false;
		}
		return true;
	}
}
namespace core {
	bool IFileSystemArchive::createFromFile(std::string_view const& path, IFileSystemArchive** const archive) {
		SmartReference<FileSystemArchive> object;
		object.attach(new FileSystemArchive());
		if (!object->open(path)) {
			return false;
		}
		*archive = object.detach();
		return true;
	}
}
namespace core {
	// IFileSystemEnumerator

	bool FileSystemArchiveEnumerator::next() {
		bool result{};
		if (m_initialized) {
			result = MZ_OK == mz_zip_reader_goto_next_entry(m_archive->m_archive);
		}
		else {
			result = MZ_OK == mz_zip_reader_goto_first_entry(m_archive->m_archive);
			m_initialized = result;
		}
		m_available = result;
		while (!isPathMatched(getName(), m_directory, m_recursive)) {
			result = MZ_OK == mz_zip_reader_goto_next_entry(m_archive->m_archive);
			m_available = result;
			if (!result) {
				break;
			}
		}
		return result;
	}
	std::string_view FileSystemArchiveEnumerator::getName() {
		if (!m_available) {
			return "";
		}
		mz_zip_file* info{};
		if (MZ_OK != mz_zip_reader_entry_get_info(m_archive->m_archive, &info)) {
			return "";
		}
		return { info->filename, info->filename_size };
	}
	FileSystemNodeType FileSystemArchiveEnumerator::getNodeType() {
		if (!m_available) {
			return FileSystemNodeType::unknown;
		}
		if (MZ_OK == mz_zip_reader_entry_is_dir(m_archive->m_archive)) {
			return FileSystemNodeType::directory;
		}
		return FileSystemNodeType::file;
	}
	size_t FileSystemArchiveEnumerator::getFileSize() {
		if (!m_available) {
			return 0;
		}
		auto const size = mz_zip_reader_entry_save_buffer_length(m_archive->m_archive);
		if (size < 0) {
			return 0;
		}
		return size;
	}
	bool FileSystemArchiveEnumerator::readFile(IData** data) {
		if (!m_available) {
			return false;
		}
		auto const size = mz_zip_reader_entry_save_buffer_length(m_archive->m_archive);
		if (size < 0) {
			return false;
		}
		SmartReference<IData> buffer;
		if (!IData::create(static_cast<size_t>(size), buffer.put())) {
			return false;
		}
		if (MZ_OK != mz_zip_reader_entry_save_buffer(m_archive->m_archive, buffer->data(), size)) {
			return false;
		}
		*data = buffer.detach();
		return true;
	}

	// FileSystemArchiveEnumerator

	FileSystemArchiveEnumerator::FileSystemArchiveEnumerator(FileSystemArchive* const archive, std::string_view const& directory, bool const recursive)
		: m_archive(archive), m_recursive(recursive) {
		assert(archive != nullptr);
		initializeDirectory(directory);
	}

	void FileSystemArchiveEnumerator::initializeDirectory(std::string_view const& directory) {
		std::u8string const normalized = normalizePath(directory);
		if (normalized.empty()) {
			return;
		}
		if (!isPathEndsWithSeparator(getStringView(normalized))) {
			// zip style directory path
			m_directory.reserve(normalized.size() + 1);
			m_directory.append(getStringView(normalized));
			m_directory.push_back('/');
		}
		else {
			m_directory.assign(getStringView(normalized));
		}
	}
}
