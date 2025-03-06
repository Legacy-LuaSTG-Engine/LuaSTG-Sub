#pragma once
#include "Core/Type.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace Core
{
    enum class FileType
    {
        Unknown = 0,
        File = 0x1,
        Directory = 0x2,
    };
    
    struct FileNode
    {
        FileType type = FileType::Unknown;
        std::string name;
        size_t size{};
    };
    
    struct FileNodeTree {
        virtual size_t findIndex(std::string_view const& name) = 0;
        virtual size_t getCount() = 0;
        virtual size_t getSize(size_t index) = 0;
        virtual size_t getSize(std::string_view const& path) = 0;
        virtual FileType getType(size_t index) = 0;
        virtual FileType getType(std::string_view const& name) = 0;
        virtual std::string_view getName(size_t index) = 0;
        virtual bool contain(std::string_view const& name) const = 0;
        virtual bool load(std::string_view const& name, std::vector<uint8_t>& buffer) = 0;
        virtual bool load(std::string_view const& name, IData** pp_data) = 0;
    };
    
    class FileArchive : public FileNodeTree
    {
    private:
        std::vector<FileNode> list;
        std::string name_;
        std::string password_;
        uint64_t uuid = 0;
        void* mz_zip_v = nullptr;
        void refresh();
    public:
        size_t findIndex(std::string_view const& name);
        size_t getCount();
        size_t getSize(size_t index);
        size_t getSize(std::string_view const& name);
        FileType getType(size_t index);
        FileType getType(std::string_view const& name);
        std::string_view getName(size_t index);
        [[nodiscard]] bool contain(std::string_view const& name) const;
        bool load(std::string_view const& name, std::vector<uint8_t>& buffer);
        bool load(std::string_view const& name, IData** pp_data);
    public:
        bool empty();
        uint64_t getUUID();
        std::string_view getFileArchiveName();
        bool setPassword(std::string_view const& password);
        bool loadEncrypted(std::string_view const& name, std::string_view const& password, std::vector<uint8_t>& buffer);
        bool loadEncrypted(std::string_view const& name, std::string_view const& password, IData** pp_data);
    public:
        FileArchive() = default;
        FileArchive(std::string_view const& path);
        ~FileArchive();
    };
    
    class FileManager : public FileNodeTree
    {
    private:
        std::vector<FileNode> list;
        std::vector<std::string> search_list;
        FileArchive null_archive;
        std::vector<std::shared_ptr<FileArchive>> archive;
        void refresh();
    public:
        size_t findIndex(std::string_view const& name);
        size_t getCount();
        size_t getSize(size_t index);
        size_t getSize(std::string_view const& name);
        size_t getSizeEx(std::string_view const& name);
        FileType getType(size_t index);
        FileType getType(std::string_view const& name);
        std::string_view getName(size_t index);
        [[nodiscard]] bool contain(std::string_view const& name) const;
        bool load(std::string_view const& name, std::vector<uint8_t>& buffer);
        bool load(std::string_view const& name, IData** pp_data);
    public:
        size_t getFileArchiveCount();
        FileArchive& getFileArchiveByUUID(uint64_t uuid);
        FileArchive& getFileArchive(size_t index);
        FileArchive& getFileArchive(std::string_view const& name);
        bool loadFileArchive(std::string_view const& name);
        bool loadFileArchive(std::string_view const& name, std::string_view const& password);
        bool containFileArchive(std::string_view const& name);
        void unloadFileArchive(std::string_view const& name);
        void unloadAllFileArchive();
    public:
        void addSearchPath(std::string_view const& path);
        [[nodiscard]] bool hasSearchPath(std::string_view const& path) const;
        void removeSearchPath(std::string_view const& path);
        void clearSearchPath();
    public:
        [[nodiscard]] bool containsIgnoreSearchPath(std::string_view const& name) const;
        [[nodiscard]] bool containEx(std::string_view const& name) const;
        bool loadFromFileSystemAndArchiveIgnoreSearchPath(std::string_view const& name, std::vector<uint8_t>& buffer);
        bool loadFromFileSystemAndArchiveIgnoreSearchPath(std::string_view const& name, IData** pp_data);
        bool loadEx(std::string_view const& name, std::vector<uint8_t>& buffer);
        bool loadEx(std::string_view const& name, IData** pp_data);
        bool write(std::string_view const& name, std::vector<uint8_t> const& buffer);
        bool write(std::string_view const& name, IData* p_data);
    public:
        FileManager();
        ~FileManager();
    public:
        static FileManager& get();
    };
}

inline Core::FileManager& GFileManager()
{
    return Core::FileManager::get();
}
