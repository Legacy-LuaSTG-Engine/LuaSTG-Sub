#include "slow/Data.hpp"
#include "slow/FileManager.hpp"
#include "RefImpl.hpp"
#include "EncodeConvert.hpp"
#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <iostream>
#include "zip.h"

namespace slow {
    namespace fs = std::filesystem;
    
    struct FileNode {
        std::string name;
        std::wstring wname;
        FileType type;
    };
    
    class FileListImpl : public RefImpl<FileList> {
    private:
        std::vector<FileNode> _list;
    public:
        uint32_t getSize() {
            return (uint32_t)_list.size();
        }
        const char* getName(uint32_t index) {
            if (index < getSize()) {
                return _list[index].name.c_str();
            }
            return nullptr;
        }
        const wchar_t* getWideName(uint32_t index) {
            if (index < getSize()) {
                return _list[index].wname.c_str();
            }
            return nullptr;
        }
        FileType getType(uint32_t index) {
            if (index < getSize()) {
                return _list[index].type;
            }
            return FileType::Unknown;
        }
    public:
        void insert(const std::string& str, FileType type) {
            FileNode node;
            node.name = str;
            utf8_utf16(str, node.wname);
            node.type = type;
            _list.push_back(node);
        }
        void insert(const std::wstring& str, FileType type) {
            FileNode node;
            utf16_utf8(str, node.name);
            node.wname = str;
            node.type = type;
            _list.push_back(node);
        }
    public:
        FileListImpl() {}
        virtual ~FileListImpl() {}
    };
    
    class FileArchiveImpl : public RefImpl<FileArchive> {
    private:
        FileArchiveMode _mode;
        zip_t* _zip;
        zip_source_t* _source;
        bool _is_file;
        std::string _password;
        std::string _path;
        std::wstring _wpath;
        std::string _path2;
        std::wstring _wpath2;
        std::string _path3;
        std::wstring _wpath3;
        Pointer<Data> _data;
    private:
        bool _impl_open(const char* path1, const wchar_t* path2) {
            // convert encode
            if (path1 != nullptr && path2 != nullptr) {
                _path = path1;
                _wpath = path2;
            }
            else if (path1 != nullptr && path2 == nullptr) {
                _path = path1;
                if (!utf8_utf16(_path, _wpath)) {
                    return false;
                }
            }
            else if (path1 == nullptr && path2 != nullptr) {
                _wpath = path2;
                if (!utf16_utf8(_wpath, _path)) {
                    return false;
                }
            }
            else {
                return false;
            }
            // open
            zip_error_t err;
            _source = zip_source_win32w_create(_wpath.c_str(), 0, -1, &err);
            if (_source != nullptr) {
                _zip = zip_open_from_source(_source, ZIP_CHECKCONS | ZIP_RDONLY, &err);
                if (_zip != nullptr) {
                    _is_file = true; // I should mark it first
                    return _impl_updatepath();
                }
            }
            return false;
        }
        bool _impl_updatepath() {
            if (_wpath.length() == 0) {
                _path2.clear();
                _wpath2.clear();
                _path3.clear();
                _wpath3.clear();
                return true;
            }
            try {
                _path2 = fs::path(_path).remove_filename().generic_string();
                _wpath2 = fs::path(_wpath).remove_filename().generic_wstring();
                _path3 = fs::path(_path).replace_extension().generic_string() + "/";
                _wpath3 = fs::path(_wpath).replace_extension().generic_wstring() + L"/";
                return true;
            }
            catch (...) {
                _path2.clear();
                _wpath2.clear();
                _path3.clear();
                _wpath3.clear();
            }
            return false;
        }
    public:
        bool open(const char* path) {
            close();
            return _impl_open(path, nullptr);
        }
        bool open(const wchar_t* path) {
            close();
            return _impl_open(nullptr, path);
        }
        bool open(uint8_t* data, uint32_t size) {
            Pointer<Data> buffer;
            if (Data::create(data, size, &buffer)) {
                bool result = open(*buffer);
                return result;
            }
            return false;
        }
        bool open(Data* data) {
            close();
            if (data != nullptr) {
                _data = data; // add ref
                zip_error_t err;
                _source = zip_source_buffer_create(_data->data(), _data->size(), 0, &err); // do not auto free
                if (_source != nullptr) {
                    _zip = zip_open_from_source(_source, ZIP_CHECKCONS | ZIP_RDONLY, &err);
                    if (_zip != nullptr) {
                        _is_file = false;
                        return true;
                    }
                }
            }
            return false;
        }
        void close() {
            if (_zip != nullptr) {
                zip_discard(_zip);
                _zip = nullptr;
            }
            if (_source != nullptr) {
                zip_source_close(_source);
                _source = nullptr;
            }
            _mode = FileArchiveMode::Root;
            _is_file = false;
            _password.clear();
            _path.clear();
            _wpath.clear();
            _path2.clear();
            _wpath2.clear();
            _path3.clear();
            _wpath3.clear();
            _data.reset();
        }
        
        bool setPath(const char* path) {
            if (!_is_file) {
                _path = path;
                if (utf8_utf16(_path, _wpath)) {
                    return _impl_updatepath();
                }
            }
            return false;
        }
        bool setPath(const wchar_t* path) {
            if (!_is_file) {
                _wpath = path;
                if (utf16_utf8(_wpath, _path)) {
                    return _impl_updatepath();
                }
            }
            return false;
        }
        const char* getPath() {
            return _path.c_str();
        }
        const wchar_t* getWidePath() {
            return _wpath.c_str();
        }
        
        void setMode(FileArchiveMode mode) {
            _mode = mode;
        }
        FileArchiveMode getMode() {
            return _mode;
        }
        
        bool setDefaultPassword(const char* password) {
            if (_zip != nullptr) {
                _password.clear();
                int ret = zip_set_default_password(_zip, password);
                if (ret == 0) {
                    // yes, I can pass a nullptr param to cancel default password, so I dont't need to storage it
                    if (password != nullptr) {
                        _password = password;
                    }
                    return true;
                }
            }
            return false;
        }
        const char* getDefaultPassword() {
            return _password.c_str();
        }
        
        bool listFile(const char* path, FileList** output) {
            if (path != nullptr && output!= nullptr && _zip != nullptr) {
                try {
                    FileListImpl* flist = new FileListImpl;
                    Pointer<FileList> _flist;
                    _flist.rawset(flist);
                    
                    zip_int64_t count = zip_get_num_entries(_zip, ZIP_FL_UNCHANGED);
                    if (count >= 0) {
                        std::string_view pathpp = path;
                        const auto path_len = pathpp.length();
                        std::string buffer;
                        for (zip_int64_t idx = 0; idx < count; idx++) {
                            const char* name = zip_get_name(_zip, idx, ZIP_FL_ENC_GUESS);
                            if (name != nullptr) {
                                std::string_view namepp = name;
                                const auto name_len = namepp.length();
                                const auto name_len_1 = name_len - 1;
                                if (name_len == 0 || name_len <= path_len) {
                                    continue; // do not in target dir
                                }
                                if (namepp.substr(0, path_len) != pathpp) {
                                    continue; // do not match
                                }
                                
                                bool next_loop = false;
                                for (size_t idx = path_len; idx < name_len; idx++) {
                                    if (namepp[idx] == '/' && idx < name_len_1) {
                                        next_loop = true; // in sub dir
                                        break;
                                    }
                                }
                                if (next_loop) {
                                    continue;
                                }
                                switch (_mode) {
                                case FileArchiveMode::Associate:
                                    buffer = _path3 + name;
                                    break;
                                case FileArchiveMode::Relative:
                                    buffer = _path2 + name;
                                    break;
                                case FileArchiveMode::Root:
                                default:
                                    buffer = name;
                                    break;
                                }
                                if (namepp[name_len_1] == '/') {
                                    flist->insert(buffer, FileType::Directory);
                                }
                                else {
                                    flist->insert(buffer, FileType::File);
                                }
                            }
                        }
                    }
                    
                    flist->retain(); // keep
                    *output = (FileList*)flist;
                    return true;
                }
                catch (...) {}
            }
            return false;
        }
        bool listFile(const wchar_t* path, FileList** output) {
            std::string buffer;
            if (utf16_utf8(path, buffer)) {
                return listFile(buffer.c_str(), output);
            }
            return false;
        }
        bool isFileExist(const char* path) {
            return getFileType(path) == FileType::File;
        }
        bool isFileExist(const wchar_t* path) {
            std::string buffer;
            if (utf16_utf8(path, buffer)) {
                return isFileExist(buffer.c_str());
            }
            return false;
        }
        FileType getFileType(const char* path) {
            if (path != nullptr && _zip != nullptr) {
                std::string_view buffer = path;
                switch (_mode) {
                    case FileArchiveMode::Associate: {
                        if (buffer.length() > _path3.length()) {
                            buffer = buffer.substr(_path3.length());
                        }
                        else {
                            return FileType::Unknown; // do not match
                        }
                        break;
                    }
                    case FileArchiveMode::Relative: {
                        if (buffer.length() > _path2.length()) {
                            buffer = buffer.substr(_path2.length());
                        }
                        else {
                            return FileType::Unknown; // do not match
                        }
                        break;
                    }
                    case FileArchiveMode::Root:
                    default: {
                        if (buffer.empty()) {
                            return FileType::Unknown; // empty path ???
                        }
                        break;
                    }
                }
                zip_int64_t index = zip_name_locate(_zip, &buffer.front(), ZIP_FL_ENC_GUESS);
                if (index >= 0) {
                    if (buffer.back() == '/') {
                        return FileType::Directory; // this is a directory!
                    }
                    else {
                        return FileType::File;
                    }
                }
            }
            return FileType::Unknown;
        }
        FileType getFileType(const wchar_t* path) {
            std::string buffer;
            if (utf16_utf8(path, buffer)) {
                return getFileType(buffer.c_str());
            }
            return FileType::Unknown;
        }
        bool loadFile(const char* path, Data** output) {
            if (path != nullptr && output != nullptr && _zip != nullptr) {
                try {
                    std::string_view buffer = path;
                    switch (_mode) {
                        case FileArchiveMode::Associate: {
                            if (buffer.length() > _path3.length()) {
                                buffer = buffer.substr(_path3.length());
                            }
                            else {
                                return false; // do not match
                            }
                            break;
                        }
                        case FileArchiveMode::Relative: {
                            if (buffer.length() > _path2.length()) {
                                buffer = buffer.substr(_path2.length());
                            }
                            else {
                                return false; // do not match
                            }
                            break;
                        }
                        case FileArchiveMode::Root:
                        default: {
                            if (buffer.empty()) {
                                return false; // empty path ???
                            }
                            break;
                        }
                    }
                    zip_int64_t index = zip_name_locate(_zip, &buffer.front(), ZIP_FL_ENC_GUESS);
                    if (index >= 0) {
                        // find!
                        zip_stat_t stat;
                        if (0 == zip_stat_index(_zip, index, ZIP_STAT_SIZE, &stat)) {
                            // create Data
                            Pointer<Data> _pdata;
                            const uint32_t _size = (uint32_t)(stat.size & 0x7FFFFFFF);
                            if (Data::create(_size, false, &_pdata)) {
                                // open file in archive
                                zip_file_t* zfile = zip_fopen_index(_zip, index, ZIP_FL_UNCHANGED);
                                if (zfile != nullptr) {
                                    // read data
                                    zip_fread(zfile, _pdata->data(), _size);
                                    zip_fclose(zfile);
                                    _pdata->retain(); // keep
                                    *output = *_pdata;
                                    return true;
                                }
                            }
                        }
                    }
                }
                catch (...) {}
            }
            return false;
        }
        bool loadFile(const wchar_t* path, Data** output) {
            std::string buffer;
            if (utf16_utf8(path, buffer)) {
                return loadFile(buffer.c_str(), output);
            }
            return false;
        }
        bool loadEncryptedFile(const char* path, const char* password, bool encrypted, Data** output) {
            if (path != nullptr && output != nullptr && _zip != nullptr) {
                try {
                    std::string_view buffer = path;
                    switch (_mode) {
                        case FileArchiveMode::Associate: {
                            if (buffer.length() > _path3.length()) {
                                buffer = buffer.substr(_path3.length());
                            }
                            else {
                                return false; // do not match
                            }
                            break;
                        }
                        case FileArchiveMode::Relative: {
                            if (buffer.length() > _path2.length()) {
                                buffer = buffer.substr(_path2.length());
                            }
                            else {
                                return false; // do not match
                            }
                            break;
                        }
                        case FileArchiveMode::Root:
                        default: {
                            if (buffer.empty()) {
                                return false; // empty path ???
                            }
                            break;
                        }
                    }
                    zip_int64_t index = zip_name_locate(_zip, &buffer.front(), ZIP_FL_ENC_GUESS);
                    if (index >= 0) {
                        // find!
                        zip_stat_t stat;
                        if (0 == zip_stat_index(_zip, index, ZIP_STAT_SIZE | ZIP_STAT_ENCRYPTION_METHOD, &stat)) {
                            // check if is encrypted
                            if (!encrypted || (encrypted && stat.encryption_method != ZIP_EM_NONE)) {
                                // create Data
                                Pointer<Data> _pdata;
                                const uint32_t _size = (uint32_t)(stat.size & 0x7FFFFFFF);
                                if (Data::create(_size, false, &_pdata)) {
                                    // open file in archive
                                    zip_file_t* zfile = zip_fopen_index_encrypted(_zip, index, ZIP_FL_UNCHANGED, password);
                                    if (zfile != nullptr) {
                                        // read data
                                        zip_fread(zfile, _pdata->data(), _size);
                                        zip_fclose(zfile);
                                        _pdata->retain(); // keep
                                        *output = *_pdata;
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
                catch (...) {}
            }
            return false;
        }
        bool loadEncryptedFile(const wchar_t* path, const char* password, bool encrypted, Data** output) {
            std::string buffer;
            if (utf16_utf8(path, buffer)) {
                return loadEncryptedFile(buffer.c_str(), password, encrypted, output);
            }
            return false;
        }
    public:
        FileArchiveImpl() : _mode(FileArchiveMode::Invalid), _zip(nullptr), _source(nullptr), _is_file(false) {}
        virtual ~FileArchiveImpl() { close(); }
    };
    
    bool FileArchive::create(FileArchive** output) {
        assert(output != nullptr);
        try {
            if (output != nullptr) {
                FileArchiveImpl* p = new FileArchiveImpl;
                *output = (FileArchive*)p;
                return true;
            }
        }
        catch(...) {}
        return false;
    }
    FileArchive* FileArchive::create() {
        FileArchive* p = nullptr;
        FileArchive::create(&p);
        return p;
    }
    
    struct FileArchiveNode {
        Pointer<FileArchive> archive;
        uint32_t id;
        int32_t priority;
    };
    struct FileArchiveNodeCmp {
        bool operator()(const FileArchiveNode* lhs, const FileArchiveNode* rhs) const {
            if (lhs->priority != rhs->priority) {
                return lhs->priority > rhs->priority;
            }
            else {
                return lhs->id > rhs->id;
            }
        }
    };
    
    class FileManagerImpl : public RefImpl<FileManager> {
    private:
        std::vector<std::string> _searchpath;
        std::vector<std::wstring> _wsearchpath;
        uint32_t _archive_id;
        std::vector<FileArchiveNode> _archive;
        std::set<FileArchiveNode*, FileArchiveNodeCmp> _archive_list;
    private:
        bool _impl_loadFileEx_system(const wchar_t* path, Data** output) {
            // load directly
            if (isFileExist(path)) {
                if (Data::create(path, output)) {
                    return true;
                }
            }
            // load by search path
            for (auto& sp : _wsearchpath) {
                auto p = sp + path;
                if (isFileExist(p.c_str())) {
                    if (Data::create(p.c_str(), output)) {
                        return true;
                    }
                }
            }
            return false;
        }
        template<typename P>
        bool _impl_loadArchive(const P* path, int32_t priority) {
            FileArchiveNode node { nullptr, 0, 0 };
            if (FileArchive::create(&node.archive)) {
                if (node.archive->open(path)) {
                    node.id = _archive_id;
                    _archive_id++;
                    node.priority = priority;
                    _archive.push_back(node);
                    _archive_list.insert(&_archive.back()); // DO NOT INSERT "node"
                }
            }
            return node.id;
        }
    public:
        bool listFile(const char* path, FileList** output) {
            std::wstring u16path;
            if (utf8_utf16(path, u16path)) {
                return listFile(u16path.c_str(), output);
            }
            return false;
        }
        bool listFile(const wchar_t* path, FileList** output) {
            try {
                FileListImpl* list = new FileListImpl;
                std::wstring _path = path;
                if (_path.length() == 0 || _path == L".") {
                    if (fs::is_directory(L".")) {
                        for (auto& p : fs::directory_iterator(L".")) {
                            switch (p.status().type()) {
                                case fs::file_type::regular: {
                                    auto str = p.path().generic_wstring();
                                    str = str.substr(2, str.length() - 2);
                                    list->insert(str, FileType::File);
                                    break;
                                }
                                case fs::file_type::directory: {
                                    auto str = p.path().generic_wstring();
                                    str = str.substr(2, str.length() - 2);
                                    list->insert(str + L"/", FileType::Directory);
                                    break;
                                }
                            }
                        }
                    }
                }
                else {
                    if (fs::is_directory(_path)) {
                        for (auto& p : fs::directory_iterator(_path)) {
                            switch (p.status().type()) {
                                case fs::file_type::regular: {
                                    list->insert(p.path().generic_wstring(), FileType::File);
                                    break;
                                }
                                case fs::file_type::directory: {
                                    list->insert(p.path().generic_wstring() + L"/", FileType::Directory);
                                    break;
                                }
                            }
                        }
                    }
                }
                *output = (FileList*)list;
                return true;
            }
            catch(...) {}
            return false;
        }
        bool isFileExist(const char* path) {
            return getFileType(path) == FileType::File;
        }
        bool isFileExist(const wchar_t* path) {
            return getFileType(path) == FileType::File;
        }
        FileType getFileType(const char* path) {
            std::wstring u16path;
            if (utf8_utf16(path, u16path)) {
                return getFileType(u16path.c_str());
            }
            return FileType::Unknown;
        }
        FileType getFileType(const wchar_t* path) {
            auto status = fs::status(path);
            switch(status.type()) {
            case fs::file_type::regular:
                return FileType::File;
            case fs::file_type::directory:
                return FileType::Directory;
            default:
                return FileType::Unknown;
            }
        }
        bool loadFile(const char* path, Data** output) {
            return Data::create(path, output);
        }
        bool loadFile(const wchar_t* path, Data** output) {
            return Data::create(path, output);
        }
        
        uint32_t loadArchive(const char* path, int32_t priority) {
            return _impl_loadArchive(path, priority);
        }
        uint32_t loadArchive(const wchar_t* path, int32_t priority) {
            return _impl_loadArchive(path, priority);
        }
        void unloadArchive(uint32_t id) {
            if (id != 0) {
                for (auto it = _archive_list.begin(); it != _archive_list.end();) {
                    if ((*it)->id == id) {
                        it = _archive_list.erase(it);
                    }
                }
                for (auto it = _archive.begin(); it != _archive.end();) {
                    if (it->id == id) {
                        it = _archive.erase(it);
                    }
                }
            }
            else {
                _archive_list.clear();
                _archive.clear();
            }
        }
        bool setArchivePriority(uint32_t id, int32_t priority) {
            if (id == 0) {
                return false;
            }
            // remove
            for (auto it = _archive_list.begin(); it != _archive_list.end();) {
                if ((*it)->id == id) {
                    it = _archive_list.erase(it);
                }
            }
            // get
            FileArchiveNode* node = nullptr;
            for (auto& i : _archive) {
                if (i.id == id) {
                    node = &i;
                    break;
                }
            }
            // change
            if (node != nullptr) {
                node->priority = priority;
                _archive_list.insert(node);
                return true;
            }
            return false;
        }
        bool getArchivePriority(uint32_t id, int32_t* priority) {
            assert(priority != nullptr);
            if (id == 0 || priority == nullptr) {
                return false;
            }
            for (auto& i : _archive) {
                if (i.id == id) {
                    *priority = i.priority;
                    return true;
                }
            }
            return false;
        }
        bool isArchiveExist(uint32_t id) {
            if (id == 0) {
                return false;
            }
            for (auto& i : _archive) {
                if (i.id == id) {
                    return true;
                }
            }
            return false;
        }
        bool getArchive(uint32_t id, FileArchive** output) {
            assert(output != nullptr);
            if (id == 0 || output == nullptr) {
                return false;
            }
            for (auto& i : _archive) {
                if (i.id == id) {
                    i.archive->retain(); // add ref
                    *output = *(i.archive);
                    return true;
                }
            }
            return false;
        }
        
        void setSearchPath(const char** pathlist, uint32_t count) {
            _searchpath.clear();
            _wsearchpath.clear();
            if (pathlist != nullptr) {
                for (uint32_t i = 0; i < count; i++) {
                    addSearchPath(pathlist[i]);
                }
            }
        }
        void setSearchPath(const wchar_t** pathlist, uint32_t count) {
            _searchpath.clear();
            _wsearchpath.clear();
            if (pathlist != nullptr) {
                for (uint32_t i = 0; i < count; i++) {
                    addSearchPath(pathlist[i]);
                }
            }
        }
        void addSearchPath(const char* path) {
            assert(path != nullptr);
            if (path != nullptr && std::string_view(path).length() > 0) {
                std::wstring buffer;
                if (utf8_utf16(path, buffer)) {
                    addSearchPath(buffer.c_str());
                }
            }
        }
        void addSearchPath(const wchar_t* path) {
            assert(path != nullptr);
            if (path != nullptr && std::wstring_view(path).length() > 0) {
                std::wstring_view vpath = path;
                if (vpath.length() == 1 && vpath.back() == L'/') {
                    return; // shit, what are you doing
                }
                _wsearchpath.push_back(path);
                auto& sp = _wsearchpath.back();
                if (sp.back() != L'/') {
                    sp.push_back(L'/');
                }
                // sync to u8
                std::string u8path;
                if (utf16_utf8(sp, u8path)) {
                    _searchpath.push_back(u8path);
                }
            }
        }
        void removeSearchPath(const char* path) {
            assert(path != nullptr);
            if (path != nullptr && std::string_view(path).length() > 0) {
                std::wstring buffer;
                if (utf8_utf16(path, buffer)) {
                    removeSearchPath(buffer.c_str());
                }
            }
        }
        void removeSearchPath(const wchar_t* path) {
            assert(path != nullptr);
            if (path != nullptr) {
                std::wstring_view pathv = path;
                if (pathv.length() > 0) {
                    for (auto it = _wsearchpath.begin(); it != _wsearchpath.end();) {
                        // equal
                        const auto cond1 = *it == pathv;
                        // only different on the last separator
                        const auto cond2 = (*it).back() == L'/';
                        const auto cond3 = (pathv.length() + 1) == (*it).length();
                        if (cond1 || (cond2 && cond3)) {
                            it = _wsearchpath.erase(it);
                        }
                        else {
                            it++;
                        }
                    }
                    std::string u8path;
                    if (utf16_utf8(path, u8path)) {
                        for (auto it = _searchpath.begin(); it != _searchpath.end();) {
                            // equal
                            const auto cond1 = (*it) == u8path;
                            // only different on the last separator
                            const auto cond2 = (*it).back() == '/';
                            const auto cond3 = (u8path.length() + 1) == (*it).length();
                            if (cond1 || (cond2 && cond3)) {
                                it = _searchpath.erase(it);
                            }
                            else {
                                it++;
                            }
                        }
                    }
                }
            }
        }
        
        bool isFileExistEx(const char* path) {
            std::wstring u16path;
            if (utf8_utf16(path, u16path)) {
                return isFileExistEx(u16path.c_str());
            }
            return false;
        }
        bool isFileExistEx(const wchar_t* path) {
            // find directly
            if (isFileExist(path)) {
                return true;
            }
            // find from search path
            for (auto& sp : _wsearchpath) {
                if (isFileExist((sp + path).c_str())) {
                    return true;
                }
            }
            // find in archive
            std::string buffer;
            if (utf16_utf8(path, buffer)) {
                for (auto& node : _archive) {
                    // find directly
                    if (node.archive->isFileExist(buffer.c_str())) {
                        return true;
                    }
                    // find from search path
                    for (auto& sp : _searchpath) {
                        if (node.archive->isFileExist((sp + buffer).c_str())) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        bool loadFileEx(const char* path, Data** output) {
            std::wstring u16path;
            if (utf8_utf16(path, u16path)) {
                return loadFileEx(u16path.c_str(), output);
            }
            return false;
        }
        bool loadFileEx(const wchar_t* path, Data** output) {
            bool self_load = false;
            // have more than one archive, mixed load
            if (_archive.size() > 0) {
                std::string u8path;
                if (!utf16_utf8(path, u8path)) {
                    return false;
                }
                for (auto& node : _archive_list) {
                    // load in file system
                    if (node->priority < 0 && !self_load) {
                        if (_impl_loadFileEx_system(path, output)) {
                            return true;
                        }
                        self_load = true;
                    }
                    // load in archive
                    {
                        // load directly
                        if (node->archive->isFileExist(u8path.c_str())) {
                            if (node->archive->loadFile(u8path.c_str(), output)) {
                                return true;
                            }
                        }
                        // load by search path
                        for (auto& sp : _searchpath) {
                            auto p = sp + u8path;
                            if (node->archive->isFileExist(p.c_str())) {
                                if (node->archive->loadFile(p.c_str(), output)) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            // so now we should load from file system
            if (!self_load) {
                if (_impl_loadFileEx_system(path, output)) {
                    return true;
                }
            }
            return false;
        }
    public:
        FileManagerImpl() : _archive_id(1) {}
        virtual ~FileManagerImpl() {}
    };
    
    bool FileManager::create(FileManager** output) {
        assert(output != nullptr);
        try {
            if (output != nullptr) {
                FileManagerImpl* p = new FileManagerImpl;
                *output = (FileManager*)p;
                return true;
            }
        }
        catch(...) {}
        return false;
    }
    FileManager* FileManager::create() {
        FileManager* p = nullptr;
        FileManager::create(&p);
        return p;
    }
};
