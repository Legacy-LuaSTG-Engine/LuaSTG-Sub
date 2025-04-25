// ReSharper disable CppUseStructuredBinding

#include "core/FileSystem.hpp"
#include "core/FileSystemOS.hpp"
#include "core/SmartReference.hpp"
#include <cassert>
#include <vector>
#include <mutex>
#include <ranges>
#include <filesystem>

using std::string_view_literals::operator ""sv;

namespace {
	std::u8string_view getUtf8StringView(std::string_view const& s) {
		return { reinterpret_cast<char8_t const*>(s.data()), s.size() };
	}
	std::string_view getStringView(std::u8string_view const& s) {
		return { reinterpret_cast<char const*>(s.data()), s.size() };
	}

	struct NamedFileSystem {
		core::SmartReference<core::IFileSystem> file_system;
		std::string name;
	};

	std::recursive_mutex s_file_systems_mutex;
	std::vector<NamedFileSystem> s_file_systems;

	std::recursive_mutex s_search_paths_mutex;
	std::vector<std::string> s_search_paths;

	enum class ResourceLocationSchema : uint8_t {
		unknown,
		resource,
		user,
	};
	struct ResourceLocation {
		std::string_view file_system;
		std::string_view path;
		ResourceLocationSchema schema{ ResourceLocationSchema::unknown };

		static ResourceLocation parse(std::string_view const& s) {
			constexpr auto schema_resource{ "resource://"sv };
			constexpr auto schema_resource_short{ "res://"sv };
			constexpr auto schema_user{ "user://"sv };

			std::string_view v(s);
			bool merged{ false };
			ResourceLocation location;

			if (v.starts_with(schema_resource)) {
				location.schema = ResourceLocationSchema::resource;
				v = v.substr(schema_resource.size());
			}
			else if (v.starts_with(schema_resource_short)) {
				location.schema = ResourceLocationSchema::resource;
				v = v.substr(schema_resource_short.size());
			}
			else if (v.starts_with(schema_user)) {
				location.schema = ResourceLocationSchema::user;
				v = v.substr(schema_user.size());
			}
			else {
				location.schema = ResourceLocationSchema::resource;
				merged = true;
			}

			if (location.schema == ResourceLocationSchema::resource) {
				if (merged) {
					location.path = v;
				}
				else {
					auto p = v.find_first_of('/');
					if (p == std::string_view::npos) {
						p = v.find_first_of('\\');
					}
					if (p != std::string_view::npos) {
						location.file_system = v.substr(0, p);
						location.path = v.substr(p + 1);
					}
				}
			}

			return location;
		}
	};

	std::u8string join(std::string_view const& a, std::string_view const& b) {
		std::filesystem::path p(getUtf8StringView(a));
		p /= getUtf8StringView(b);
		return p.lexically_normal().generic_u8string();
	}

	struct ResolvedResourceLocation {
		core::IFileSystem* file_system{};
		std::string path;
	};

	ResolvedResourceLocation resolve(ResourceLocation const& l) {
		if (l.schema != ResourceLocationSchema::resource && l.schema != ResourceLocationSchema::user) {
			return {};
		}

	#define RESULT(FS, P) ResolvedResourceLocation{ .file_system = (FS), .path = std::string(P) }

		if (l.schema == ResourceLocationSchema::resource) {
			for (auto const& v : s_file_systems) {
				if (!l.file_system.empty() && v.name != l.file_system) {
					continue;
				}
				for (auto const& s : s_search_paths | std::ranges::views::reverse) {
					auto const p = join(s, l.path);
					if (p.find(u8".."sv) != std::u8string::npos) {
						continue; // not allowed for archive
					}
					if (v.file_system->hasNode(getStringView(p))) {
						return RESULT(v.file_system.get(), getStringView(p));
					}
				}
				if (v.file_system->hasNode(l.path)) {
					return RESULT(v.file_system.get(), l.path);
				}
			}
		}

		// TODO: read configuration
		auto const os_file_system = core::FileSystemOS::getInstance();
		for (auto const& s : s_search_paths | std::ranges::views::reverse) {
			auto const p = join(s, l.path);
			if (os_file_system->hasNode(getStringView(p))) {
				return RESULT(os_file_system, getStringView(p));
			}
		}
		if (os_file_system->hasNode(l.path)) {
			return RESULT(os_file_system, l.path);
		}

	#undef RESULT

		return {};
	}
}

namespace core {
	void FileSystemManager::addFileSystem(std::string_view const& name, IFileSystem* const file_system) {
		assert(!name.empty());
		assert(file_system != nullptr);
		if (name.empty()) {
			return;
		}
		if (file_system == nullptr) {
			return;
		}
		[[maybe_unused]] std::lock_guard lock(s_file_systems_mutex);
		for (auto const& v : s_file_systems) {
			if (v.name == name) {
				return;
			}
		}
		auto& v = s_file_systems.emplace_back();
		v.file_system = file_system;
		v.name = name;
	}
	bool FileSystemManager::hasFileSystem(std::string_view const& name) {
		assert(!name.empty());
		if (name.empty()) {
			return false;
		}
		[[maybe_unused]] std::lock_guard lock(s_file_systems_mutex);
		for (auto const& v : s_file_systems) {
			if (v.name == name) {
				return true;
			}
		}
		return false;
	}
	void FileSystemManager::removeFileSystem(std::string_view const& name) {
		assert(!name.empty());
		if (name.empty()) {
			return;
		}
		[[maybe_unused]] std::lock_guard lock(s_file_systems_mutex);
		for (auto it = s_file_systems.begin(); it != s_file_systems.end();) {
			if (it->name == name) {
				it = s_file_systems.erase(it);
			}
			else {
				++it;
			}
		}
	}
	void FileSystemManager::removeAllFileSystem() {
		[[maybe_unused]] std::lock_guard lock(s_file_systems_mutex);
		s_file_systems.clear();
	}

	void FileSystemManager::addSearchPath(std::string_view const& path) {
		[[maybe_unused]] std::lock_guard lock(s_search_paths_mutex);
		for (auto const& v : s_search_paths) {
			if (v == path) {
				return;
			}
		}
		s_search_paths.emplace_back(path);
	}
	bool FileSystemManager::hasSearchPath(std::string_view const& path) {
		[[maybe_unused]] std::lock_guard lock(s_search_paths_mutex);
		for (auto const& v : s_search_paths) {
			if (v == path) {
				return true;
			}
		}
		return false;
	}
	void FileSystemManager::removeSearchPath(std::string_view const& path) {
		[[maybe_unused]] std::lock_guard lock(s_search_paths_mutex);
		for (auto it = s_search_paths.begin(); it != s_search_paths.end();) {
			if (*it == path) {
				it = s_search_paths.erase(it);
			}
			else {
				++it;
			}
		}
	}
	void FileSystemManager::removeAllSearchPath() {
		[[maybe_unused]] std::lock_guard lock(s_search_paths_mutex);
		s_search_paths.clear();
	}

	bool FileSystemManager::hasNode(std::string_view const& name) {
		auto const l = ResourceLocation::parse(name);
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		auto const r = resolve(l);
		return r.file_system != nullptr;
	}
	FileSystemNodeType FileSystemManager::getNodeType(std::string_view const& name) {
		auto const l = ResourceLocation::parse(name);
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		auto const r = resolve(l);
		if (r.file_system == nullptr) {
			return FileSystemNodeType::unknown;
		}
		return r.file_system->getNodeType(r.path);
	}
	bool FileSystemManager::hasFile(std::string_view const& name) {
		auto const l = ResourceLocation::parse(name);
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		auto const r = resolve(l);
		if (r.file_system == nullptr) {
			return false;
		}
		return r.file_system->hasFile(r.path);
	}
	size_t FileSystemManager::getFileSize(std::string_view const& name) {
		auto const l = ResourceLocation::parse(name);
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		auto const r = resolve(l);
		if (r.file_system == nullptr) {
			return 0;
		}
		return r.file_system->getFileSize(r.path);
	}
	bool FileSystemManager::readFile(std::string_view const& name, IData** const data) {
		assert(data != nullptr);
		if (data == nullptr) {
			return false;
		}
		auto const l = ResourceLocation::parse(name);
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		auto const r = resolve(l);
		if (r.file_system == nullptr) {
			return false;
		}
		return r.file_system->readFile(r.path, data);
	}
	bool FileSystemManager::hasDirectory(std::string_view const& name) {
		auto const l = ResourceLocation::parse(name);
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		auto const r = resolve(l);
		if (r.file_system == nullptr) {
			return false;
		}
		return r.file_system->hasDirectory(r.path);
	}

	bool FileSystemManager::createEnumerator(IFileSystemEnumerator** const enumerator, std::string_view const& directory, bool const recursive) {
		[[maybe_unused]] std::lock_guard lock_file_systems(s_file_systems_mutex);
		[[maybe_unused]] std::lock_guard lock_search_paths(s_search_paths_mutex);
		return false;
	}
}
