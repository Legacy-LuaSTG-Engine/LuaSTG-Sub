#include "lua_xlsx_csv.h"

#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include "lua.hpp"

namespace csv {
    struct config_t {
        std::string sep;
        std::string border;
        std::string next;
        config_t() :
            sep(","), border("\""), next("\r\n") {}
        config_t(const std::string& pseq, const std::string& pborder, const std::string& pnext) :
            sep(pseq), border(pborder), next(pnext) {}
    };
    
    using row_t = std::vector<std::string>;
    using table_t = std::vector<row_t>;
    
    bool decode(const char* src, const size_t len, table_t& dest, const config_t& cfg) {
        const auto sep_len = cfg.sep.length();
        const auto border_len = cfg.border.length();
        const auto next_len = cfg.next.length();
        
        if (!(sep_len > 0 && border_len > 0 && next_len > 0)) {
            return false; // 兄啊这样怎么解析啊
        }
        
        const auto border_len2 = 2 * border_len;
        
        const char* sep = cfg.sep.c_str();
        const char* border = cfg.border.c_str();
        const char* next = cfg.next.c_str();
        
        char* view = (char*)src;
        size_t view_len = len;
        
        bool is_cell = false; // 包围cell
        bool cell_begin = true; // 是否是一个cell的开头
        row_t row;
        std::string cell;
        bool next_end = false; // 是否以换行符结尾
        if (std::strncmp(view + (view_len - next_len), next, next_len) == 0) {
            next_end = true;
        }
        
        while (view_len > 0) {
            if (!is_cell) {
                // 未进入包围cell
                if ((view_len >= sep_len) && (std::strncmp(view, sep, sep_len) == 0)) {
                    // cell结束
                    view += sep_len;
                    view_len -= sep_len;
                    row.push_back(cell);
                    cell.clear();
                    cell_begin = true; // 上一个cell结束，又是cell的开头了！
                }
                else if ((view_len >= next_len) && (std::strncmp(view, next, next_len) == 0)) {
                    // 没有在cell内，cell结束，行结束
                    view += next_len;
                    view_len -= next_len;
                    row.push_back(cell);
                    cell.clear();
                    dest.push_back(row);
                    row.clear();
                    cell_begin = true; // 上一个cell和行结束，又是cell的开头了！
                }
                else if (cell_begin && (view_len >= border_len) && (std::strncmp(view, border, border_len) == 0)) {
                    // 出现边界符，进入包围cell
                    view += border_len;
                    view_len -= border_len;
                    is_cell = true;
                }
                else {
                    // 储存cell的内容
                    cell.push_back(view[0]);
                    view++;
                    view_len--;
                    cell_begin = false; // 接下来就不是cell的开头了！
                }
            }
            else {
                // 包围cell状态
                if ((view_len >= border_len) && (std::strncmp(view, border, border_len) == 0)) {
                    // cell结束？
                    if ((view_len >= border_len2) && (std::strncmp(view + border_len, border, border_len) == 0)) {
                        // 还没有，是cell内的转义字符
                        for (auto* c = view; c < (view + border_len); c++) {
                            cell.push_back(*c);
                        }
                        // 连续前进两次
                        view += border_len2;
                        view_len -= border_len2;
                    }
                    else {
                        // 出现真·边界符，离开包围cell
                        view += border_len;
                        view_len -= border_len;
                        is_cell = false;
                    }
                }
                else {
                    // 储存cell的内容
                    cell.push_back(view[0]);
                    view++;
                    view_len--;
                }
            }
        }
        
        // 收尾工作，某些憨批软件生成的csv没结尾的换行符
        if (!next_end) {
            row.push_back(cell);
            dest.push_back(row);
        }
        
        return true;
    }
    
    bool decode(const std::string& src, table_t& dest, const config_t& cfg) {
        return decode(src.c_str(), src.length(), dest, cfg);
    }
    
    bool decode(std::ifstream& file, table_t& dest, const config_t& cfg) {
        // 计算大小
        const auto p2 = file.seekg(0, std::ios::end).tellg();
        const auto p1 = file.seekg(0, std::ios::beg).tellg();
        const auto dp = p2 - p1;
        // 读取数据
        std::string s;
        s.resize((size_t)dp);
        file.read(s.data(), dp);
        file.seekg(0, std::ios::beg); // 恢复
        // 接锅
        return decode(s, dest, cfg);
    }
};

static int csv_decode(lua_State* L) {
    const char* source = luaL_checkstring(L, 1);
    uint32_t size = lua_objlen(L, 1);
    csv::config_t cfg;
    int args = lua_gettop(L);
    if (args >= 2) {
        cfg.next = luaL_checkstring(L, 2);
    }
    if (args >= 3) {
        cfg.sep = luaL_checkstring(L, 3);
    }
    if (args >= 4) {
        cfg.border = luaL_checkstring(L, 4);
    }
    csv::table_t ws;
    if (csv::decode(source, size, ws, cfg)) {
        lua_createtable(L, (int)ws.size(), 0);     // ws
        int row_index = 1, col_index = 1;
        for (auto& r : ws) {
            lua_pushinteger(L, row_index);                  // ws ri row
            lua_createtable(L, (int)r.size(), 0);           // ws ri row
            col_index = 1;
            for (auto& cell : r) {
                lua_pushinteger(L, col_index);              // ws ri row ci
                lua_pushstring(L, cell.c_str());            // ws ri row ci cell
                lua_settable(L, -3);                        // ws ri row
                col_index++;
            }
            lua_settable(L, -3);                            // ws
            row_index++;
        }
        return 1;
    }
    else {
        return 0;
    }
}

static const luaL_Reg lib[] = {
    { "decode", csv_decode },
    { nullptr, nullptr },
};

int lua_csv_open(lua_State* L)
{
    luaL_register(L, "csv", lib);
    return 1;
}

#ifdef BUILD_LUA_DLL_LIB
extern "C" __declspec(dllexport) int luaopen_csv(lua_State* L) {
    lua_newtable(L);
    luaL_register(L, nullptr, lib);
    return 1;
}
#endif
