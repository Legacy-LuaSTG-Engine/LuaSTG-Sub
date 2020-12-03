#pragma once
#include "slow/Ref.hpp"
#include <cstdint>

namespace slow {
	struct Data : public Ref {
		// get size
		virtual uint32_t size() = 0;
		// get data buffer
		virtual uint8_t* data() = 0;
		// is resizable?
		virtual bool resizable() = 0;
		// resize data buffer
		virtual bool resize(uint32_t size) = 0;
		
		// create new
		static bool create(uint32_t size, bool resizable, Data** output);
		static Data* create(uint32_t size, bool resizable);
		// create from file
		static bool create(const char* path, Data** output);
		static Data* create(const char* path);
		// create from file (utf16-LE)
		static bool create(const wchar_t* path, Data** output);
		static Data* create(const wchar_t* path);
		// create from memory
		static bool create(uint8_t* data, uint32_t size, Data** output);
		static Data* create(uint8_t* data, uint32_t size);
	};
};
