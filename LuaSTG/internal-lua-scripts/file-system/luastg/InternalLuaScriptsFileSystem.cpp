// ReSharper disable CppUseStructuredBinding

#include "luastg/InternalLuaScriptsFileSystem.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/SmartReference.hpp"
#include "luastg/mask.hpp"
#include "generated/scripts.hpp"
#include <ranges>
#include <algorithm>

using std::string_view_literals::operator ""sv;

namespace luastg {
    using Node = InternalLuaScriptsFileSystemNode;

    class InternalLuaScriptsFileSystem final
            : public core::implement::NoOperationReferenceCounted<IInternalLuaScriptsFileSystem> {
    public:
        bool hasNode(std::string_view const& name) override {
            return std::ranges::any_of(generated::files, [name](Node const& file) -> bool {
                return file.name == name;
            });
        }

        core::FileSystemNodeType getNodeType(std::string_view const& name) override {
            for (auto const& file: generated::files) {
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
            for (auto const& file: generated::files) {
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
            for (auto const& file: generated::files) {
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

        bool createEnumerator(core::IFileSystemEnumerator** enumerator, std::string_view const& directory,
                              bool recursive) override {
            return false;
        }
    };

    IInternalLuaScriptsFileSystem* IInternalLuaScriptsFileSystem::getInstance() {
        static InternalLuaScriptsFileSystem instance;
        return &instance;
    }
}
