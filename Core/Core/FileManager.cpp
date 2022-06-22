#include "Core/FileManager.hpp"
#include <filesystem>
#include <fstream>
#include "utility/encoding.hpp"
#include "utility/path.hpp"
#include "mz.h"
#include "mz_strm.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

namespace Core
{
    static uint64_t g_uuid = 0;
    constexpr size_t invalid_index = size_t(-1);
    
    // FileArchive

    struct mz_zip_scope_password
    {
        void* mz_zip_v = nullptr;
        std::string_view password_v;
        mz_zip_scope_password(void* p, std::string_view const& password, std::string_view const& scope_pw) : mz_zip_v(p), password_v(password)
        {
            if (mz_zip_v)
            {
                mz_zip_reader_set_password(mz_zip_v, scope_pw.data());
            }
        }
        ~mz_zip_scope_password()
        {
            if (mz_zip_v)
            {
                mz_zip_reader_set_password(mz_zip_v, password_v.data());
            }
        }
    };

    void FileArchive::refresh()
    {
        list.clear();
        if (!mz_zip_v)
        {
            return;
        }
        if (MZ_OK != mz_zip_reader_goto_first_entry(mz_zip_v))
        {
            return;
        }
        do
        {
            mz_zip_file* mz_zip_file_v = nullptr;
            if (MZ_OK == mz_zip_reader_entry_get_info(mz_zip_v, &mz_zip_file_v))
            {
                bool is_dir = (MZ_OK == mz_zip_reader_entry_is_dir(mz_zip_v));
                list.emplace_back(FileNode{
                    .type = is_dir ? FileType::Directory : FileType::File,
                    .name = mz_zip_file_v->filename,
                });
            }
        } while (MZ_OK == mz_zip_reader_goto_next_entry(mz_zip_v));
    }
    size_t FileArchive::findIndex(std::string_view const& name)
    {
        if (list.empty())
        {
            refresh();
            if (list.empty())
            {
                return invalid_index;
            }
        }
        for (auto const& v : list)
        {
            if (v.name == name)
            {
                return &v - list.data();
            }
        }
        return invalid_index;
    }
    size_t FileArchive::getCount()
    {
        if (list.empty())
        {
            refresh();
        }
        return list.size();
    }
    FileType FileArchive::getType(size_t index)
    {
        if ((index < 0) || (index >= getCount()))
        {
            return FileType::Unknown;
        }
        return list[index].type;
    }
    FileType FileArchive::getType(std::string_view const& name)
    {
        if (!mz_zip_v)
        {
            return FileType::Unknown;
        }
        if (MZ_OK != mz_zip_reader_locate_entry(mz_zip_v, name.data(), false))
        {
            return FileType::Unknown;
        }
        bool const is_dir = (MZ_OK == mz_zip_reader_entry_is_dir(mz_zip_v));
        return is_dir ? FileType::Directory : FileType::File;
    }
    std::string_view FileArchive::getName(size_t index)
    {
        if ((index < 0) || (index >= getCount()))
        {
            return "";
        }
        return list[index].name;
    }
    bool FileArchive::contain(std::string_view const& name)
    {
        if (!mz_zip_v)
        {
            return false;
        }
        if (MZ_OK != mz_zip_reader_locate_entry(mz_zip_v, name.data(), false))
        {
            return false;
        }
        return MZ_OK != mz_zip_reader_entry_is_dir(mz_zip_v);
    }
    bool FileArchive::load(std::string_view const& name, std::vector<uint8_t>& buffer)
    {
        if (!mz_zip_v)
        {
            return false;
        }
        if (MZ_OK != mz_zip_reader_locate_entry(mz_zip_v, name.data(), false))
        {
            return false;
        }
        int32_t script_size = mz_zip_reader_entry_save_buffer_length(mz_zip_v);
        if (script_size < 0)
        {
            return false;
        }
        try
        {
            buffer.resize(script_size);
        }
        catch (...)
        {
            return false;
        }
        return MZ_OK == mz_zip_reader_entry_save_buffer(mz_zip_v, buffer.data(), script_size);
    }
    
    bool FileArchive::empty()
    {
        if (!mz_zip_v)
        {
            return true;
        }
        if (MZ_OK != mz_zip_reader_goto_first_entry(mz_zip_v))
        {
            return true; // test failed...
        }
        return false;
    }
    uint64_t FileArchive::getUUID() { return uuid; }
    std::string_view FileArchive::getFileArchiveName()
    {
        return name_;
    }
    bool FileArchive::setPassword(std::string_view const& password)
    {
        if (!mz_zip_v)
        {
            return false;
        }
        password_ = password;
        mz_zip_reader_set_password(mz_zip_v, password_.c_str());
        return true;
    }
    bool FileArchive::loadEncrypted(std::string_view const& name, std::string_view const& password, std::vector<uint8_t>& buffer)
    {
        if (!mz_zip_v)
        {
            return false;
        }
        mz_zip_scope_password scope_password(mz_zip_v, password_, password);
        if (MZ_OK != mz_zip_reader_locate_entry(mz_zip_v, name.data(), false))
        {
            return false;
        }
        mz_zip_file* mz_zip_file_v = nullptr;
        if (MZ_OK != mz_zip_reader_entry_get_info(mz_zip_v, &mz_zip_file_v))
        {
            return false;
        }
        if (MZ_ZIP_FLAG_ENCRYPTED != (mz_zip_file_v->flag & MZ_ZIP_FLAG_ENCRYPTED))
        {
            return false;
        }
        int32_t script_size = mz_zip_reader_entry_save_buffer_length(mz_zip_v);
        if (script_size < 0)
        {
            return false;
        }
        try
        {
            buffer.resize(script_size);
        }
        catch (...)
        {
            return false;
        }
        return MZ_OK == mz_zip_reader_entry_save_buffer(mz_zip_v, buffer.data(), script_size);
    }
    
    FileArchive::FileArchive(std::string_view const& path) : name_(path), uuid(g_uuid++)
    {
        if (mz_zip_reader_create(&mz_zip_v))
        {
            if (MZ_OK == mz_zip_reader_open_file(mz_zip_v, path.data()))
            {
                std::ignore = nullptr;
            }
        }
    }
    FileArchive::~FileArchive()
    {
        if (mz_zip_v)
        {
            mz_zip_reader_close(mz_zip_v);
            mz_zip_reader_delete(&mz_zip_v);
        }
    }
    
    // FileManager

    void FileManager::refresh()
    {
        list.clear();
        std::error_code ec;
        for (auto& entry : std::filesystem::recursive_directory_iterator(L".", ec))
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
    FileType FileManager::getType(std::string_view const& name)
    {
        std::error_code ec;
        std::wstring name_str(utility::encoding::to_wide(name));
        if (std::filesystem::is_regular_file(name_str, ec))
        {
            return FileType::File;
        }
        else if (std::filesystem::is_directory(name_str, ec))
        {
            return FileType::Directory;
        }
        else
        {
            return FileType::Unknown;
        }
    }
    std::string_view FileManager::getName(size_t index) { return list[index].name; }
    bool FileManager::contain(std::string_view const& name)
    {
        std::error_code ec;
        return std::filesystem::is_regular_file(utility::encoding::to_wide(name), ec);
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
        if (!(size >= 0 && size <= INTPTR_MAX))
        {
            spdlog::error("[luastg] [LuaSTG::Core::FileManager::load] 无法加载文件 '{}'，大小超过 '{}' 字节", name, INTPTR_MAX);
            assert(false);
            return false;
        }
        buffer.resize((size_t)size);
        file.read((char*)buffer.data(), size);
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
