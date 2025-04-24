#include "FileSystemArchive.hpp"
#include "core/SmartReference.hpp"
#include <array>
#include <memory_resource>
#include "mz.h"
#include "mz_strm.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

#define MEMORY_RESOURCE() \
	std::array<std::byte, 256> memory_resource_stack_buffer{}; \
	std::pmr::monotonic_buffer_resource memory_resource(memory_resource_stack_buffer.data(), memory_resource_stack_buffer.size(), std::pmr::get_default_resource())

#define MEMORY_RESOURCE_STRING(NAME, SOURCE) std::pmr::string const NAME ((SOURCE), &memory_resource)

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

	bool FileSystemArchive::createEnumerator(IFileSystemEnumerator** enumerator) {
		if (!m_archive) {
			return false;
		}
		*enumerator = new FileSystemArchiveEnumerator(this);
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
		return false;
	}
	std::string_view FileSystemArchiveEnumerator::getName() {
		return "";
	}
	FileSystemNodeType FileSystemArchiveEnumerator::getNodeType() {
		return FileSystemNodeType::unknown;
	}
	size_t FileSystemArchiveEnumerator::getFileSize() {
		return 0;
	}
	bool FileSystemArchiveEnumerator::readFile(IData** data) {
		std::ignore = data;
		return false;
	}

	// FileSystemArchiveEnumerator

	FileSystemArchiveEnumerator::FileSystemArchiveEnumerator(FileSystemArchive* archive) : m_archive(archive) {}
}
