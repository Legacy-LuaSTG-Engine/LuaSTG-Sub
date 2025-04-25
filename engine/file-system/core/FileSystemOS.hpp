#pragma once
#include "core/FileSystem.hpp"
#include "core/implement/ReferenceCounted.hpp"

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
}
