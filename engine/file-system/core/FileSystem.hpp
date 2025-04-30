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

		virtual bool createEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory, bool recursive = false) = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystem
	template<> constexpr InterfaceId getInterfaceId<IFileSystem>() { return UUID::parse("61c1bc5e-6e1e-5fe2-958c-527e85138010"); }

	struct CORE_NO_VIRTUAL_TABLE IFileSystemOS : IFileSystem {
		static IFileSystemOS* getInstance();
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystemOS
	template<> constexpr InterfaceId getInterfaceId<IFileSystemOS>() { return UUID::parse("f21693cb-e71a-5877-9786-7815d4af4ffd"); }

	struct CORE_NO_VIRTUAL_TABLE IFileSystemArchive : IFileSystem {
		virtual std::string_view getArchivePath() = 0;
		virtual bool setPassword(std::string_view const& password) = 0;

		static bool createFromFile(std::string_view const& path, IFileSystemArchive** archive);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystemArchive
	template<> constexpr InterfaceId getInterfaceId<IFileSystemArchive>() { return UUID::parse("a36e930b-4fb8-5061-b88b-127e5200474e"); }

	struct CORE_NO_VIRTUAL_TABLE IFileSystemFileSystemEnumerator : IReferenceCounted {
		virtual bool next(IFileSystem** output) = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IFileSystemFileSystemEnumerator
	template<> constexpr InterfaceId getInterfaceId<IFileSystemFileSystemEnumerator>() { return UUID::parse("21f75b75-0d43-5fa1-93d0-9924036782b6"); }

	class FileSystemManager {
	public:
		static void addFileSystem(std::string_view const& name, IFileSystem* file_system);
		static bool hasFileSystem(std::string_view const& name);
		static void removeFileSystem(std::string_view const& name);
		static void removeFileSystem(IFileSystem* file_system);
		static void removeAllFileSystem();
		static bool createFileSystemEnumerator(IFileSystemFileSystemEnumerator** enumerator);

		static bool getFileSystemArchiveByPath(std::string_view const& path, IFileSystemArchive** output);

		static void addSearchPath(std::string_view const& path);
		static bool hasSearchPath(std::string_view const& path);
		static void removeSearchPath(std::string_view const& path);
		static void removeAllSearchPath();
		static void resolveLocation(std::string_view const& path, IFileSystemEnumerator** enumerator);

		static bool hasNode(std::string_view const& name);
		static FileSystemNodeType getNodeType(std::string_view const& name);
		static bool hasFile(std::string_view const& name);
		static size_t getFileSize(std::string_view const& name);;
		static bool readFile(std::string_view const& name, IData** data);
		static bool hasDirectory(std::string_view const& name);

		static bool writeFile(std::string_view const& name, IData* data);
		static bool createEnumerator(IFileSystemEnumerator** enumerator, std::string_view const& directory, bool recursive = false);
	};
}
