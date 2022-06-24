#include "lua_random.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "Utility/xorshift.hpp"

static std::string_view const LibraryID("random");

template<typename RNG>
class RandomBase
{
public:
	struct Data
	{
		RNG rng;
		std::uniform_int_distribution<lua_Integer> int_gn;
		std::uniform_real_distribution<lua_Number> num_gn;
		lua_Integer seed;
		Data() : rng(0), seed(0) {  }
		~Data() {}
	};

	static std::string_view const ClassID;

private:
	static std::string_view const CreateID;

	static int seed(lua_State* L)
	{
		Data* self = Cast(L, 1);
		int const argc = lua_gettop(L);
		if (argc == 1)
		{
			lua_pushinteger(L, self->seed);
			return 1;
		}
		else if (argc == 2)
		{
			self->seed = luaL_checkinteger(L, 2);
			self->rng.seed(uint64_t(self->seed));
			return 0;
		}
		else
		{
			return luaL_error(L, "invalid parameter");
		}
	}
	static int _bad_integer(lua_State* L)
	{
		Data* self = Cast(L, 1);
		int const argc = lua_gettop(L);
		if (argc == 1)
		{
			uint64_t const d = self->rng();
			if constexpr (sizeof(lua_Integer) == sizeof(uint64_t))
			{
				lua_pushinteger(L, static_cast<lua_Integer>(d));
			}
			else
			{
				static_assert(sizeof(lua_Integer) == sizeof(uint32_t));
				lua_pushinteger(L, static_cast<lua_Integer>((d >> 32) & 0xFFFFFFFFu)); // discard low 32bits
			}
			return 1;
		}
		else if (argc == 2)
		{
			lua_Integer b = luaL_checkinteger(L, 2);
			if (b <= 0)
				return luaL_error(L, "invalid parameter, required upper_bound > 0");
			uint64_t const d = random::bounded_rand(self->rng, uint64_t(b));
			lua_pushinteger(L, lua_Integer(d));
			return 1;
		}
		else if (argc == 3)
		{
			lua_Integer a = luaL_checkinteger(L, 2);
			lua_Integer b = luaL_checkinteger(L, 3);
			if (a > b) std::swap(a, b);
			lua_Integer const c = b - a;
			uint64_t const d = random::bounded_rand(self->rng, uint64_t(c));
			lua_pushinteger(L, a + lua_Integer(d));
			return 1;
		}
		else
		{
			return luaL_error(L, "invalid parameter");
		}
	}
	static int integer(lua_State* L)
	{
		Data* self = Cast(L, 1);
		int const argc = lua_gettop(L);
		if (argc == 1)
		{
			lua_pushinteger(L, self->int_gn(self->rng, std::uniform_int_distribution<lua_Integer>::param_type(
				0, std::numeric_limits<lua_Integer>::max())));
			return 1;
		}
		else if (argc == 2)
		{
			lua_Integer b = luaL_checkinteger(L, 2);
			if (b < 0) b = -b;
			lua_pushinteger(L, self->int_gn(self->rng, std::uniform_int_distribution<lua_Integer>::param_type(0, b)));
			return 1;
		}
		else if (argc == 3)
		{
			lua_Integer a = luaL_checkinteger(L, 2);
			lua_Integer b = luaL_checkinteger(L, 3);
			if (a > b) std::swap(a, b);
			lua_pushinteger(L, self->int_gn(self->rng, std::uniform_int_distribution<lua_Integer>::param_type(a, b)));
			return 1;
		}
		else
		{
			return luaL_error(L, "invalid parameter");
		}
	}
	static int _bad_number(lua_State* L)
	{
		Data* self = Cast(L, 1);
		int const argc = lua_gettop(L);
		if (argc == 1)
		{
			double const d = random::to_double(self->rng());
			lua_pushnumber(L, d);
			return 1;
		}
		else if (argc == 2)
		{
			lua_Number b = luaL_checknumber(L, 2);
			if (b <= 0.0)
				return luaL_error(L, "invalid parameter, required upper_bound > 0");
			double const d = random::to_double(self->rng());
			lua_pushnumber(L, b * d);
			return 1;
		}
		else if (argc == 3)
		{
			lua_Number a = luaL_checknumber(L, 2);
			lua_Number b = luaL_checknumber(L, 3);
			if (a > b) std::swap(a, b);
			lua_Number const c = b - a;
			double const d = random::to_double(self->rng());
			lua_pushnumber(L, a + c * d);
			return 1;
		}
		else
		{
			return luaL_error(L, "invalid parameter");
		}
	}
	static int number(lua_State* L)
	{
		Data* self = Cast(L, 1);
		int const argc = lua_gettop(L);
		if (argc == 1)
		{
			lua_pushnumber(L, self->num_gn(self->rng, std::uniform_real_distribution<lua_Number>::param_type(
				0.0, std::nextafter(1.0, std::numeric_limits<lua_Number>::max()))));
			return 1;
		}
		else if (argc == 2)
		{
			lua_Number b = luaL_checknumber(L, 2);
			if (b < 0.0) b = -b;
			lua_pushnumber(L, self->num_gn(self->rng, std::uniform_real_distribution<lua_Number>::param_type(
				0.0, std::nextafter(b, std::numeric_limits<lua_Number>::max()))));
			return 1;
		}
		else if (argc == 3)
		{
			lua_Number a = luaL_checknumber(L, 2);
			lua_Number b = luaL_checknumber(L, 3);
			if (a > b) std::swap(a, b);
			lua_pushnumber(L, self->num_gn(self->rng, std::uniform_real_distribution<lua_Number>::param_type(
				a, std::nextafter(b, std::numeric_limits<lua_Number>::max()))));
			return 1;
		}
		else
		{
			return luaL_error(L, "invalid parameter");
		}
	}
	static int sign(lua_State* L)
	{
		Data* self = Cast(L, 1);
		lua_pushinteger(L, self->int_gn(self->rng, std::uniform_int_distribution<lua_Integer>::param_type(0, 1)) * 2 - 1);
		return 1;
	}

	static int __gc(lua_State* L)
	{
		Data* self = Cast(L, 1);
		self->~Data();
		return 1;
	}
	static int __tostring(lua_State* L)
	{
		std::ignore = Cast(L, 1);
		lua_push_string_view(L, ClassID);
		return 1;
	}

	static int create(lua_State* L)
	{
		std::ignore = Create(L);
		return 1;
	}

public:
	static Data* Cast(lua_State* L, int idx)
	{
		return static_cast<Data*>(luaL_checkudata(L, idx, ClassID.data()));
	}
	static Data* Create(lua_State* L)
	{
		Data* self = static_cast<Data*>(lua_newuserdata(L, sizeof(Data)));
		new(self) Data();
		luaL_getmetatable(L, ClassID.data());
		lua_setmetatable(L, -2);
		return self;
	}
	static void Register(lua_State* L)
	{
		luaL_Reg const lib[] = {
			{ "seed", &seed },
			{ "integer", &integer },
			{ "number", &number },
			{ "sign", &sign },
			{ NULL, NULL }
		};

		luaL_Reg const mt[] = {
			{ "__gc", &__gc },
			{ "__tostring", &__tostring },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, ClassID.data()); // ... mt
		luaL_register(L, NULL, mt);           // ... mt
		lua_push_string_view(L, "__index");   // ... mt "__index"
		lua_createtable(L, 0, 4);             // ... mt "__index" lib
		luaL_register(L, NULL, lib);          // ... mt "__index" lib
		lua_settable(L, -3);                  // ... mt
		lua_pop(L, 1);                        // ...

		luaL_Reg const ctor[] = {
			{ CreateID.data(), &create},
			{ NULL, NULL }
		};

		luaL_register(L, LibraryID.data(), ctor); // ... random
		lua_pop(L, 1);
	}
};

#define MAKE_TYPE(T) \
	template class RandomBase<random::T>;\
	std::string_view const RandomBase<random::T>::ClassID("random." #T);\
	std::string_view const RandomBase<random::T>::CreateID(#T);\
	using lua_##T##_t = RandomBase<random::T>;

MAKE_TYPE(splitmix64);

// xoroshiro128 family

MAKE_TYPE(xoroshiro128p);
MAKE_TYPE(xoroshiro128pp);
MAKE_TYPE(xoroshiro128ss);

// xoshiro256 family

MAKE_TYPE(xoshiro256p);
MAKE_TYPE(xoshiro256pp);
MAKE_TYPE(xoshiro256ss);

// xoshiro512 family

MAKE_TYPE(xoshiro512p);
MAKE_TYPE(xoshiro512pp);
MAKE_TYPE(xoshiro512ss);

// xoroshiro1024 family

MAKE_TYPE(xoroshiro1024s);
MAKE_TYPE(xoroshiro1024pp);
MAKE_TYPE(xoroshiro1024ss);

int luaopen_random(lua_State* L)
{
	luaL_Reg const empty[] = {
			{ NULL, NULL }
	};
	luaL_register(L, LibraryID.data(), empty); // ... random

#define REGISTER_TYPE(T)  lua_##T##_t::Register(L);

	REGISTER_TYPE(splitmix64);

	// xoroshiro128 family

	REGISTER_TYPE(xoroshiro128p);
	REGISTER_TYPE(xoroshiro128pp);
	REGISTER_TYPE(xoroshiro128ss);

	// xoshiro256 family

	REGISTER_TYPE(xoshiro256p);
	REGISTER_TYPE(xoshiro256pp);
	REGISTER_TYPE(xoshiro256ss);

	// xoshiro512 family

	REGISTER_TYPE(xoshiro512p);
	REGISTER_TYPE(xoshiro512pp);
	REGISTER_TYPE(xoshiro512ss);

	// xoroshiro1024 family

	REGISTER_TYPE(xoroshiro1024s);
	REGISTER_TYPE(xoroshiro1024pp);
	REGISTER_TYPE(xoroshiro1024ss);

	return 1;
}
