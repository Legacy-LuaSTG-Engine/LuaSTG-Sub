#pragma once
#include <cstdint>
#include "slow/Ref.hpp"
#include "slow/Data.hpp"

namespace slow {
    // file type, "file" or "directory"
    enum class FileType : uint32_t {
        File = 0x0000,
        Directory = 0x0001,
        Unknown = 0xFFFF,
    };
    
    // contain a list of "file" and "directory" path
    struct FileList : public Ref {
        virtual uint32_t getSize() = 0;
        virtual const char* getName(uint32_t index) = 0;
        virtual const wchar_t* getWideName(uint32_t index) = 0;
        virtual FileType getType(uint32_t index) = 0;
    };
    
    // determine how to search a file in archive when load file
    enum class FileArchiveMode : uint32_t {
        // the files in archive will regard as if they are in the root of file system
        Root = 0x0000,
        // the files in archive will plus the relative path of archive
        Relative = 0x0001,
        // the files in archive will plus the relative path of archive and the archive name (like a folder)
        Associate = 0x0002,
        // ???
        Invalid = 0xFFFF,
    };
    
    // basic file loader
    struct FileLoader : public Ref {
        // list files in a directory, return a file list
        // the file list will include "file" and "directory" type
        virtual bool listFile(const char* path, FileList** output) = 0;
        virtual bool listFile(const wchar_t* path, FileList** output) = 0;
        // check whether a file of "file" type exist
        virtual bool isFileExist(const char* path) = 0;
        virtual bool isFileExist(const wchar_t* path) = 0;
        // get the file type
        virtual FileType getFileType(const char* path) = 0;
        virtual FileType getFileType(const wchar_t* path) = 0;
        // load a file of "file" type
        virtual bool loadFile(const char* path, Data** output) = 0;
        virtual bool loadFile(const wchar_t* path, Data** output) = 0;
    };
    
    // contain a set of files
    struct FileArchive : public FileLoader {
        // load a file of "file" type, search from search path list
        // if required encrypted and the file is not encrypted, will failed
        virtual bool loadEncryptedFile(const char* path, const char* password, bool encrypted, Data** output) = 0;
        virtual bool loadEncryptedFile(const wchar_t* path, const char* password, bool encrypted, Data** output) = 0;
        
        // open or close a archive
        virtual bool open(const char* path) = 0;
        virtual bool open(const wchar_t* path) = 0;
        virtual bool open(uint8_t* data, uint32_t size) = 0;
        virtual bool open(Data* data) = 0;
        virtual void close() = 0;
        
        // set archive path (only when open from memory)
        virtual bool setPath(const char* path) = 0;
        virtual bool setPath(const wchar_t* path) = 0;
        // get archive path
        virtual const char* getPath() = 0;
        virtual const wchar_t* getWidePath() = 0;
        
        // set search mode
        virtual void setMode(FileArchiveMode mode) = 0;
        virtual FileArchiveMode getMode() = 0;
        
        // manage default password
        virtual bool setDefaultPassword(const char* password) = 0;
        virtual const char* getDefaultPassword() = 0;
        
        static bool create(FileArchive** output);
        static FileArchive* create();
    };
    
    // file manager, load data from file system
    struct FileManager : public FileLoader {
        // load a archive and return a id (0 if failed)
        virtual uint32_t loadArchive(const char* path, int32_t priority) = 0;
        virtual uint32_t loadArchive(const wchar_t* path, int32_t priority) = 0;
        // unload a archive (or unload all if id is 0)
        virtual void unloadArchive(uint32_t id) = 0;
        // change archive priority, default to 0
        // lager than 0 means find before FileManager, smaller than 0 means find after FileManager
        virtual bool setArchivePriority(uint32_t id, int32_t priority) = 0;
        virtual bool getArchivePriority(uint32_t id, int32_t* priority) = 0;
        // get archive
        virtual bool isArchiveExist(uint32_t id) = 0;
        virtual bool getArchive(uint32_t id, FileArchive** output) = 0;
        
        // set search path list
        virtual void setSearchPath(const char** pathlist, uint32_t count) = 0;
        virtual void setSearchPath(const wchar_t** pathlist, uint32_t count) = 0;
        // add search path
        virtual void addSearchPath(const char* path) = 0;
        virtual void addSearchPath(const wchar_t* path) = 0;
        // remove search path
        virtual void removeSearchPath(const char* path) = 0;
        virtual void removeSearchPath(const wchar_t* path) = 0;
        
        // check whether a file of "file" type exist, search from search path, include archive
        virtual bool isFileExistEx(const char* path) = 0;
        virtual bool isFileExistEx(const wchar_t* path) = 0;
        // load a file of "file" type, search from search path list and archive
        virtual bool loadFileEx(const char* path, Data** output) = 0;
        virtual bool loadFileEx(const wchar_t* path, Data** output) = 0;
        
        static bool create(FileManager** output);
        static FileManager* create();
    };
};
