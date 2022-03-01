#include "Core/FileManager.hpp"
#include <filesystem>
#include <fstream>
#include "utility/encoding.hpp"
#include "utility/path.hpp"
#include "zip.h"

#define CUSTOM_ZIP_STAT (ZIP_STAT_INDEX | ZIP_STAT_SIZE | ZIP_STAT_ENCRYPTION_METHOD)

namespace LuaSTG::Core
{
    #define p_zip_t ((zip_t*)zip_v)
    
    static uint64_t g_uuid = 0;
    constexpr size_t invalid_index = size_t(-1);
    
    size_t FileArchive::findIndex(std::string_view const& name)
    {
        if (!p_zip_t)
        {
            return invalid_index;
        }
        std::string name_str(name);
        utility::path::to_slash(name_str);
        zip_int64_t index = zip_name_locate(p_zip_t, name_str.c_str(), ZIP_FL_ENC_GUESS);
        if (index < 0)
        {
            return invalid_index;
        }
        return (size_t)index;
    }
    size_t FileArchive::getCount()
    {
        zip_int64_t count = zip_get_num_entries(p_zip_t, ZIP_FL_UNCHANGED);
        return (count >= 0) ? count : 0;
    }
    FileType FileArchive::getType(size_t index)
    {
        if (getName(index).ends_with("/"))
        {
            return FileType::Directory;
        }
        else
        {
            return FileType::File;
        }
    }
    std::string_view FileArchive::getName(size_t index)
    {
        if ((index < 0) || (index >= getCount()))
        {
            return "";
        }
        return zip_get_name(p_zip_t, index, ZIP_FL_ENC_GUESS);
    }
    bool FileArchive::contain(std::string_view const& name)
    {
        return invalid_index != findIndex(name);
    }
    bool FileArchive::load(std::string_view const& name, std::vector<uint8_t>& buffer)
    {
        size_t index = findIndex(name);
        if (index == invalid_index)
        {
            return false;
        }
        zip_stat_t zs = {};
        if (zip_stat_index(p_zip_t, index, ZIP_FL_UNCHANGED, &zs) != 0)
        {
            return false;
        }
        if (CUSTOM_ZIP_STAT != (zs.valid & CUSTOM_ZIP_STAT))
        {
            return false;
        }
        zip_file_t* zf = zip_fopen_index(p_zip_t, index, ZIP_FL_UNCHANGED);
        if (zf)
        {
            buffer.resize((size_t)zs.size);
            zip_int64_t read = zip_fread(zf, buffer.data(), zs.size);
            zip_fclose(zf);
            return (zip_int64_t)zs.size == read;
        }
        return false;
    }
    bool FileArchive::load(std::string_view const& name, fcyMemStream** buffer)
    {
        size_t index = findIndex(name);
        if (index == invalid_index)
        {
            return false;
        }
        zip_stat_t zs = {};
        if (zip_stat_index(p_zip_t, index, ZIP_FL_UNCHANGED, &zs) != 0)
        {
            return false;
        }
        if (CUSTOM_ZIP_STAT != (zs.valid & CUSTOM_ZIP_STAT))
        {
            return false;
        }
        zip_file_t* zf = zip_fopen_index(p_zip_t, index, ZIP_FL_UNCHANGED);
        if (zf)
        {
            fcyMemStream* stream = new fcyMemStream(nullptr, zs.size, true, false);
            *buffer = stream;
            zip_int64_t read = zip_fread(zf, stream->GetInternalBuffer(), zs.size);
            zip_fclose(zf);
            if ((zip_int64_t)zs.size == read)
            {
                return true;
            }
            else
            {
                stream->Release();
                *buffer = nullptr;
                return false;
            }
        }
        return false;
    }
    
    bool FileArchive::empty()
    {
        if (!p_zip_t)
        {
            return true;
        }
        return getCount() == 0;
    }
    uint64_t FileArchive::getUUID() { return uuid; }
    std::string_view FileArchive::getFileArchiveName()
    {
        return name;
    }
    bool FileArchive::setPassword(std::string_view const& password)
    {
        std::string password_str(password);
        return zip_set_default_password(p_zip_t, password_str.c_str()) == 0;
    }
    bool FileArchive::loadEncrypted(std::string_view const& name, std::string_view const& password, std::vector<uint8_t>& buffer)
    {
        size_t index = findIndex(name);
        if (index == invalid_index)
        {
            return false;
        }
        zip_stat_t zs = {};
        if (zip_stat_index(p_zip_t, index, ZIP_FL_UNCHANGED, &zs) != 0)
        {
            return false;
        }
        if (CUSTOM_ZIP_STAT != (zs.valid & CUSTOM_ZIP_STAT))
        {
            return false;
        }
        if (ZIP_EM_NONE == zs.encryption_method)
        {
            return false;
        }
        std::string password_str(password);
        zip_file_t* zf = zip_fopen_index_encrypted(p_zip_t, index, ZIP_FL_UNCHANGED, password_str.c_str());
        if (zf)
        {
            buffer.resize((size_t)zs.size);
            zip_int64_t read = zip_fread(zf, buffer.data(), zs.size);
            zip_fclose(zf);
            return (zip_int64_t)zs.size == read;
        }
        return false;
    }
    bool FileArchive::loadEncrypted(std::string_view const& name, std::string_view const& password, fcyMemStream** buffer)
    {
        size_t index = findIndex(name);
        if (index == invalid_index)
        {
            return false;
        }
        zip_stat_t zs = {};
        if (zip_stat_index(p_zip_t, index, ZIP_FL_UNCHANGED, &zs) != 0)
        {
            return false;
        }
        if (CUSTOM_ZIP_STAT != (zs.valid & CUSTOM_ZIP_STAT))
        {
            return false;
        }
        if (ZIP_EM_NONE == zs.encryption_method)
        {
            return false;
        }
        std::string password_str(password);
        zip_file_t* zf = zip_fopen_index_encrypted(p_zip_t, index, ZIP_FL_UNCHANGED, password_str.c_str());
        if (zf)
        {
            fcyMemStream* stream = new fcyMemStream(nullptr, zs.size, true, false);
            *buffer = stream;
            zip_int64_t read = zip_fread(zf, stream->GetInternalBuffer(), zs.size);
            zip_fclose(zf);
            if ((zip_int64_t)zs.size == read)
            {
                return true;
            }
            else
            {
                stream->Release();
                *buffer = nullptr;
                return false;
            }
        }
        return false;
    }
    
    FileArchive::FileArchive(std::string_view const& path) : name(path), uuid(g_uuid++)
    {
        int err = 0;
        std::string path_str(path);
        zip_v = zip_open(path_str.c_str(), ZIP_RDONLY, &err);
    }
    FileArchive::~FileArchive()
    {
        if (p_zip_t)
        {
            zip_discard(p_zip_t);
        }
    }
    
    void FileManager::refresh()
    {
        list.clear();
        for (auto& entry : std::filesystem::recursive_directory_iterator(L"."))
        {
            list.emplace_back();
            FileNode& node = list.back();
            if (entry.is_regular_file())
            {
                node.type = FileType::File;
                node.name = utility::encoding::to_utf8(entry.path().generic_wstring()).substr(2);
            }
            else if (entry.is_directory())
            {
                node.type = FileType::Directory;
                node.name = utility::encoding::to_utf8(entry.path().generic_wstring()).substr(2);
                node.name.push_back('/');
            }
            else
            {
                list.pop_back();
            }
        }
        std::ignore = nullptr;
    }
    size_t FileManager::findIndex(std::string_view const& name)
    {
        if (list.empty())
        {
            refresh();
        }
        for (auto& v : list)
        {
            if (v.name == name)
            {
                return &v - list.data();
            }
        }
        return invalid_index;
    }
    size_t FileManager::getCount()
    {
        refresh();
        return list.size();
    }
    FileType FileManager::getType(size_t index) { return list[index].type; }
    std::string_view FileManager::getName(size_t index) { return list[index].name; }
    bool FileManager::contain(std::string_view const& name)
    {
        return std::filesystem::is_regular_file(utility::encoding::to_wide(name));
    }
    bool FileManager::load(std::string_view const& name, std::vector<uint8_t>& buffer)
    {
        std::ifstream file(utility::encoding::to_wide(name), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
        file.seekg(0, std::ios::end);
        auto end = file.tellg();
        file.seekg(0, std::ios::beg);
        auto beg = file.tellg();
        auto size = end - beg;
        buffer.resize(size);
        file.read((char*)buffer.data(), size);
        file.close();
        return true;
    }
    bool FileManager::load(std::string_view const& name, fcyMemStream** buffer)
    {
        std::ifstream file(utility::encoding::to_wide(name), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
        file.seekg(0, std::ios::end);
        auto end = file.tellg();
        file.seekg(0, std::ios::beg);
        auto beg = file.tellg();
        auto size = end - beg;
        fcyMemStream* stream = new fcyMemStream(nullptr, size, true, false);
        *buffer = stream;
        file.read((char*)stream->GetInternalBuffer(), size);
        file.close();
        return true;
    }
    
    size_t FileManager::getFileArchiveCount()
    {
        return archive.size();
    }
    FileArchive& FileManager::getFileArchiveByUUID(uint64_t uuid)
    {
        for (auto& v : archive)
        {
            if (v->getUUID() == uuid)
            {
                return *v;
            }
        }
        return null_archive;
    }
    FileArchive& FileManager::getFileArchive(size_t index)
    {
        return *archive[index];
    }
    FileArchive& FileManager::getFileArchive(std::string_view const& name)
    {
        for (auto& v : archive)
        {
            if (v->getFileArchiveName() == name)
            {
                return *v;
            }
        }
        return null_archive;
    }
    bool FileManager::loadFileArchive(std::string_view const& name)
    {
        std::shared_ptr<FileArchive> arc = std::make_shared<FileArchive>(name);
        if (arc->empty())
        {
            return false;
        }
        archive.insert(archive.begin(), arc);
        return true;
    }
    bool FileManager::loadFileArchive(std::string_view const& name, std::string_view const& password)
    {
        std::shared_ptr<FileArchive> arc = std::make_shared<FileArchive>(name);
        if (arc->empty())
        {
            return false;
        }
        arc->setPassword(password);
        archive.insert(archive.begin(), arc);
        return true;
    }
    bool FileManager::containFileArchive(std::string_view const& name)
    {
        for (auto& v : archive)
        {
            if (v->getFileArchiveName() == name)
            {
                return true;
            }
        }
        return false;
    }
    void FileManager::unloadFileArchive(std::string_view const& name)
    {
        for (auto it = archive.begin(); it != archive.end();)
        {
            if ((*it)->getFileArchiveName() == name)
            {
                it = archive.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    void FileManager::unloadAllFileArchive()
    {
        archive.clear();
    }
    
    void FileManager::addSearchPath(std::string_view const& path)
    {
        removeSearchPath(path);
        search_list.emplace_back(path);
    }
    void FileManager::removeSearchPath(std::string_view const& path)
    {
        for (auto it = search_list.begin(); it != search_list.end();)
        {
            if (*it == path)
            {
                it = search_list.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    void FileManager::clearSearchPath()
    {
        search_list.clear();
    }
    
    bool FileManager::containEx(std::string_view const& name)
    {
        auto proc = [&](std::string_view const& name) -> bool
        {
            if (contain(name))
            {
                return true;
            }
            for (auto& arc : archive)
            {
                if (arc->contain(name))
                {
                    return true;
                }
            }
            return false;
        };
        if (proc(name))
        {
            return true;
        }
        for (auto& p : search_list)
        {
            std::string path(p); path.append(name);
            if (proc(path))
            {
                return true;
            }
        }
        return false;
    }
    bool FileManager::loadEx(std::string_view const& name, std::vector<uint8_t>& buffer)
    {
        auto proc = [&](std::string_view const& name, std::vector<uint8_t>& buffer) -> bool
        {
            for (auto& arc : archive)
            {
                if (arc->load(name, buffer))
                {
                    return true;
                }
            }
            if (load(name, buffer))
            {
                return true;
            }
            return false;
        };
        if (proc(name, buffer))
        {
            return true;
        }
        for (auto& p : search_list)
        {
            std::string path(p); path.append(name);
            if (proc(path, buffer))
            {
                return true;
            }
        }
        return false;
    }
    bool FileManager::loadEx(std::string_view const& name, fcyMemStream** buffer)
    {
        auto proc = [&](std::string_view const& name, fcyMemStream** buffer) -> bool
        {
            for (auto& arc : archive)
            {
                if (arc->load(name, buffer))
                {
                    return true;
                }
            }
            if (load(name, buffer))
            {
                return true;
            }
            return false;
        };
        if (proc(name, buffer))
        {
            return true;
        }
        for (auto& p : search_list)
        {
            std::string path(p); path.append(name);
            if (proc(path, buffer))
            {
                return true;
            }
        }
        return false;
    }
    
    FileManager::FileManager()
    {
    }
    FileManager::~FileManager()
    {
    }
    
    FileManager& FileManager::get()
    {
        static FileManager instance;
        return instance;
    }
}
