#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Data.hpp"

namespace core {
	enum class FileSystemNodeType : uint8_t {
		unknown,
		file,
		directory,
	};

	struct CORE_NO_VIRTUAL_TABLE IFileSystemEnumerator : IReferenceCounted {
		virtual bool next() = 0;
		virtual std::string_view getName() = 0;
		virtual FileSystemNodeType getNodeType() = 0;
		virtual size_t getFileSize() = 0;
		virtual bool readFile(IData** data) = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystemEnumerator
	template<> constexpr InterfaceId getInterfaceId<IFileSystemEnumerator>() { return UUID::parse("49e754fe-15af-58ac-9632-d9ed06b3f0d4"); }

	struct CORE_NO_VIRTUAL_TABLE IFileSystem : IReferenceCounted {
		virtual bool hasNode(std::string_view const& name) = 0;
		virtual FileSystemNodeType getNodeType(std::string_view const& name) = 0;
		virtual bool hasFile(std::string_view const& name) = 0;
		virtual size_t getFileSize(std::string_view const& name) = 0;
		virtual bool readFile(std::string_view const& name, IData** data) = 0;
		virtual bool hasDirectory(std::string_view const& name) = 0;

		virtual bool createEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory) = 0;
		virtual bool createRecursiveEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory) = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystem
	template<> constexpr InterfaceId getInterfaceId<IFileSystem>() { return UUID::parse("61c1bc5e-6e1e-5fe2-958c-527e85138010"); }

	struct CORE_NO_VIRTUAL_TABLE IFileSystemArchive : IFileSystem {
		virtual bool setPassword(std::string_view const& password) = 0;

		static bool createFromFile(std::string_view const& path, IFileSystemArchive** archive);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystemArchive
	template<> constexpr InterfaceId getInterfaceId<IFileSystemArchive>() { return UUID::parse("a36e930b-4fb8-5061-b88b-127e5200474e"); }

	class FileSystemFactory {
		
	};
}
