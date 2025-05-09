// ReSharper disable CppUseStructuredBinding

#include "luastg/EmbeddedFileSystem.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/SmartReference.hpp"
#include "luastg/mask.hpp"
#include "generated/scripts.hpp"
#include "core/FileSystemCommon.hpp"
#include <ranges>
#include <algorithm>

using std::string_view_literals::operator ""sv;

namespace luastg {
	using Node = InternalLuaScriptsFileSystemNode;

	class EmbeddedFileSystemEnumerator final : public core::implement::ReferenceCounted<core::IFileSystemEnumerator> {
	public:
		bool next() override {
			if (generated::files.empty()) {
				return false;
			}
			if (m_index == SIZE_MAX) {
				m_index = 0;
				m_available = true;
			}
			while (!isPathMatched(getName(), m_directory, m_recursive)) {
				++m_index;
				m_available = m_index < generated::files.size();
				if (!m_available) {
					break;
				}
			}
			return m_available;
		}
		std::string_view getName() override {
			if (!m_available) {
				return ""sv;
			}
			return generated::files[m_index].name;
		}
		core::FileSystemNodeType getNodeType() override {
			if (!m_available) {
				return core::FileSystemNodeType::unknown;
			}
			return generated::files[m_index].name.ends_with("/"sv)
				? core::FileSystemNodeType::directory
				: core::FileSystemNodeType::file;
		}
		size_t getFileSize() override {
			if (!m_available) {
				return 0;
			}
			return generated::files[m_index].data.size();
		}
		bool readFile(core::IData** data) override {
			if (!m_available) {
				return false;
			}
			auto const& file_data = generated::files[m_index].data;
			core::SmartReference<core::IData> temp;
			if (!core::IData::create(file_data.size(), temp.put())) {
				return false;
			}
			std::memcpy(temp->data(), file_data.data(), file_data.size());
			mask(temp->data(), temp->size());
			*data = temp.detach();
			return true;
		}

		EmbeddedFileSystemEnumerator(std::string_view const& directory, bool const recursive) : m_recursive(recursive) {
			initializeDirectory(directory);
		}
		EmbeddedFileSystemEnumerator(EmbeddedFileSystemEnumerator const&) = delete;
		EmbeddedFileSystemEnumerator(EmbeddedFileSystemEnumerator&&) = delete;
		~EmbeddedFileSystemEnumerator() override = default;

		EmbeddedFileSystemEnumerator& operator=(EmbeddedFileSystemEnumerator const&) = delete;
		EmbeddedFileSystemEnumerator& operator=(EmbeddedFileSystemEnumerator&&) = delete;

		void initializeDirectory(std::string_view const& directory) {
			if (directory.empty()) {
				return;
			}
			std::u8string const normalized = normalizePath(directory, true);
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

	private:
		std::string m_directory;
		size_t m_index{ SIZE_MAX };
		bool m_available{ false };
		bool m_recursive{ false };
	};

	class EmbeddedFileSystem final
		: public core::implement::NoOperationReferenceCounted<IEmbeddedFileSystem> {
	public:
		bool hasNode(std::string_view const& name) override {
			return std::ranges::any_of(generated::files, [name](Node const& file) -> bool {
				return file.name == name;
			});
		}
		core::FileSystemNodeType getNodeType(std::string_view const& name) override {
			for (auto const& file : generated::files) {
				if (file.name == name) {
					return file.name.ends_with('/')
						? core::FileSystemNodeType::directory
						: core::FileSystemNodeType::file;
				}
			}
			return core::FileSystemNodeType::unknown;
		}
		bool hasFile(std::string_view const& name) override {
			return getNodeType(name) == core::FileSystemNodeType::file;
		}
		size_t getFileSize(std::string_view const& name) override {
			for (auto const& file : generated::files) {
				if (file.name == name) {
					return file.data.size();
				}
			}
			return 0;
		}
		bool readFile(std::string_view const& name, core::IData** data) override {
			if (data == nullptr) {
				assert(false);
				return false;
			}
			for (auto const& file : generated::files) {
				if (file.name == name) {
					core::SmartReference<core::IData> temp;
					if (!core::IData::create(file.data.size(), temp.put())) {
						return false;
					}
					std::memcpy(temp->data(), file.data.data(), file.data.size());
					mask(temp->data(), temp->size());
					*data = temp.detach();
					return true;
				}
			}
			return false;
		}
		bool hasDirectory(std::string_view const& name) override {
			return getNodeType(name) == core::FileSystemNodeType::directory;
		}
		bool createEnumerator(core::IFileSystemEnumerator** enumerator, std::string_view const& directory, bool recursive) override {
			assert(enumerator != nullptr);
			*enumerator = new EmbeddedFileSystemEnumerator(directory, recursive);
			return true;
		}
	};

	IEmbeddedFileSystem* IEmbeddedFileSystem::getInstance() {
		static EmbeddedFileSystem instance;
		return &instance;
	}
}
