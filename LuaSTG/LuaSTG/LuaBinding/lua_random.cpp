#pragma warning(disable:4310) // cast truncates constant value
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4244) // 'argument': conversion from 'unsigned int' to 'const pcg_extras::bitcount_t', possible loss of data

#include "lua_random.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "Utility/xorshift.hpp"
#include "pcg_random.hpp"
#include "Utility/sfc.hpp"
#include "Utility/jsf.hpp"

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
			self->rng.seed(static_cast<uint64_t>(self->seed));
			return 0;
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
	static int clone(lua_State* L)
	{
		auto* self = Cast(L, 1);
		auto* other = Create(L);
		other->rng = self->rng;
		other->int_gn = self->int_gn;
		other->num_gn = self->num_gn;
		other->seed = self->seed;
		return 1;
	}
	static int serialize(lua_State* L)noexcept
	{
		Data* self = Cast(L, 1);
		lua_pushstring(L, self->rng.serialize().c_str());
		return 1;
	}
	static int deserialize(lua_State* L)noexcept
	{
		Data* self = Cast(L, 1);
		lua_pushboolean(L, self->rng.deserialize(luaL_checkstring(L, 2)));
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
			{ "clone", &clone },
			{ "serialize", &serialize },
			{ "deserialize", &deserialize },
			// compatible api
			{ "Seed", &seed },
			{ "GetSeed", &seed },
			{ "Int", &integer },
			{ "Float", &number },
			{ "Sign", &sign },
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

// xoshiro128 family

MAKE_TYPE(xoshiro128p);
MAKE_TYPE(xoshiro128pp);
MAKE_TYPE(xoshiro128ss);

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

#undef MAKE_TYPE

template<typename RNG>
class RandomBasePCG
{
public:
	struct Data
	{
		RNG rng;
		std::uniform_int_distribution<lua_Integer> int_gn;
		std::uniform_real_distribution<lua_Number> num_gn;
		lua_Integer seed;

		constexpr size_t _Size() { return sizeof(*this); }

		inline void setSeed(lua_Integer seedv)
		{
			seed = seedv;
			pcg_extras::seed_seq_from<random::splitmix64> seed_rng(static_cast<typename random::splitmix64::result_type>(seed));
			rng.seed(seed_rng);
		}

		Data() { setSeed(0); }
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
			lua_Integer const seed = luaL_checkinteger(L, 2);
			self->setSeed(seed);
			return 0;
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
	static int clone(lua_State* L)
	{
		auto* self = Cast(L, 1);
		auto* other = Create(L);
		other->rng = self->rng;
		other->int_gn = self->int_gn;
		other->num_gn = self->num_gn;
		other->seed = self->seed;
		return 1;
	}
	static int serialize(lua_State* L)noexcept
	{
		Data* self = Cast(L, 1);
		lua_pushstring(L, self->rng.serialize().c_str());
		return 1;
	}
	static int deserialize(lua_State* L)noexcept
	{
		Data* self = Cast(L, 1);
		lua_pushboolean(L, self->rng.deserialize(luaL_checkstring(L, 2)));
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
			{ "clone", &clone },
			{ "serialize", &serialize },
			{ "deserialize", &deserialize },
			// compatible api
			{ "Seed", &seed },
			{ "GetSeed", &seed },
			{ "Int", &integer },
			{ "Float", &number },
			{ "Sign", &sign },
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

namespace random
{
	class pcg32_oneseq_ex : public pcg32_oneseq
	{
	protected:
		std::string_view name() { return "pcg32-oneseq"; }
	public:
		template<typename... Args>
		pcg32_oneseq_ex(Args&&... args) : pcg32_oneseq(std::forward<Args>(args)...) {}

		std::string serialize()
		{
			std::ostringstream ss;
			ss << name()
				<< "-" << multiplier()
				<< "-" << increment()
				<< "-" << state_;
			return ss.str();
		}
		bool deserialize(std::string const& data)
		{
			if (!data.starts_with(name())) {
				return false;
			}
			std::string tail = data.substr(name().size() + 1);
			for (auto& c : tail) {
				if (c == '-') {
					c = ' ';
				}
			}
			std::istringstream ss(tail);
			try {
				state_type v_multiplier{};
				state_type v_increment{};
				state_type v_state{};
				ss >> v_multiplier >> v_increment >> v_state;
				bool good = true;
				if (v_multiplier != multiplier()) {
					good = false;
				}
				else if (can_specify_stream) {
					set_stream(v_increment >> 1);
				}
				else if (v_increment != increment()) {
					good = false;
				}
				if (good) {
					state_ = v_state;
				}
				return good;
			}
			catch (std::exception const& e) {
				std::ignore = e;
			}
			return false;
		}
	};

	class pcg32_fast_ex : public pcg32_fast
	{
	protected:
		std::string_view name() { return "pcg32-fast"; }
	public:
		template<typename... Args>
		pcg32_fast_ex(Args&&... args) : pcg32_fast(std::forward<Args>(args)...) {}

		std::string serialize()
		{
			std::ostringstream ss;
			ss << name()
				<< "-" << multiplier()
				<< "-" << increment()
				<< "-" << state_;
			return ss.str();
		}
		bool deserialize(std::string const& data)
		{
			if (!data.starts_with(name())) {
				return false;
			}
			std::string tail = data.substr(name().size() + 1);
			for (auto& c : tail) {
				if (c == '-') {
					c = ' ';
				}
			}
			std::istringstream ss(tail);
			try {
				state_type v_multiplier{};
				state_type v_increment{};
				state_type v_state{};
				ss >> v_multiplier >> v_increment >> v_state;
				bool good = true;
				if (v_multiplier != multiplier()) {
					good = false;
				}
				else if (can_specify_stream) {
					set_stream(v_increment >> 1);
				}
				else if (v_increment != increment()) {
					good = false;
				}
				if (good) {
					state_ = v_state;
				}
				return good;
			}
			catch (std::exception const& e) {
				std::ignore = e;
			}
			return false;
		}
	};

	class pcg64_oneseq_ex : public pcg64_oneseq
	{
	protected:
		std::string_view name() { return "pcg64-oneseq"; }
	public:
		template<typename... Args>
		pcg64_oneseq_ex(Args&&... args) : pcg64_oneseq(std::forward<Args>(args)...) {}

		std::string serialize()
		{
			std::ostringstream ss;
			ss << name()
				<< "-" << multiplier()
				<< "-" << increment()
				<< "-" << state_;
			return ss.str();
		}
		bool deserialize(std::string const& data)
		{
			if (!data.starts_with(name())) {
				return false;
			}
			std::string tail = data.substr(name().size() + 1);
			for (auto& c : tail) {
				if (c == '-') {
					c = ' ';
				}
			}
			std::istringstream ss(tail);
			try {
				state_type v_multiplier{};
				state_type v_increment{};
				state_type v_state{};
				ss >> v_multiplier >> v_increment >> v_state;
				bool good = true;
				if (v_multiplier != multiplier()) {
					good = false;
				}
				else if (can_specify_stream) {
					set_stream(v_increment >> 1);
				}
				else if (v_increment != increment()) {
					good = false;
				}
				if (good) {
					state_ = v_state;
				}
				return good;
			}
			catch (std::exception const& e) {
				std::ignore = e;
			}
			return false;
		}
	};

	class pcg64_fast_ex : public pcg64_fast
	{
	protected:
		std::string_view name() { return "pcg64-fast"; }
	public:
		template<typename... Args>
		pcg64_fast_ex(Args&&... args) : pcg64_fast(std::forward<Args>(args)...) {}

		std::string serialize()
		{
			std::ostringstream ss;
			ss << name()
				<< "-" << multiplier()
				<< "-" << increment()
				<< "-" << state_;
			return ss.str();
		}
		bool deserialize(std::string const& data)
		{
			if (!data.starts_with(name())) {
				return false;
			}
			std::string tail = data.substr(name().size() + 1);
			for (auto& c : tail) {
				if (c == '-') {
					c = ' ';
				}
			}
			std::istringstream ss(tail);
			try {
				state_type v_multiplier{};
				state_type v_increment{};
				state_type v_state{};
				ss >> v_multiplier >> v_increment >> v_state;
				bool good = true;
				if (v_multiplier != multiplier()) {
					good = false;
				}
				else if (can_specify_stream) {
					set_stream(v_increment >> 1);
				}
				else if (v_increment != increment()) {
					good = false;
				}
				if (good) {
					state_ = v_state;
				}
				return good;
			}
			catch (std::exception const& e) {
				std::ignore = e;
			}
			return false;
		}
	};
}

#define MAKE_TYPE(T) \
	template class RandomBasePCG<random::T##_ex>;\
	std::string_view const RandomBasePCG<random::T##_ex>::ClassID("random." #T);\
	std::string_view const RandomBasePCG<random::T##_ex>::CreateID(#T);\
	using lua_##T##_t = RandomBasePCG<random::T##_ex>;

// pcg family

//MAKE_TYPE(pcg32);
MAKE_TYPE(pcg32_oneseq);
MAKE_TYPE(pcg32_fast);
//MAKE_TYPE(pcg64);
MAKE_TYPE(pcg64_oneseq);
MAKE_TYPE(pcg64_fast);

#undef MAKE_TYPE

template<typename RNG>
class RandomBaseOther
{
public:
	struct Data
	{
		RNG rng;
		std::uniform_int_distribution<lua_Integer> int_gn;
		std::uniform_real_distribution<lua_Number> num_gn;
		lua_Integer seed;

		constexpr size_t _Size() { return sizeof(*this); }

		inline void setSeed(lua_Integer seedv)
		{
			seed = seedv;
			random::splitmix64 seed_rng(static_cast<typename random::splitmix64::result_type>(seed));
			auto const a = static_cast<typename RNG::result_type>(seed_rng());
			if constexpr (std::is_same<RNG, jsf32>() || std::is_same<RNG, jsf64>())
			{
				rng = RNG(a);
			}
			else
			{
				auto const b = static_cast<typename RNG::result_type>(seed_rng());
				auto const c = static_cast<typename RNG::result_type>(seed_rng());
				rng = RNG(a, b, c);
			}
		}

		Data() { setSeed(0); }
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
			lua_Integer const seed = luaL_checkinteger(L, 2);
			self->setSeed(seed);
			return 0;
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
	static int clone(lua_State* L)
	{
		auto* self = Cast(L, 1);
		auto* other = Create(L);
		other->rng = self->rng;
		other->int_gn = self->int_gn;
		other->num_gn = self->num_gn;
		other->seed = self->seed;
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
			{ "clone", &clone },
			// compatible api
			{ "Seed", &seed },
			{ "GetSeed", &seed },
			{ "Int", &integer },
			{ "Float", &number },
			{ "Sign", &sign },
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
	template class RandomBaseOther<T>;\
	std::string_view const RandomBaseOther<T>::ClassID("random." #T);\
	std::string_view const RandomBaseOther<T>::CreateID(#T);\
	using lua_##T##_t = RandomBaseOther<T>;

// jsf family

MAKE_TYPE(jsf32);
MAKE_TYPE(jsf64);

// sfc family

MAKE_TYPE(sfc32);
MAKE_TYPE(sfc64);

#undef MAKE_TYPE

int luaopen_random(lua_State* L)
{
	luaL_Reg const empty[] = {
			{ NULL, NULL }
	};
	luaL_register(L, LibraryID.data(), empty); // ... random

#define REGISTER_TYPE(T)  lua_##T##_t::Register(L);

	REGISTER_TYPE(splitmix64);

	// xoshiro128 family

	REGISTER_TYPE(xoshiro128p);
	REGISTER_TYPE(xoshiro128pp);
	REGISTER_TYPE(xoshiro128ss);

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

	// pcg family

	//REGISTER_TYPE(pcg32);
	REGISTER_TYPE(pcg32_oneseq);
	REGISTER_TYPE(pcg32_fast);
	//REGISTER_TYPE(pcg64);
	REGISTER_TYPE(pcg64_oneseq);
	REGISTER_TYPE(pcg64_fast);

	// jsf family

	REGISTER_TYPE(jsf32);
	REGISTER_TYPE(jsf64);

	// sfc family

	REGISTER_TYPE(sfc32);
	REGISTER_TYPE(sfc64);

	return 1;
}
