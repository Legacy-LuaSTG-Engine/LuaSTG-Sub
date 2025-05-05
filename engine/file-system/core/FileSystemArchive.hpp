#pragma once
#include "core/FileSystem.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <mutex>

namespace core {
	class FileSystemArchive final : public implement::ReferenceCounted<IFileSystemArchive> {
		friend class FileSystemArchiveEnumerator;
	public:
		// IFileSystem

		bool hasNode(std::string_view const& name) override;
		FileSystemNodeType getNodeType(std::string_view const& name) override;
		bool hasFile(std::string_view const& name) override;
		size_t getFileSize(std::string_view const& name) override;
		bool readFile(std::string_view const& name, IData** data) override;
		bool hasDirectory(std::string_view const& name) override;

		bool createEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory, bool recursive) override;

		// IFileSystemArchive

		std::string_view getArchivePath() override { return m_name; }
		bool setPassword(std::string_view const& password) override;

		// FileSystemArchive

		FileSystemArchive() = default;
		FileSystemArchive(FileSystemArchive const&) = delete;
		FileSystemArchive(FileSystemArchive&&) = delete;
		~FileSystemArchive() override;

		FileSystemArchive& operator=(FileSystemArchive const&) = delete;
		FileSystemArchive& operator=(FileSystemArchive&&) = delete;

		bool open(std::string_view const& path);

	private:
		std::string m_name;
		std::string m_password;
		std::recursive_mutex m_mutex;
		void* m_archive{};
	};

	class FileSystemArchiveEnumerator final : public implement::ReferenceCounted<IFileSystemEnumerator> {
	public:
		// IFileSystemEnumerator

		bool next() override;
		std::string_view getName() override;
		FileSystemNodeType getNodeType() override;
		size_t getFileSize() override;
		bool readFile(IData** data) override;

		// FileSystemArchiveEnumerator

		FileSystemArchiveEnumerator() = delete;
		FileSystemArchiveEnumerator(FileSystemArchive* archive, std::string_view const& directory, bool recursive);
		FileSystemArchiveEnumerator(FileSystemArchiveEnumerator const&) = delete;
		FileSystemArchiveEnumerator(FileSystemArchiveEnumerator&&) = delete;
		~FileSystemArchiveEnumerator() override;

		FileSystemArchiveEnumerator& operator=(FileSystemArchiveEnumerator const&) = delete;
		FileSystemArchiveEnumerator& operator=(FileSystemArchiveEnumerator&&) = delete;

		void initializeDirectory(std::string_view const& directory);

	private:
		SmartReference<FileSystemArchive> m_archive;
		std::string m_directory;
		bool m_recursive{ false };
		bool m_initialized{ false };
		bool m_available{ false };
	};
}
