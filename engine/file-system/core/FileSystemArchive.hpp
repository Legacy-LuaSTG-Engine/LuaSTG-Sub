#pragma once
#include "core/FileSystem.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <vector>

namespace core {
	class FileSystemArchive final : public implement::ReferenceCounted<IFileSystemArchive> {
	public:
		// IFileSystem

		bool hasNode(std::string_view const& name) override;
		FileSystemNodeType getNodeType(std::string_view const& name) override;
		bool hasFile(std::string_view const& name) override;
		size_t getFileSize(std::string_view const& name) override;
		bool readFile(std::string_view const& name, IData** data) override;
		bool hasDirectory(std::string_view const& name) override;

		bool createEnumerator(IFileSystemEnumerator** enumerator) override;

		// IFileSystemArchive

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
		void* m_archive{};
	};
}
