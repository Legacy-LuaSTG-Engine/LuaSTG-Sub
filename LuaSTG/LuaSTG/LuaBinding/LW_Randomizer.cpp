#include "LuaBinding/LuaWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////
/// @brief WELL512随机数算法
/// @note  摘自《游戏编程精粹 7》
////////////////////////////////////////////////////////////////////////////////
class fcyRandomWELL512
{
private:
	uint32_t m_State[16];   ///< @brief 内部状态机
	uint32_t m_Index;       ///< @brief 下标
	uint32_t m_RSeed;       ///< @brief 随机数种子
public:
	/// @brief 获得随机数种子
	uint32_t GetRandSeed() const;
	/// @brief     设置随机数种子
	/// @param[in] Seed 随机数种子
	void SetSeed(uint32_t Seed);
	/// @brief 产生无符号整数
	uint32_t GetRandUInt();
	/// @brief     产生[0, Max]之间的无符号整数
	/// @param[in] Max 最大数
	uint32_t GetRandUInt(uint32_t Max);
	/// @brief 产生[0, 1]之间的随机小数
	float GetRandFloat();
	/// @brief 产生[MinBound, MaxBound]之间的随机小数
	/// @param[in] MinBound 下界
	/// @param[in] MaxBound 上界
	float GetRandFloat(float MinBound, float MaxBound);

	std::string Serialize();

	bool Deserialize(std::string const& data);
public:
	/// @brief 默认构造函数
	/// @note  调用GetTickCount()进行初始化
	fcyRandomWELL512();
	/// @brief 指定种子初始化随机数发生器
	fcyRandomWELL512(uint32_t Seed);
	fcyRandomWELL512(fcyRandomWELL512 const&) = default;
	~fcyRandomWELL512();
};

fcyRandomWELL512::fcyRandomWELL512()
{
	SetSeed(clock());
}

fcyRandomWELL512::fcyRandomWELL512(uint32_t Seed)
{
	SetSeed(Seed);
}

fcyRandomWELL512::~fcyRandomWELL512()
{}

uint32_t fcyRandomWELL512::GetRandSeed() const
{
	return m_RSeed;
}

void fcyRandomWELL512::SetSeed(uint32_t Seed)
{
	m_RSeed = Seed;

	m_Index = 0;

	// 线性同余产生种子
	const unsigned int mask = ~0u;
	m_State[0] = Seed & mask;
	for (unsigned int i = 1; i < 16; ++i)
	{
		m_State[i] = (1812433253UL * (m_State[i - 1] ^ (m_State[i - 1] >> 30)) + i) & mask;
	}
}

uint32_t fcyRandomWELL512::GetRandUInt()
{
	unsigned int a, b, c, d;
	a = m_State[m_Index];
	c = m_State[(m_Index + 13) & 15];
	b = a ^ c ^ (a << 16) ^ (c << 15);
	c = m_State[(m_Index + 9) & 15];
	c ^= (c >> 11);
	a = m_State[m_Index] = b ^ c;
	d = a ^ ((a << 5) & 0xDA442D24UL);
	m_Index = (m_Index + 15) & 15;
	a = m_State[m_Index];
	m_State[m_Index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
	return m_State[m_Index];
}

uint32_t fcyRandomWELL512::GetRandUInt(uint32_t Max)
{
	return GetRandUInt() % (Max + 1);
}

float fcyRandomWELL512::GetRandFloat()
{
	return GetRandUInt(1000000) / 1000000.f;
}

float fcyRandomWELL512::GetRandFloat(float MinBound, float MaxBound)
{
	return GetRandFloat() * (MaxBound - MinBound) + MinBound;
}

std::string fcyRandomWELL512::Serialize()
{
	return std::format("well512-"
		"{}-"
		"{}-{}-{}-{}-"
		"{}-{}-{}-{}-"
		"{}-{}-{}-{}-"
		"{}-{}-{}-{}-",
		m_Index,
		m_State[0], m_State[1], m_State[2], m_State[3],
		m_State[4], m_State[5], m_State[6], m_State[7],
		m_State[8], m_State[9], m_State[10], m_State[11],
		m_State[12], m_State[13], m_State[14], m_State[15]);
}

bool fcyRandomWELL512::Deserialize(std::string const& data)
{
	constexpr std::string_view head("well512-");
	if (!data.starts_with(head)) {
		return false;
	}
	std::string tail = data.substr(head.size());
	for (auto& c : tail) {
		if (c == '-') {
			c = ' ';
		}
	}
	std::stringstream iss(tail);
	try {
		uint32_t v_Index{};
		uint32_t v_State[16]{};
		iss >> v_Index
			>> v_State[0] >> v_State[1] >> v_State[2] >> v_State[3]
			>> v_State[4] >> v_State[5] >> v_State[6] >> v_State[7]
			>> v_State[8] >> v_State[9] >> v_State[10] >> v_State[11]
			>> v_State[12] >> v_State[13] >> v_State[14] >> v_State[15];
		m_Index = v_Index;
		std::memcpy(m_State, v_State, sizeof(m_State));
		return true;
	}
	catch (std::exception const& e) {
		std::ignore = e;
	}
	return false;
}

namespace luastg::binding
{
	template<typename T>
	void make_less(T& a, T& b) {
		if (a > b) {
			std::swap(a, b);
		}
	}

	void Randomizer::Register(lua_State* L)noexcept
	{
		struct Function
		{
		#define GETUDATA(p, i) fcyRandomWELL512* (p) = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_RANDGEN));
			static int Seed(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				p->SetSeed((uint32_t)luaL_checkinteger(L, 2));
				return 0;
			}
			static int GetSeed(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				lua_pushinteger(L, (lua_Integer)p->GetRandSeed());
				return 1;
			}
			static int Int(lua_State* const L) noexcept
			{
				GETUDATA(p, 1);
				auto a = luaL_checkinteger(L, 2);
				auto b = luaL_checkinteger(L, 3);
				make_less(a, b);
				auto const range = static_cast<uint64_t>(b - a);
				if (range > 0x7fffffff) {
					auto const message = std::format("range [a:{}, b:{}] too large, (b - a) must <= 2147483647", a, b);
					return luaL_error(L, message.c_str());
				}
				auto const result = p->GetRandUInt(static_cast<uint32_t>(range));
				lua_pushinteger(L, a + static_cast<lua_Integer>(result));
				return 1;
			}
			static int Float(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				float a = (float)luaL_checknumber(L, 2);
				float b = (float)luaL_checknumber(L, 3);
				make_less(a, b);
				lua_pushnumber(L, (lua_Number)p->GetRandFloat(a, b));
				return 1;
			}
			static int Sign(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				lua_pushinteger(L, (lua_Integer)p->GetRandUInt(1) * 2 - 1);
				return 1;
			}
			static int clone(lua_State* L)noexcept
			{
				GETUDATA(self, 1);
				Randomizer::CreateAndPush(L);
				GETUDATA(other, -1);
				*other = *self;
				return 1;
			}
			static int serialize(lua_State* L)noexcept
			{
				GETUDATA(self, 1);
				lua_pushstring(L, self->Serialize().c_str());
				return 1;
			}
			static int deserialize(lua_State* L)noexcept
			{
				GETUDATA(self, 1);
				lua_pushboolean(L, self->Deserialize(luaL_checkstring(L, 2)));
				return 1;
			}
			static int Meta_ToString(lua_State* L)noexcept
			{
				lua_pushfstring(L, LUASTG_LUA_TYPENAME_RANDGEN);
				return 1;
			}
		#undef GETUDATA
		};

		luaL_Reg tMethods[] =
		{
			{ "Seed", &Function::Seed },
			{ "GetSeed", &Function::GetSeed },
			{ "Int", &Function::Int },
			{ "Float", &Function::Float },
			{ "Sign", &Function::Sign },
			{ "clone", &Function::clone },
			{ "serialize", &Function::serialize },
			{ "deserialize", &Function::deserialize },
			{ NULL, NULL }
		};

		luaL_Reg tMetaTable[] =
		{
			{ "__tostring", &Function::Meta_ToString },
			{ NULL, NULL }
		};

		RegisterClassIntoTable(L, ".Rand", tMethods, LUASTG_LUA_TYPENAME_RANDGEN, tMetaTable);
	}

	void Randomizer::CreateAndPush(lua_State* L)
	{
		fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(lua_newuserdata(L, sizeof(fcyRandomWELL512))); // udata
		new(p) fcyRandomWELL512();
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_RANDGEN); // udata mt
		lua_setmetatable(L, -2); // udata
	}
}
