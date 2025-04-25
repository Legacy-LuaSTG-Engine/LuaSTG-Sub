#pragma once
#include "core/FileSystem.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <filesystem>

namespace core {
	class FileSystemOS final : public implement::NoOperationReferenceCounted<IFileSystem> {
	public:
		bool hasNode(std::string_view const& name) override;
		FileSystemNodeType getNodeType(std::string_view const& name) override;
		bool hasFile(std::string_view const& name) override;
		size_t getFileSize(std::string_view const& name) override;
		bool readFile(std::string_view const& name, IData** data) override;
		bool hasDirectory(std::string_view const& name) override;

		bool createEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory, bool recursive = false) override;

		static IFileSystem* getInstance();
	};

	class FileSystemOsEnumerator final : public implement::ReferenceCounted<IFileSystemEnumerator> {
	public:
		// IFileSystemEnumerator

		bool next() override;
		std::string_view getName() override;
		FileSystemNodeType getNodeType() override;
		size_t getFileSize() override;
		bool readFile(IData** data) override;

		// FileSystemOsEnumerator

		FileSystemOsEnumerator() = delete;
		explicit FileSystemOsEnumerator(std::string_view const& directory);
		FileSystemOsEnumerator(FileSystemOsEnumerator const&) = delete;
		FileSystemOsEnumerator(FileSystemOsEnumerator&&) = delete;
		~FileSystemOsEnumerator() override = default;

		FileSystemOsEnumerator& operator=(FileSystemOsEnumerator const&) = delete;
		FileSystemOsEnumerator& operator=(FileSystemOsEnumerator&&) = delete;

		void initializeDirectory(std::string_view const& directory);

	private:
		std::filesystem::directory_iterator m_iterator;
		std::filesystem::directory_iterator m_end;
		std::string m_directory;
		std::string m_path;
		bool m_initialized{ false };
		bool m_available{ false };
	};

	class FileSystemOsRecursiveEnumerator final : public implement::ReferenceCounted<IFileSystemEnumerator> {
	public:
		// IFileSystemEnumerator

		bool next() override;
		std::string_view getName() override;
		FileSystemNodeType getNodeType() override;
		size_t getFileSize() override;
		bool readFile(IData** data) override;

		// FileSystemOsEnumerator

		FileSystemOsRecursiveEnumerator() = delete;
		explicit FileSystemOsRecursiveEnumerator(std::string_view const& directory);
		FileSystemOsRecursiveEnumerator(FileSystemOsRecursiveEnumerator const&) = delete;
		FileSystemOsRecursiveEnumerator(FileSystemOsRecursiveEnumerator&&) = delete;
		~FileSystemOsRecursiveEnumerator() override = default;

		FileSystemOsRecursiveEnumerator& operator=(FileSystemOsRecursiveEnumerator const&) = delete;
		FileSystemOsRecursiveEnumerator& operator=(FileSystemOsRecursiveEnumerator&&) = delete;

		void initializeDirectory(std::string_view const& directory);

	private:
		std::filesystem::recursive_directory_iterator m_iterator;
		std::filesystem::recursive_directory_iterator m_end;
		std::string m_directory;
		std::string m_path;
		bool m_initialized{ false };
		bool m_available{ false };
	};
}
