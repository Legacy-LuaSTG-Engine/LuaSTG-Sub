#pragma once
#include "slow/Data.hpp"
#include "RefImpl.hpp"
#include <cassert>
#include <cstdint>
#include <fstream>

namespace slow {
	class DataImpl : public RefImpl<Data> {
	private:
		bool _resizable;
		uint32_t _size = 0;
		uint8_t* _data;
	public:
		uint32_t size() {
			return _size;
		}
		uint8_t* data() {
			return _data;
		}
		bool resizable() {
			return _resizable;
		}
		bool resize(uint32_t size) {
			// check
			if (!_resizable) {
				return false;
			}
			// resize
			try {
				// free old
				if (_data != nullptr) {
					delete[] _data;
					_data = nullptr;
				}
				// alloc new
				_data = new uint8_t[size + 1]; // reserve 1 byte
				if (_data != nullptr) {
					std::memset(_data, 0, sizeof(uint8_t) * (size + 1));
				}
			}
			catch(...) {}
			// save size
			if ((size == 0) || ((size > 0) && (_data != nullptr))) {
				_size = size;
				return true;
			}
			else {
				_size = 0;
				return false;
			}
		}
	public:
		DataImpl(uint32_t size, bool resizable) : _resizable(true), _size(0), _data(nullptr) {
			resize(size);
			_resizable = resizable;
		}
		virtual ~DataImpl() {
			if (_data != nullptr) {
				delete[] _data;
				_data = nullptr;
			}
		}
	};
	
	bool Data::create(uint32_t size, bool resizable, Data** output) {
		assert((size >= 0) && (size <= 0x7FFFFFFF));
		assert(!(!resizable && (size == 0))); // can't resize but size is 0
		assert(output != nullptr);
		try {
			if (output != nullptr) {
				DataImpl* p = new DataImpl(size, resizable);
				*output = (Data*)p;
				return true;
			}
		}
		catch (...) { }
		return false;
	}
	Data* Data::create(uint32_t size, bool resizable) {
		Data* p = nullptr;
		Data::create(size, resizable, &p);
		return p;
	}
	bool Data::create(const char* path, Data** output) {
		assert(path != nullptr);
		assert(output != nullptr);
		try {
			if (output != nullptr) {
				std::ifstream file(path, std::ios::in | std::ios::binary);
				if (file.is_open()) {
					file.seekg(0, std::ios::end);
					auto p2 = file.tellg();
					file.seekg(0, std::ios::beg);
					auto p1 = file.tellg();
					auto dp = p2 - p1;
					if (dp <= 0x7FFFFFFF) {
						DataImpl* p = new DataImpl((uint32_t)dp, false);
						file.read((char*)p->data(), dp);
						file.close();
						*output = (Data*)p;
						return true;
					}
				}
			}
		}
		catch (...) {}
		return false;
	}
	Data* Data::create(const char* path) {
		Data* p = nullptr;
		Data::create(path, &p);
		return p;
	}
	bool Data::create(const wchar_t* path, Data** output) {
		assert(path != nullptr);
		assert(output != nullptr);
		try {
			if (output != nullptr) {
				std::ifstream file(path, std::ios::in | std::ios::binary);
				if (file.is_open()) {
					file.seekg(0, std::ios::end);
					auto p2 = file.tellg();
					file.seekg(0, std::ios::beg);
					auto p1 = file.tellg();
					auto dp = p2 - p1;
					if (dp <= 0x7FFFFFFF) {
						DataImpl* p = new DataImpl((uint32_t)dp, false);
						file.read((char*)p->data(), dp);
						file.close();
						*output = (Data*)p;
						return true;
					}
				}
			}
		}
		catch (...) {}
		return false;
	}
	Data* Data::create(const wchar_t* path) {
		Data* p = nullptr;
		Data::create(path, &p);
		return p;
	}
	bool Data::create(uint8_t* data, uint32_t size, Data** output) {
		assert(data != nullptr);
		assert((size > 0) && (size <= 0x7FFFFFFF));
		assert(output != nullptr);
		try {
			if (output != nullptr) {
				DataImpl* p = new DataImpl(size, false);
				std::memcpy(p->data(), data, size);
				*output = (Data*)p;
				return true;
			}
		}
		catch (...) {}
		return false;
	}
	Data* Data::create(uint8_t* data, uint32_t size) {
		Data* p = nullptr;
		Data::create(data, size, &p);
		return p;
	}
};
