#include "lua_xlsx_csv.h"

#include <iostream>
#include <fstream>
#include <clocale>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "zip.h"
#include "pugixml.hpp"
#include "lua.hpp"

namespace slow
{
    inline bool readfile(std::string path, std::vector<uint8_t>& buffer) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (file.is_open()) {
            auto p2 = file.seekg(0, std::ios::end).tellg();
            auto p1 = file.seekg(0, std::ios::beg).tellg();
            auto dp = p2 - p1;
            buffer.resize(dp);
            file.read((char*)buffer.data(), dp);
            return true;
        }
        return false;
    }
    inline bool readfile(std::wstring path, std::vector<uint8_t>& buffer) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (file.is_open()) {
            auto p2 = file.seekg(0, std::ios::end).tellg();
            auto p1 = file.seekg(0, std::ios::beg).tellg();
            auto dp = p2 - p1;
            buffer.resize(dp);
            file.read((char*)buffer.data(), dp);
            return true;
        }
        return false;
    }
    
    class zip {
    private:
        std::vector<uint8_t> _buffer;
        zip_source* _source;
        zip_t* _archive;
    private:
        bool _open() {
            zip_error_t ziperr;
            _source = zip_source_buffer_create(_buffer.data(), _buffer.size(), 0, &ziperr);
            if (nullptr != _source) {
                _archive = zip_open_from_source(_source, ZIP_CHECKCONS | ZIP_RDONLY, &ziperr);
                if (nullptr != _archive) {
                    return true;
                }
            }
            return false;
        }
    public:
        bool open(const char* path) {
            close();
            if (readfile(path, _buffer)) {
                return _open();
            }
            else {
                std::printf("[slow::zip::open] read file '%s' failed\n", path);
            }
            return false;
        }
        bool open(const wchar_t* path) {
            close();
            if (readfile(path, _buffer)) {
                return _open();
            }
            else {
                std::wprintf(L"[slow::zip::open] read file '%s' failed\n", path);
            }
            return false;
        }
        bool open(const uint8_t* buffer, const uint32_t size) {
            close();
            _buffer.resize(size);
            std::memcpy(_buffer.data(), buffer, size);
            return _open();
        }
        void close() {
            _buffer.clear();
            if (nullptr != _archive) zip_discard(_archive);
            if (nullptr != _source) zip_source_close(_source);
        }
        bool fexist(const char* path) {
            zip_int64_t i = zip_name_locate(_archive, path, ZIP_FL_ENC_GUESS);
            return (i >= 0);
        }
        uint32_t fsize(const char* path) {
            zip_int64_t i = zip_name_locate(_archive, path, ZIP_FL_ENC_GUESS);
            if (i >= 0) {
                zip_stat_t stat;
                int flag = ZIP_STAT_NAME | ZIP_STAT_INDEX | ZIP_STAT_SIZE | ZIP_STAT_ENCRYPTION_METHOD;
                int b = zip_stat_index(_archive, i, flag, &stat);
                if (b == 0) {
                    return (uint32_t)(stat.size & 0x000000007FFFFFFF); // only support 32bit size
                }
            }
            return 0;
        }
        bool fread(const char* path, uint8_t* buffer, uint32_t size) {
            zip_int64_t i = zip_name_locate(_archive, path, ZIP_FL_ENC_GUESS);
            if (i >= 0) {
                zip_file_t* _file = zip_fopen_index(_archive, i, ZIP_FL_UNCHANGED);
                if (nullptr != _file) {
                    zip_int64_t r = zip_fread(_file, buffer, size);
                    zip_fclose(_file);
                    return (r >= 0) && (r == (zip_int64_t)size);
                }
            }
            return false;
        }
    public:
        zip() : _source(nullptr), _archive(nullptr) {}
        ~zip() { close(); }
    };
    
    bool zip_fread(zip& c, const std::string& path, std::vector<uint8_t>& buffer) {
        if (c.fexist(path.c_str())) {
            uint32_t size = c.fsize(path.c_str());
            if (size > 0) {
                buffer.resize(size);
                if (c.fread(path.c_str(), buffer.data(), size)) {
                    return true;
                }
                else {
                    std::printf("read '%s' failed\n", path.c_str());
                }
            }
            else {
                std::printf("the size of '%s' is zero\n", path.c_str());
            }
        }
        else {
            std::printf("find '%s' failed\n", path.c_str());
        }
        return false;
    }
};

int cell_coord_to_uint32(const std::string& str) {
    int ret = 0;
    int base = 1;
    for (int idx = (str.length() - 1); idx >= 0; idx--) {
        if (str[idx] >= 'A' && str[idx] <= 'Z') {
            ret += base * (int)(str[idx] - '\x40');
            base *= 26;
        }
        else if (str[idx] >= 'a' && str[idx] <= 'z') {
            ret += base * (int)(str[idx] - '\x60');
            base *= 26;
        }
    }
    return ret;
}

namespace xlsx
{
    using row = std::vector<std::string>;
    using worksheet = std::vector<row>;
    using workbook = std::unordered_map<std::string, worksheet>;
    
    bool read(const uint8_t* data, const uint32_t size, workbook& wb) {
        // try open xlsx (zip)
        slow::zip zip;
        if (!zip.open(data, size))
        {
            return false;
        }
        
        // common buffer
        std::vector<uint8_t> buffer;
        
        // read cell string resource
        std::vector<std::string> static_string;
        if (slow::zip_fread(zip, "xl/sharedStrings.xml", buffer))
        {
            pugi::xml_document doc;
            auto result = doc.load_buffer(buffer.data(), buffer.size());
            if (result)
            {
                for (auto tool : doc.child("sst").children("si"))
                {
                    if (!tool.child("t").empty())
                    {
                        const auto* nodev = tool.child("t").child_value();
                        static_string.push_back(nodev);
                    }
                    else if (!tool.child("r").empty())
                    {
                        std::string str;
                        for (auto v : tool.children("r"))
                        {
                            str += v.child("t").child_value();
                        }
                        static_string.push_back(std::move(str));
                    }
                    else
                    {
                        static_string.push_back("<?>");
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        
        // worksheet list
        // worksheet attr
        const std::string_view attr_worksheet_head = "worksheets/sheet";
        // data map
        std::unordered_map<std::string, std::string> workbook_entry;
        std::unordered_map<std::string, std::string> worksheet_name;
        std::unordered_map<std::string, std::string> worksheet_path;
        // workbook entry (files)
        if (slow::zip_fread(zip, "xl/_rels/workbook.xml.rels", buffer))
        {
            pugi::xml_document xml;
            if (xml.load_buffer(buffer.data(), buffer.size()))
            {
                for (auto node : xml.child("Relationships").children("Relationship"))
                {
                    const std::string_view target = node.attribute("Target").as_string();
                    if (target.length() > attr_worksheet_head.length() && target.substr(0, attr_worksheet_head.length()) == attr_worksheet_head)
                    {
                        std::string key = node.attribute("Id").as_string();
                        std::string value = "xl/"; value += node.attribute("Target").as_string();
                        workbook_entry.emplace(std::move(key), std::move(value));
                    }
                }
            }
            else
            {
                std::printf("parser xml [xl/_rels/workbook.xml.rels] failed");
                return false;
            }
        }
        else
        {
            std::printf("read file [xl/_rels/workbook.xml.rels] failed");
            return false;
        }
        // worksheet name
        if (slow::zip_fread(zip, "xl/workbook.xml", buffer))
        {
            pugi::xml_document xml;
            if (xml.load_buffer(buffer.data(), buffer.size()))
            {
                for (auto node : xml.child("workbook").child("sheets").children("sheet"))
                {
                    std::string key = node.attribute("r:id").as_string();
                    std::string value = node.attribute("name").as_string();
                    worksheet_name.emplace(std::move(key), std::move(value));
                }
            }
            else
            {
                std::printf("parser xml [xl/workbook.xml] failed");
                return false;
            }
        }
        else
        {
            std::printf("read file [xl/workbook.xml] failed");
            return false;
        }
        // worksheet path
        for (auto& v : workbook_entry)
        {
            auto it = worksheet_name.find(v.first);
            if (it != worksheet_name.end())
            {
                worksheet_path.emplace(it->second, v.second);
            }
            else
            {
                worksheet_path.emplace(v);
            }
        }
        
        // read workbook
        // cell attr
        const std::string_view attr_name_y = "r";
        const std::string_view attr_name_tpye = "t";
        const std::string_view cell_type_bool = "b";
        const std::string_view cell_type_num = "n";
        const std::string_view cell_type_str = "s";
        const std::string_view bool_value_false = "0";
        const std::string_view bool_value_true = "1";
        // read all workbook
        for (auto& item : worksheet_path) {
            pugi::xml_document doc;
            if (slow::zip_fread(zip, item.second, buffer)) {
                auto result = doc.load_buffer(buffer.data(), buffer.size());
                if (result) {
                    // read worksheet
                    worksheet ws;
                    for (auto tool : doc.child("worksheet").child("sheetData").children("row")) {
                        // read row
                        row xlrow;
                        int col = 1;
                        for (auto cell : tool.children("c")) {
                            // read cell
                            
                            // check attr
                            bool is_string = false;
                            bool is_number = false;
                            bool is_boolean = false;
                            std::string cellcoord;
                            for (auto attr : cell.attributes())
                            {
                                if (attr_name_y == attr.name())
                                {
                                    cellcoord = attr.as_string();
                                }
                                else if (attr_name_tpye == attr.name())
                                {
                                    if (cell_type_str == attr.value())
                                    {
                                        is_string = true;
                                    }
                                    else if (cell_type_num == attr.value())
                                    {
                                        is_number = true;
                                    }
                                    else if (cell_type_bool == attr.value())
                                    {
                                        is_boolean = true;
                                    }
                                    else
                                    {
                                        is_string = true; // unknown type ??? regard it as string
                                    }
                                }
                                else
                                {
                                    // ignore attr
                                }
                            }
                            
                            // add empty cell
                            int colv = cell_coord_to_uint32(cellcoord);
                            while ((colv - col) > 0)
                            {
                                xlrow.emplace_back("");
                                col++;
                            }
                            
                            // get value
                            std::string cellvalue;
                            for (auto v : cell.children("v"))
                            {
                                if (is_string)
                                {
                                    char* end = nullptr;
                                    const auto uv = std::strtoul(v.child_value(), &end, 10);
                                    cellvalue = static_string[uv];
                                }
                                else if (is_number)
                                {
                                    cellvalue = v.child_value();
                                }
                                else if (is_boolean)
                                {
                                    if (bool_value_true == v.child_value())
                                    {
                                        cellvalue = "true";
                                    }
                                    else // if (bool_value_false == v.child_value())
                                    {
                                        cellvalue = "false";
                                    }
                                }
                                else
                                {
                                    cellvalue = v.child_value();
                                }
                            }
                            
                            // save cell
                            xlrow.push_back(cellvalue);
                            col++;
                        }
                        // save row
                        ws.push_back(xlrow);
                    }
                    // save worksheet
                    wb.emplace(item.first, ws);
                }
                else {
                    // failed parser
                }
            }
            else {
                // failed read file
            }
        }
        
        // make worksheet square
        for (auto& ws : wb)
        {
            size_t length = 0;
            for (auto& r : ws.second)
            {
                length = std::max(length, r.size());
            }
            for (auto& r : ws.second)
            {
                while (r.size() < length) {
                    r.emplace_back("");
                }
            }
        }
        
        // ok
        return true;
    }
    
    bool read(const std::vector<uint8_t>& data, workbook& wb)
    {
        return read(data.data(), data.size(), wb);
    }
};

void idle() {}

int main()
{
    std::setlocale(LC_ALL, "zh_CN.UTF-8");
    
    std::vector<uint8_t> buffer;
    if (slow::readfile("txwk/test.xlsx", buffer))
    {
        xlsx::workbook wb;
        if (xlsx::read(buffer, wb))
        {
            idle();
        }
    }
    
    return 0;
}

static int xlsx_decode(lua_State* L) {
    const char* source = luaL_checkstring(L, 1);
    uint32_t size = lua_objlen(L, 1);
    xlsx::workbook wb;
    if (xlsx::read((const uint8_t*)source, size, wb)) {
        lua_createtable(L, 0, (int)wb.size());                  // wb
        for (auto& item : wb) {
            lua_pushstring(L, item.first.c_str());              // wb name
            lua_createtable(L, (int)item.second.size(), 0);     // wb name ws
            int row_index = 1, col_index = 1;
            for (auto& r : item.second) {
                lua_pushinteger(L, row_index);                  // wb name ws ri row
                lua_createtable(L, (int)r.size(), 0);           // wb name ws ri row
                col_index = 1;
                for (auto& cell : r) {
                    lua_pushinteger(L, col_index);              // wb name ws ri row ci
                    lua_pushstring(L, cell.c_str());            // wb name ws ri row ci cell
                    lua_settable(L, -3);                        // wb name ws ri row
                    col_index++;
                }
                lua_settable(L, -3);                            // wb name ws
                row_index++;
            }
            lua_settable(L, -3);                                // wb
        }
        return 1;
    }
    else {
        return 0;
    }
}

static const luaL_Reg lib[] = {
    { "decode", xlsx_decode },
    { nullptr, nullptr },
};

int lua_xlsx_open(lua_State* L)
{
    luaL_register(L, "xlsx", lib);
    return 1;
}

#ifdef BUILD_LUA_DLL_LIB
extern "C" __declspec(dllexport) int luaopen_xlsx(lua_State* L) {
    lua_newtable(L);
    luaL_register(L, nullptr, lib);
    return 1;
}
#endif
