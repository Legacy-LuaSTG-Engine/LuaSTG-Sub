#include "lua_dwrite.hpp"

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <functional>

#include "utility/encoding.hpp"
#include "platform/HResultChecker.hpp"
#include "Core/FileManager.hpp"
#include "AppFrame.h"
#include "LuaBinding/LuaWrapper.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSERVICE
#define NOMCX
#define NOIME

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#ifdef WINVER
#undef WINVER
#endif

#include <sdkddkver.h>

#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <wincodec.h>
#include <d2d1_3.h>
#include <dwrite_3.h>

#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef NOSERVICE
#undef NOMCX
#undef NOIME

namespace DirectWrite
{
	// c++ helper

	struct ScopeFunction
	{
		std::function<void()> f;
		ScopeFunction(std::function<void()> fv) : f(fv) {}
		~ScopeFunction() { f(); }
	};

	// wic helper

	class AutoDeleteFileWIC
	{
	public:
		AutoDeleteFileWIC(Microsoft::WRL::ComPtr<IWICStream>& hFile, std::wstring_view szFile) noexcept
			: m_filename(szFile), m_handle(hFile) {}
		~AutoDeleteFileWIC()
		{
			if (!m_filename.empty())
			{
				m_handle.Reset();
				DeleteFileW(m_filename.data());
			}
		}

		AutoDeleteFileWIC(const AutoDeleteFileWIC&) = delete;
		AutoDeleteFileWIC& operator=(const AutoDeleteFileWIC&) = delete;

		AutoDeleteFileWIC(const AutoDeleteFileWIC&&) = delete;
		AutoDeleteFileWIC& operator=(const AutoDeleteFileWIC&&) = delete;

		void clear() noexcept { m_filename = m_filename.substr(0, 0); }

	private:
		std::wstring_view m_filename;
		Microsoft::WRL::ComPtr<IWICStream>& m_handle;
	};

	// lua helper

	inline void lua_push_string_view(lua_State* L, std::string_view sv)
	{
		lua_pushlstring(L, sv.data(), sv.size());
	}
	inline std::string_view luaL_check_string_view(lua_State* L, int idx)
	{
		size_t len = 0;
		char const* str = luaL_checklstring(L, idx, &len);
		return std::string_view(str, len);
	}

	template<typename E>
	inline E luaL_check_C_enum(lua_State* L, int idx)
	{
		return static_cast<E>(luaL_checkinteger(L, idx));
	}

	inline bool lua_to_boolean(lua_State* L, int idx)
	{
		return lua_toboolean(L, idx);
	}
	inline void lua_push_float(lua_State* L, float v)
	{
		lua_pushnumber(L, v);
	}
	inline float luaL_check_float(lua_State* L, int idx)
	{
		return (float)luaL_checknumber(L, idx);
	}
	inline float luaL_optional_float(lua_State* L, int idx, float v)
	{
		return (float)luaL_optnumber(L, idx, v);
	}
	inline uint32_t luaL_check_uint32(lua_State* L, int idx)
	{
		if constexpr (sizeof(lua_Integer) >= 8)
		{
			return 0xFFFFFFFFu & luaL_checkinteger(L, idx);
		}
		else
		{
			return (uint32_t)luaL_checknumber(L, idx);
		}
	}

	// DirectWrite helper

	void printFontCollectionInfo(IDWriteFontCollection* dwrite_font_collection, std::stringstream& string_buffer)
	{
		for (UINT32 ff_idx = 0; ff_idx < dwrite_font_collection->GetFontFamilyCount(); ff_idx += 1)
		{
			string_buffer << '[' << ff_idx << "] Font Family\n";

			Microsoft::WRL::ComPtr<IDWriteFontFamily> dwrite_font_family;
			gHR = dwrite_font_collection->GetFontFamily(ff_idx, &dwrite_font_family);

			string_buffer << "    Name:\n";
			Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> dwrite_font_family_names;
			gHR = dwrite_font_family->GetFamilyNames(&dwrite_font_family_names);
			for (UINT32 name_idx = 0; name_idx < dwrite_font_family_names->GetCount(); name_idx += 1)
			{
				UINT32 str_len = 0;

				gHR = dwrite_font_family_names->GetStringLength(name_idx, &str_len);
				std::wstring name(str_len + 1, L'\0');
				gHR = dwrite_font_family_names->GetString(name_idx, name.data(), str_len + 1);
				if (name.back() == L'\0') name.pop_back();

				gHR = dwrite_font_family_names->GetLocaleNameLength(name_idx, &str_len);
				std::wstring locale_name(str_len + 1, L'\0');
				gHR = dwrite_font_family_names->GetLocaleName(name_idx, locale_name.data(), str_len + 1);
				if (locale_name.back() == L'\0') locale_name.pop_back();

				string_buffer << "        [" << name_idx << "] (" << utility::encoding::to_utf8(locale_name) << ") " << utility::encoding::to_utf8(name) << '\n';
			}

			string_buffer << "    Font:\n";
			for (UINT32 font_idx = 0; font_idx < dwrite_font_family->GetFontCount(); font_idx += 1)
			{
				string_buffer << "        [" << font_idx << "] Font\n";

				Microsoft::WRL::ComPtr<IDWriteFont> dwrite_font;
				gHR = dwrite_font_family->GetFont(font_idx, &dwrite_font);

				string_buffer << "            Name:\n";
				Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> dwrite_font_face_names;
				gHR = dwrite_font->GetFaceNames(&dwrite_font_face_names);
				for (UINT32 name_idx = 0; name_idx < dwrite_font_face_names->GetCount(); name_idx += 1)
				{
					UINT32 str_len = 0;

					gHR = dwrite_font_face_names->GetStringLength(name_idx, &str_len);
					std::wstring name(str_len + 1, L'\0');
					gHR = dwrite_font_face_names->GetString(name_idx, name.data(), str_len + 1);
					if (name.back() == L'\0') name.pop_back();

					gHR = dwrite_font_face_names->GetLocaleNameLength(name_idx, &str_len);
					std::wstring locale_name(str_len + 1, L'\0');
					gHR = dwrite_font_face_names->GetLocaleName(name_idx, locale_name.data(), str_len + 1);
					if (locale_name.back() == L'\0') locale_name.pop_back();

					string_buffer << "                [" << name_idx << "] (" << utility::encoding::to_utf8(locale_name) << ") " << utility::encoding::to_utf8(name) << '\n';
				}

				/*
				string_buffer << "            Information:\n";
				struct InfoStringCollection
				{
					DWRITE_INFORMATIONAL_STRING_ID id;
					std::string_view name;
				};
				std::array<InfoStringCollection, 21> const info_string_collections = {
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_COPYRIGHT_NOTICE, "Copyright" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_VERSION_STRINGS, "Version" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_TRADEMARK, "Trademark" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_MANUFACTURER, "Manufacturer" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_DESIGNER, "Designer" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_DESIGNER_URL, "Designer (URL)" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_DESCRIPTION, "Description" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_FONT_VENDOR_URL, "Vendor (URL)" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_LICENSE_DESCRIPTION, "License" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_LICENSE_INFO_URL, "License (URL)" },

					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_WIN32_FAMILY_NAMES, "Family Names (GDI)" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_WIN32_SUBFAMILY_NAMES, "SubFamily Names (GDI)" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_TYPOGRAPHIC_FAMILY_NAMES, "Typographic Family Names (GDI)" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_TYPOGRAPHIC_SUBFAMILY_NAMES, "Typographic SubFamily Names (GDI)" },

					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_SAMPLE_TEXT, "Sample Text" },

					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_FULL_NAME, "Full Name" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME, "Postscript Name" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_CID_NAME, "Postscript CID Findfont Name" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_WEIGHT_STRETCH_STYLE_FAMILY_NAME, "Weight-stretch-style Family Name" },

					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_DESIGN_SCRIPT_LANGUAGE_TAG, "Design Script/Language Tag" },
					InfoStringCollection{ DWRITE_INFORMATIONAL_STRING_SUPPORTED_SCRIPT_LANGUAGE_TAG, "Support Script/Language Tag" },
				};
				for (uint32_t info_idx = 0; info_idx < info_string_collections.size(); info_idx += 1)
				{
					string_buffer << "                " << info_string_collections[info_idx].name << ":\n";

					Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> info_text;
					BOOL is_exist = FALSE;
					HR = dwrite_font->GetInformationalStrings(info_string_collections[info_idx].id, &info_text, &is_exist);
					if (is_exist)
					{
						for (UINT32 name_idx = 0; name_idx < info_text->GetCount(); name_idx += 1)
						{
							UINT32 str_len = 0;

							HR = info_text->GetStringLength(name_idx, &str_len);
							std::wstring name(str_len + 1, L'\0');
							HR = info_text->GetString(name_idx, name.data(), str_len + 1);
							if (name.back() == L'\0') name.pop_back();

							HR = info_text->GetLocaleNameLength(name_idx, &str_len);
							std::wstring locale_name(str_len + 1, L'\0');
							HR = info_text->GetLocaleName(name_idx, locale_name.data(), str_len + 1);
							if (locale_name.back() == L'\0') locale_name.pop_back();

							string_buffer << "                    [" << name_idx << "] (" << to_utf8(locale_name) << ") " << to_utf8(name) << '\n';
						}
					}
				}
				//*/

				string_buffer << "            Simulations: ";
				auto const font_sim = dwrite_font->GetSimulations();
				if (font_sim == DWRITE_FONT_SIMULATIONS_NONE) string_buffer << "None";
				if (font_sim & DWRITE_FONT_SIMULATIONS_BOLD) string_buffer << "Algorithmic Emboldening";
				if ((font_sim & DWRITE_FONT_SIMULATIONS_BOLD) && (font_sim & DWRITE_FONT_SIMULATIONS_OBLIQUE)) string_buffer << ", ";
				if (font_sim & DWRITE_FONT_SIMULATIONS_OBLIQUE) string_buffer << "Algorithmic Italicization";
				string_buffer << "\n";

				switch (dwrite_font->GetStretch())
				{
				case DWRITE_FONT_STRETCH_UNDEFINED:       string_buffer << "            Stretch: Not known (0)\n";       break;
				case DWRITE_FONT_STRETCH_ULTRA_CONDENSED: string_buffer << "            Stretch: Ultra-condensed (1)\n"; break;
				case DWRITE_FONT_STRETCH_EXTRA_CONDENSED: string_buffer << "            Stretch: Extra-condensed (2)\n"; break;
				case DWRITE_FONT_STRETCH_CONDENSED:       string_buffer << "            Stretch: Condensed (3)\n";       break;
				case DWRITE_FONT_STRETCH_SEMI_CONDENSED:  string_buffer << "            Stretch: Semi-condensed (4)\n";  break;
				case DWRITE_FONT_STRETCH_NORMAL:          string_buffer << "            Stretch: Normal/Medium (5)\n";   break;
				case DWRITE_FONT_STRETCH_SEMI_EXPANDED:   string_buffer << "            Stretch: Semi-expanded (6)\n";   break;
				case DWRITE_FONT_STRETCH_EXPANDED:        string_buffer << "            Stretch: Expanded (7)\n";        break;
				case DWRITE_FONT_STRETCH_EXTRA_EXPANDED:  string_buffer << "            Stretch: Extra-expanded (8)\n";  break;
				case DWRITE_FONT_STRETCH_ULTRA_EXPANDED:  string_buffer << "            Stretch: Ultra-expanded (9)\n";  break;
				default: assert(false); break;
				}

				switch (dwrite_font->GetStyle())
				{
				case DWRITE_FONT_STYLE_NORMAL:  string_buffer << "            Slope Style: Normal\n";  break;
				case DWRITE_FONT_STYLE_OBLIQUE: string_buffer << "            Slope Style: Oblique\n"; break;
				case DWRITE_FONT_STYLE_ITALIC:  string_buffer << "            Slope Style: Italic\n";  break;
				default: assert(false); break;
				}

				auto const font_weight = dwrite_font->GetWeight();
				switch (font_weight)
				{
				case DWRITE_FONT_WEIGHT_THIN:        string_buffer << "            Weight: Thin (100)\n";                    break;
				case DWRITE_FONT_WEIGHT_EXTRA_LIGHT: string_buffer << "            Weight: Extra-light/Ultra-light (200)\n"; break;
				case DWRITE_FONT_WEIGHT_LIGHT:       string_buffer << "            Weight: Light (300)\n";                   break;
				case DWRITE_FONT_WEIGHT_SEMI_LIGHT:  string_buffer << "            Weight: Semi-light (350)\n";              break;
				case DWRITE_FONT_WEIGHT_NORMAL:      string_buffer << "            Weight: Normal/Regular (400)\n";          break;
				case DWRITE_FONT_WEIGHT_MEDIUM:      string_buffer << "            Weight: Medium (500)\n";                  break;
				case DWRITE_FONT_WEIGHT_DEMI_BOLD:   string_buffer << "            Weight: Demi-bold/Semi-bold (600)\n";     break;
				case DWRITE_FONT_WEIGHT_BOLD:        string_buffer << "            Weight: Bold (700)\n";                    break;
				case DWRITE_FONT_WEIGHT_EXTRA_BOLD:  string_buffer << "            Weight: Extra-bold/Ultra-bold (800)\n";   break;
				case DWRITE_FONT_WEIGHT_BLACK:       string_buffer << "            Weight: Black/Heavy (900)\n";             break;
				case DWRITE_FONT_WEIGHT_EXTRA_BLACK: string_buffer << "            Weight: Extra-black/Ultra-black (950)\n"; break;
				default: string_buffer << "            Weight: " << (int)font_weight << "\n"; break;
				}

				string_buffer << "            Symbol Font: " << (dwrite_font->IsSymbolFont() ? "Yes" : "No") << "\n";
			}
		}
	}

	template<typename T>
	class UnknownImplement : public T
	{
	private:
		volatile unsigned long m_ref;
	public:
		HRESULT WINAPI QueryInterface(IID const& riid, void** ppvObject)
		{
			if (riid == __uuidof(IUnknown))
			{
				AddRef();
				*ppvObject = static_cast<IUnknown*>(this);
				return S_OK;
			}
			else if (riid == __uuidof(T))
			{
				AddRef();
				*ppvObject = static_cast<T*>(this);
				return S_OK;
			}
			else
			{
				return E_NOINTERFACE;
			}
		}
		ULONG WINAPI AddRef()
		{
			return InterlockedIncrement(&m_ref);
		}
		ULONG WINAPI Release()
		{
			ULONG const ref_count = InterlockedDecrement(&m_ref);
			if (ref_count == 0u)
			{
				delete this;
			}
			return ref_count;
		}
	public:
		UnknownImplement() : m_ref(1) {}
		virtual ~UnknownImplement() {}
	};

	class DWriteFontFileStreamImplement : public UnknownImplement<IDWriteFontFileStream>
	{
	private:
		std::vector<uint8_t> m_data;
	public:
		HRESULT WINAPI ReadFileFragment(void const** fragmentStart, UINT64 fileOffset, UINT64 fragmentSize, void** fragmentContext)
		{
			assert(fragmentStart);
			assert(fragmentContext);
			assert(fileOffset <= UINT32_MAX && fragmentSize <= UINT32_MAX && (fileOffset + fragmentSize) <= UINT32_MAX); // only files smaller than 4GB are supported
			if ((fileOffset + fragmentSize) > m_data.size()) return E_INVALIDARG;
			*fragmentStart = m_data.data() + fileOffset;
			*fragmentContext = m_data.data() + fileOffset; // for identification only
			return S_OK;
		}
		void WINAPI ReleaseFileFragment(void* fragmentContext)
		{
			UNREFERENCED_PARAMETER(fragmentContext);
			// no additional heap memory to free
		}
		HRESULT WINAPI GetFileSize(UINT64* fileSize)
		{
			assert(fileSize);
			*fileSize = m_data.size();
			return S_OK; // always succeed
		}
		HRESULT WINAPI GetLastWriteTime(UINT64* lastWriteTime)
		{
			UNREFERENCED_PARAMETER(lastWriteTime);
			return E_NOTIMPL; // always failed (not applicable for in-memory font files)
		}
	public:
		bool loadFromFile(std::string_view const path)
		{
			std::wstring wide_path(utility::encoding::to_wide(path)); // OOM catch by factory
			Microsoft::WRL::Wrappers::FileHandle file;
			file.Attach(CreateFileW(
				wide_path.c_str(),
				FILE_GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			));
			if (!file.IsValid())
			{
				return false;
			}
			LARGE_INTEGER size = {};
			if (!GetFileSizeEx(file.Get(), &size))
			{
				return false;
			}
			if (size.QuadPart > (LONGLONG)UINT32_MAX)
			{
				assert(false);
				return false;
			}
			m_data.resize(size.LowPart); // OOM catch by factory
			DWORD read_size = 0;
			if (!ReadFile(file.Get(), m_data.data(), size.LowPart, &read_size, NULL))
			{
				return false;
			}
			if (read_size != size.LowPart)
			{
				assert(false);
				return false;
			}
			return true;
		}
		bool loadFromFileManager(std::string_view const path)
		{
			return GFileManager().loadEx(path, m_data); // OOM catch by factory
		}
	public:
		DWriteFontFileStreamImplement() {}
		virtual ~DWriteFontFileStreamImplement() {}
	};

	class DWriteFontFileLoaderImplement : public UnknownImplement<IDWriteFontFileLoader>
	{
	private:
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<DWriteFontFileStreamImplement>> m_cache;
	public:
		HRESULT WINAPI CreateStreamFromKey(void const* fontFileReferenceKey, UINT32 fontFileReferenceKeySize, IDWriteFontFileStream** fontFileStream)
		{
			assert(fontFileReferenceKey && fontFileReferenceKeySize > 0);
			assert(fontFileStream);
			try
			{
				std::string path((char*)fontFileReferenceKey, fontFileReferenceKeySize);
				auto it = m_cache.find(path);
				if (it != m_cache.end())
				{
					it->second->AddRef();
					*fontFileStream = it->second.Get();
					return S_OK;
				}

				Microsoft::WRL::ComPtr<DWriteFontFileStreamImplement> object;
				object.Attach(new DWriteFontFileStreamImplement());
				if (!object->loadFromFileManager(path))
					return E_FAIL;

				object->AddRef();
				*fontFileStream = object.Get();

				m_cache.emplace(std::move(path), std::move(object));
				return S_OK;
			}
			catch (...)
			{
				return E_FAIL;
			}
		}
	public:
		DWriteFontFileLoaderImplement() {}
		virtual ~DWriteFontFileLoaderImplement() {}
	};

	using shared_string_list = std::shared_ptr<std::vector<std::string>>;

	class DWriteFontFileEnumeratorImplement : public UnknownImplement<IDWriteFontFileEnumerator>
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		Microsoft::WRL::ComPtr<IDWriteFontFileLoader> m_dwrite_font_file_loader;
		shared_string_list m_font_file_name_list;
		LONG m_index{};
	public:
		HRESULT WINAPI MoveNext(BOOL* hasCurrentFile)
		{
			assert(hasCurrentFile);
			assert(m_font_file_name_list);
			m_index += 1;
			if (m_index >= 0 && m_index < (LONG)m_font_file_name_list->size())
			{
				*hasCurrentFile = TRUE;
			}
			else
			{
				*hasCurrentFile = FALSE;
			}
			return S_OK;
		}
		HRESULT WINAPI GetCurrentFontFile(IDWriteFontFile** fontFile)
		{
			assert(fontFile);
			assert(m_font_file_name_list);
			assert(m_index >= 0 && m_index < (LONG)m_font_file_name_list->size());
			if (m_index < 0 || m_index >(LONG)m_font_file_name_list->size())
			{
				return E_FAIL;
			}
			std::string const& path = m_font_file_name_list->at((size_t)m_index);
			if (GFileManager().containEx(path))
			{
				return m_dwrite_factory->CreateCustomFontFileReference(
					path.data(),
					(UINT32)path.size(),
					m_dwrite_font_file_loader.Get(),
					fontFile
				);
			}
			else
			{
				std::wstring wide_path(utility::encoding::to_wide(path));
				return m_dwrite_factory->CreateFontFileReference(
					wide_path.c_str(),
					NULL,
					fontFile
				);
			}
		}
	public:
		void reset(IDWriteFactory* factory, IDWriteFontFileLoader* loader, shared_string_list list)
		{
			assert(factory);
			assert(loader);
			m_dwrite_factory = factory;
			m_dwrite_font_file_loader = loader;
			m_font_file_name_list = list; // bulk copy operations, OOM catch by factory
			m_index = -1;
		}
	public:
		DWriteFontFileEnumeratorImplement() {}
		virtual ~DWriteFontFileEnumeratorImplement() {}
	};

	class DWriteFontCollectionLoaderImplement : public UnknownImplement<IDWriteFontCollectionLoader>
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		Microsoft::WRL::ComPtr<IDWriteFontFileLoader> m_dwrite_font_file_loader;
		shared_string_list m_font_file_name_list;
	public:
		HRESULT WINAPI CreateEnumeratorFromKey(IDWriteFactory* factory, void const* collectionKey, UINT32 collectionKeySize, IDWriteFontFileEnumerator** fontFileEnumerator)
		{
			UNREFERENCED_PARAMETER(factory);
			UNREFERENCED_PARAMETER(collectionKey);
			UNREFERENCED_PARAMETER(collectionKeySize);
			assert(m_dwrite_factory);
			assert(m_dwrite_font_file_loader);
			assert(m_font_file_name_list);
			assert(collectionKey || collectionKeySize == 0);
			assert(factory);
			assert(fontFileEnumerator);
			//if (std::string_view((char*)collectionKey, collectionKeySize) != "?")
			//	return E_INVALIDARG;
			try
			{
				Microsoft::WRL::ComPtr<DWriteFontFileEnumeratorImplement> object;
				object.Attach(new DWriteFontFileEnumeratorImplement());
				object->reset(
					m_dwrite_factory.Get(),
					m_dwrite_font_file_loader.Get(),
					m_font_file_name_list
				);
				*fontFileEnumerator = object.Detach();
				return S_OK;
			}
			catch (std::exception const&)
			{
				return E_OUTOFMEMORY;
			}
		}
	public:
		void reset(IDWriteFactory* factory, IDWriteFontFileLoader* loader, shared_string_list list)
		{
			assert(factory);
			assert(loader);
			m_dwrite_factory = factory;
			m_dwrite_font_file_loader = loader;
			m_font_file_name_list = list;
		}
	public:
		DWriteFontCollectionLoaderImplement() {}
		virtual ~DWriteFontCollectionLoaderImplement() {}
	};

	static struct ModuleLoader
	{
		HMODULE dll_d2d1;
		HMODULE dll_dwrite;
		HRESULT(WINAPI* api_D2D1CreateFactory)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS*, void**);
		HRESULT(WINAPI* api_DWriteCreateFactory)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);
		void(WINAPI* api_D2D1MakeRotateMatrix)(FLOAT, D2D1_POINT_2F, D2D1_MATRIX_3X2_F*);

		ModuleLoader()
			: dll_d2d1(NULL)
			, dll_dwrite(NULL)
			, api_D2D1CreateFactory(NULL)
			, api_DWriteCreateFactory(NULL)
			, api_D2D1MakeRotateMatrix(NULL)
		{
			dll_d2d1 = LoadLibraryW(L"d2d1.dll");
			dll_dwrite = LoadLibraryW(L"dwrite.dll");
			if (dll_d2d1)
			{
				api_D2D1CreateFactory = (decltype(api_D2D1CreateFactory))GetProcAddress(dll_d2d1, "D2D1CreateFactory");
				api_D2D1MakeRotateMatrix = (decltype(api_D2D1MakeRotateMatrix))GetProcAddress(dll_d2d1, "D2D1MakeRotateMatrix");
			}
			if (dll_dwrite)
				api_DWriteCreateFactory = (decltype(api_DWriteCreateFactory))GetProcAddress(dll_dwrite, "DWriteCreateFactory");
			assert(api_D2D1CreateFactory);
			assert(api_DWriteCreateFactory);
			assert(api_D2D1MakeRotateMatrix);
		}
		~ModuleLoader()
		{
			if (dll_d2d1) FreeLibrary(dll_d2d1);
			if (dll_dwrite) FreeLibrary(dll_dwrite);
			dll_d2d1 = NULL;
			dll_dwrite = NULL;
			api_D2D1CreateFactory = NULL;
			api_DWriteCreateFactory = NULL;
			api_D2D1MakeRotateMatrix = NULL;
		}
	} DLL;

	inline D2D1::ColorF Color4BToColorF(Core::Color4B c)
	{
		return D2D1::ColorF((FLOAT)c.r / 255.0f, (FLOAT)c.g / 255.0f, (FLOAT)c.b / 255.0f, (FLOAT)c.a / 255.0f);
	}

	// DirectWrite renderer

	class DWriteTextRendererImplement : public IDWriteTextRenderer1
	{
	private:
		Microsoft::WRL::ComPtr<ID2D1Factory> d2d1_factory;
		Microsoft::WRL::ComPtr<ID2D1RenderTarget> d2d1_rt;
		Microsoft::WRL::ComPtr<IDWriteTextLayout> dwrite_text_layout;
		Microsoft::WRL::ComPtr<ID2D1Brush> d2d1_brush_outline;
		Microsoft::WRL::ComPtr<ID2D1Brush> d2d1_brush_fill;
		FLOAT outline_width;
	public:
		HRESULT WINAPI QueryInterface(IID const& riid, void** ppvObject)
		{
			if (riid == __uuidof(IUnknown))
			{
				AddRef();
				*ppvObject = static_cast<IUnknown*>(this);
				return S_OK;
			}
			else if (riid == __uuidof(IDWritePixelSnapping))
			{
				AddRef();
				*ppvObject = static_cast<IDWritePixelSnapping*>(this);
				return S_OK;
			}
			else if (riid == __uuidof(IDWriteTextRenderer))
			{
				AddRef();
				*ppvObject = static_cast<IDWriteTextRenderer*>(this);
				return S_OK;
			}
			else if (riid == __uuidof(IDWriteTextRenderer1))
			{
				AddRef();
				*ppvObject = static_cast<IDWriteTextRenderer1*>(this);
				return S_OK;
			}
			else
			{
				return E_NOINTERFACE;
			}
		}
		ULONG WINAPI AddRef() { return 2; }
		ULONG WINAPI Release() { return 1; }
	public:
		HRESULT WINAPI IsPixelSnappingDisabled(void* clientDrawingContext, BOOL* isDisabled)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			*isDisabled = FALSE; // recommended default value
			return S_OK;
		}
		HRESULT WINAPI GetCurrentTransform(void* clientDrawingContext, DWRITE_MATRIX* transform)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			// forward the render target's transform
			d2d1_rt->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
			return S_OK;
		}
		HRESULT WINAPI GetPixelsPerDip(void* clientDrawingContext, FLOAT* pixelsPerDip)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			float x = 0.0f, y = 0.0f;
			d2d1_rt->GetDpi(&x, &y);
			*pixelsPerDip = x / 96.0f;
			return S_OK;
		}
	public:
		HRESULT WINAPI DrawGlyphRun(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_MEASURING_MODE measuringMode,
			DWRITE_GLYPH_RUN const* glyphRun,
			DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(measuringMode);
			UNREFERENCED_PARAMETER(glyphRunDescription);
			UNREFERENCED_PARAMETER(clientDrawingEffect);

			HRESULT hr = S_OK;

			// Create the path geometry.

			Microsoft::WRL::ComPtr<ID2D1PathGeometry> d2d1_path_geometry;
			hr = gHR = d2d1_factory->CreatePathGeometry(&d2d1_path_geometry);
			if (FAILED(hr)) return hr;

			// Write to the path geometry using the geometry sink.

			Microsoft::WRL::ComPtr<ID2D1GeometrySink> d2d1_geometry_sink;
			hr = gHR = d2d1_path_geometry->Open(&d2d1_geometry_sink);
			if (FAILED(hr)) return hr;

			hr = gHR = glyphRun->fontFace->GetGlyphRunOutline(
				glyphRun->fontEmSize,
				glyphRun->glyphIndices,
				glyphRun->glyphAdvances,
				glyphRun->glyphOffsets,
				glyphRun->glyphCount,
				glyphRun->isSideways,
				glyphRun->bidiLevel % 2,
				d2d1_geometry_sink.Get());
			if (FAILED(hr)) return hr;

			hr = gHR = d2d1_geometry_sink->Close();
			if (FAILED(hr)) return hr;

			D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
				1.0f, 0.0f,
				0.0f, 1.0f,
				baselineOriginX, baselineOriginY
			);
			Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
			hr = gHR = d2d1_factory->CreateTransformedGeometry(
				d2d1_path_geometry.Get(),
				&matrix,
				&d2d1_transformed_geometry);
			if (FAILED(hr)) return hr;

			// Draw the outline of the glyph run

			d2d1_rt->DrawGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_outline.Get(), outline_width);

			// Fill in the glyph run

			d2d1_rt->FillGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_fill.Get());

			return S_OK;
		}
		HRESULT WINAPI DrawUnderline(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_UNDERLINE const* underline,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(clientDrawingEffect);

			HRESULT hr = S_OK;

			D2D1_RECT_F rect = D2D1::RectF(
				0,
				underline->offset,
				underline->width,
				underline->offset + underline->thickness
			);

			Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> d2d1_rect_geometry;
			hr = gHR = d2d1_factory->CreateRectangleGeometry(&rect, &d2d1_rect_geometry);
			if (FAILED(hr)) return hr;

			D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
				1.0f, 0.0f,
				0.0f, 1.0f,
				baselineOriginX, baselineOriginY
			);

			Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
			hr = gHR = d2d1_factory->CreateTransformedGeometry(
				d2d1_rect_geometry.Get(),
				&matrix,
				&d2d1_transformed_geometry);
			if (FAILED(hr)) return hr;

			d2d1_rt->DrawGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_outline.Get(), outline_width);
			d2d1_rt->FillGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_fill.Get());

			return S_OK;
		}
		HRESULT WINAPI DrawStrikethrough(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_STRIKETHROUGH const* strikethrough,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(clientDrawingEffect);

			HRESULT hr = S_OK;

			D2D1_RECT_F rect = D2D1::RectF(
				0,
				strikethrough->offset,
				strikethrough->width,
				strikethrough->offset + strikethrough->thickness
			);

			Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> d2d1_rect_geometry;
			hr = gHR = d2d1_factory->CreateRectangleGeometry(&rect, &d2d1_rect_geometry);
			if (FAILED(hr)) return hr;

			D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
				1.0f, 0.0f,
				0.0f, 1.0f,
				baselineOriginX, baselineOriginY
			);

			Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
			hr = gHR = d2d1_factory->CreateTransformedGeometry(
				d2d1_rect_geometry.Get(),
				&matrix,
				&d2d1_transformed_geometry);
			if (FAILED(hr)) return hr;

			d2d1_rt->DrawGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_outline.Get(), outline_width);
			d2d1_rt->FillGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_fill.Get());

			return S_OK;
		}
		HRESULT WINAPI DrawInlineObject(
			void* clientDrawingContext,
			FLOAT originX,
			FLOAT originY,
			IDWriteInlineObject* inlineObject,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(originX);
			UNREFERENCED_PARAMETER(originY);
			UNREFERENCED_PARAMETER(inlineObject);
			UNREFERENCED_PARAMETER(isSideways);
			UNREFERENCED_PARAMETER(isRightToLeft);
			UNREFERENCED_PARAMETER(clientDrawingEffect);
			return E_NOTIMPL;
		}
	public:
		HRESULT WINAPI DrawGlyphRun(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
			DWRITE_MEASURING_MODE measuringMode,
			DWRITE_GLYPH_RUN const* glyphRun,
			DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(measuringMode);
			UNREFERENCED_PARAMETER(glyphRunDescription);
			UNREFERENCED_PARAMETER(clientDrawingEffect);

			HRESULT hr = S_OK;

			// Create the path geometry.

			Microsoft::WRL::ComPtr<ID2D1PathGeometry> d2d1_path_geometry;
			hr = gHR = d2d1_factory->CreatePathGeometry(&d2d1_path_geometry);
			if (FAILED(hr)) return hr;

			// Write to the path geometry using the geometry sink.

			Microsoft::WRL::ComPtr<ID2D1GeometrySink> d2d1_geometry_sink;
			hr = gHR = d2d1_path_geometry->Open(&d2d1_geometry_sink);
			if (FAILED(hr)) return hr;

			hr = gHR = glyphRun->fontFace->GetGlyphRunOutline(
				glyphRun->fontEmSize,
				glyphRun->glyphIndices,
				glyphRun->glyphAdvances,
				glyphRun->glyphOffsets,
				glyphRun->glyphCount,
				glyphRun->isSideways,
				glyphRun->bidiLevel % 2,
				d2d1_geometry_sink.Get());
			if (FAILED(hr)) return hr;

			hr = gHR = d2d1_geometry_sink->Close();
			if (FAILED(hr)) return hr;

			// TODO: 为什么旋转方向是这样判断的？
			FLOAT rotate_angle = 0.0f;
			//switch (orientationAngle)
			//{
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES: rotate_angle = 0.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES: rotate_angle = 90.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: rotate_angle = 180.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: rotate_angle = 270.0f; break;
			//default: assert(false); break;
			//}
			UNREFERENCED_PARAMETER(orientationAngle);
			switch (dwrite_text_layout->GetReadingDirection())
			{
			case DWRITE_READING_DIRECTION_LEFT_TO_RIGHT: rotate_angle = 0.0f; break;
			//case DWRITE_READING_DIRECTION_RIGHT_TO_LEFT: rotate_angle = 0.0f; break;
			case DWRITE_READING_DIRECTION_TOP_TO_BOTTOM: rotate_angle = 90.0f; break;
			//case DWRITE_READING_DIRECTION_BOTTOM_TO_TOP: rotate_angle = 90.0f; break;
			default: assert(false); break;
			}
			D2D1::Matrix3x2F matrix;
			DLL.api_D2D1MakeRotateMatrix(rotate_angle, D2D1::Point2F(), &matrix);
			matrix.dx = baselineOriginX;
			matrix.dy = baselineOriginY;
			Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
			hr = gHR = d2d1_factory->CreateTransformedGeometry(
				d2d1_path_geometry.Get(),
				&matrix,
				&d2d1_transformed_geometry);
			if (FAILED(hr)) return hr;

			// Draw the outline of the glyph run

			d2d1_rt->DrawGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_outline.Get(), outline_width);

			// Fill in the glyph run

			d2d1_rt->FillGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_fill.Get());

			return S_OK;
		}
		HRESULT WINAPI DrawUnderline(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
			DWRITE_UNDERLINE const* underline,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(clientDrawingEffect);

			HRESULT hr = S_OK;

			D2D1_RECT_F rect = D2D1::RectF(
				0,
				underline->offset,
				underline->width,
				underline->offset + underline->thickness
			);

			Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> d2d1_rect_geometry;
			hr = gHR = d2d1_factory->CreateRectangleGeometry(&rect, &d2d1_rect_geometry);
			if (FAILED(hr)) return hr;

			// TODO: 为什么旋转方向是这样判断的？
			FLOAT rotate_angle = 0.0f;
			//switch (orientationAngle)
			//{
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES: rotate_angle = 0.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES: rotate_angle = 90.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: rotate_angle = 180.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: rotate_angle = 270.0f; break;
			//default: assert(false); break;
			//}
			UNREFERENCED_PARAMETER(orientationAngle);
			switch (dwrite_text_layout->GetReadingDirection())
			{
			case DWRITE_READING_DIRECTION_LEFT_TO_RIGHT: rotate_angle = 0.0f; break;
				//case DWRITE_READING_DIRECTION_RIGHT_TO_LEFT: rotate_angle = 0.0f; break;
			case DWRITE_READING_DIRECTION_TOP_TO_BOTTOM: rotate_angle = 90.0f; break;
				//case DWRITE_READING_DIRECTION_BOTTOM_TO_TOP: rotate_angle = 90.0f; break;
			default: assert(false); break;
			}
			D2D1::Matrix3x2F matrix;
			DLL.api_D2D1MakeRotateMatrix(rotate_angle, D2D1::Point2F(), &matrix);
			matrix.dx = baselineOriginX;
			matrix.dy = baselineOriginY;
			Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
			hr = gHR = d2d1_factory->CreateTransformedGeometry(
				d2d1_rect_geometry.Get(),
				&matrix,
				&d2d1_transformed_geometry);
			if (FAILED(hr)) return hr;

			d2d1_rt->DrawGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_outline.Get(), outline_width);
			d2d1_rt->FillGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_fill.Get());

			return S_OK;
		}
		HRESULT WINAPI DrawStrikethrough(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
			DWRITE_STRIKETHROUGH const* strikethrough,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(clientDrawingEffect);

			HRESULT hr = S_OK;

			D2D1_RECT_F rect = D2D1::RectF(
				0,
				strikethrough->offset,
				strikethrough->width,
				strikethrough->offset + strikethrough->thickness
			);

			Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> d2d1_rect_geometry;
			hr = gHR = d2d1_factory->CreateRectangleGeometry(&rect, &d2d1_rect_geometry);
			if (FAILED(hr)) return hr;

			// TODO: 为什么旋转方向是这样判断的？
			FLOAT rotate_angle = 0.0f;
			//switch (orientationAngle)
			//{
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES: rotate_angle = 0.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES: rotate_angle = 90.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: rotate_angle = 180.0f; break;
			//case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: rotate_angle = 270.0f; break;
			//default: assert(false); break;
			//}
			UNREFERENCED_PARAMETER(orientationAngle);
			switch (dwrite_text_layout->GetReadingDirection())
			{
			case DWRITE_READING_DIRECTION_LEFT_TO_RIGHT: rotate_angle = 0.0f; break;
				//case DWRITE_READING_DIRECTION_RIGHT_TO_LEFT: rotate_angle = 0.0f; break;
			case DWRITE_READING_DIRECTION_TOP_TO_BOTTOM: rotate_angle = 90.0f; break;
				//case DWRITE_READING_DIRECTION_BOTTOM_TO_TOP: rotate_angle = 90.0f; break;
			default: assert(false); break;
			}
			D2D1::Matrix3x2F matrix;
			DLL.api_D2D1MakeRotateMatrix(rotate_angle, D2D1::Point2F(), &matrix);
			matrix.dx = baselineOriginX;
			matrix.dy = baselineOriginY;
			Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
			hr = gHR = d2d1_factory->CreateTransformedGeometry(
				d2d1_rect_geometry.Get(),
				&matrix,
				&d2d1_transformed_geometry);
			if (FAILED(hr)) return hr;

			d2d1_rt->DrawGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_outline.Get(), outline_width);
			d2d1_rt->FillGeometry(d2d1_transformed_geometry.Get(), d2d1_brush_fill.Get());

			return S_OK;
		}
		HRESULT WINAPI DrawInlineObject(
			void* clientDrawingContext,
			FLOAT originX,
			FLOAT originY,
			DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
			IDWriteInlineObject* inlineObject,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown* clientDrawingEffect)
		{
			UNREFERENCED_PARAMETER(clientDrawingContext);
			UNREFERENCED_PARAMETER(originX);
			UNREFERENCED_PARAMETER(originY);
			UNREFERENCED_PARAMETER(orientationAngle);
			UNREFERENCED_PARAMETER(inlineObject);
			UNREFERENCED_PARAMETER(isSideways);
			UNREFERENCED_PARAMETER(isRightToLeft);
			UNREFERENCED_PARAMETER(clientDrawingEffect);
			return E_NOTIMPL;
		}
	public:
		DWriteTextRendererImplement(
			ID2D1Factory* factory,
			ID2D1RenderTarget* target,
			IDWriteTextLayout* layout,
			ID2D1Brush* outline,
			ID2D1Brush* fill,
			FLOAT width)
			: d2d1_factory(factory)
			, d2d1_rt(target)
			, dwrite_text_layout(layout)
			, d2d1_brush_outline(outline)
			, d2d1_brush_fill(fill)
			, outline_width(width) {}
		~DWriteTextRendererImplement() {}
	};

	// lib

	static int LUA_KEY = 0;

	struct FontCollection
	{
		static std::string_view const ClassID;

		std::string name;
		Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_factory;
		Microsoft::WRL::ComPtr<DWriteFontFileLoaderImplement> dwrite_font_file_loader; // from core
		Microsoft::WRL::ComPtr<DWriteFontCollectionLoaderImplement> dwrite_font_collection_loader;
		Microsoft::WRL::ComPtr<IDWriteFontCollection> dwrite_font_collection;
		shared_string_list font_file_name_list;

		void _test()
		{
		}

		bool InitComponents()
		{
			HRESULT hr = S_OK;

			std::stringstream ss;
			ss << this;
			name = ss.str();

			dwrite_font_collection_loader.Attach(new DWriteFontCollectionLoaderImplement());
			dwrite_font_collection_loader->reset(
				dwrite_factory.Get(),
				dwrite_font_file_loader.Get(),
				font_file_name_list);

			hr = gHR = dwrite_factory->RegisterFontCollectionLoader(dwrite_font_collection_loader.Get());
			if (FAILED(hr)) return false;

			hr = gHR = dwrite_factory->CreateCustomFontCollection(
				dwrite_font_collection_loader.Get(),
				name.data(),
				(UINT32)name.size(),
				&dwrite_font_collection);
			if (FAILED(hr)) return false;

			return true;
		}

		FontCollection() {}
		~FontCollection()
		{
			if (dwrite_factory && dwrite_font_collection_loader)
			{
				gHR = dwrite_factory->UnregisterFontCollectionLoader(dwrite_font_collection_loader.Get());
			}
		}

		static int api_GetDebugInformation(lua_State* L)
		{
			FontCollection* self = Cast(L, 1);
			std::stringstream ss;
			printFontCollectionInfo(self->dwrite_font_collection.Get(), ss);
			std::string buf(ss.str());
			lua_push_string_view(L, buf);
			return 1;
		}

		static int api___tostring(lua_State* L)
		{
			Cast(L, 1);
			lua_pushlstring(L, ClassID.data(), ClassID.size());
			return 1;
		}
		static int api___gc(lua_State* L)
		{
			FontCollection* self = Cast(L, 1);
			self->~FontCollection();
			return 0;
		}

		static FontCollection* Cast(lua_State* L, int idx)
		{
			return (FontCollection*)luaL_checkudata(L, idx, ClassID.data());
		}
		static FontCollection* Create(lua_State* L)
		{
			FontCollection* self = (FontCollection*)lua_newuserdata(L, sizeof(FontCollection));
			new(self) FontCollection();
			luaL_getmetatable(L, ClassID.data()); // ??? udata mt
			lua_setmetatable(L, -2);              // ??? udata
			return self;
		}
		static void Register(lua_State* L)
		{
			luaL_Reg const mt[] = {
				{ "__tostring", &api___tostring },
				{ "__gc", &api___gc },
				{ NULL, NULL },
			};
			luaL_Reg const lib[] = {
				{ "GetDebugInformation", &api_GetDebugInformation },
				{ NULL, NULL },
			};
			luaL_newmetatable(L, ClassID.data()); // mt
			luaL_register(L, NULL, mt);
			lua_createtable(L, 0, 1);             // mt lib
			luaL_register(L, NULL, lib);
			lua_setfield(L, -2, "__index");       // mt
			lua_pop(L, 1);
		}
	};
	std::string_view const FontCollection::ClassID("DirectWrite.FontCollection");

	struct TextFormat
	{
		static std::string_view const ClassID;

		Microsoft::WRL::ComPtr<IDWriteTextFormat> dwrite_text_format;

		void _test()
		{
		}

		TextFormat() {}
		~TextFormat() {}

		static int api___tostring(lua_State* L)
		{
			Cast(L, 1);
			lua_pushlstring(L, ClassID.data(), ClassID.size());
			return 1;
		}
		static int api___gc(lua_State* L)
		{
			TextFormat* self = Cast(L, 1);
			self->~TextFormat();
			return 0;
		}

		static TextFormat* Cast(lua_State* L, int idx)
		{
			return (TextFormat*)luaL_checkudata(L, idx, ClassID.data());
		}
		static TextFormat* Create(lua_State* L)
		{
			TextFormat* self = (TextFormat*)lua_newuserdata(L, sizeof(TextFormat));
			new(self) TextFormat();
			luaL_getmetatable(L, ClassID.data()); // ??? udata mt
			lua_setmetatable(L, -2);              // ??? udata
			return self;
		}
		static void Register(lua_State* L)
		{
			luaL_Reg const mt[] = {
				{ "__tostring", &api___tostring },
				{ "__gc", &api___gc },
				{ NULL, NULL },
			};
			luaL_newmetatable(L, ClassID.data());
			luaL_register(L, NULL, mt);
			lua_pop(L, 1);
		}
	};
	std::string_view const TextFormat::ClassID("DirectWrite.TextFormat");

	struct TextLayout
	{
		static std::string_view const ClassID;

		Microsoft::WRL::ComPtr<IDWriteTextLayout> dwrite_text_layout;
		std::wstring text;

		void _test()
		{
			
		}

		TextLayout() {}
		~TextLayout() {}

		static int api_SetFontCollection(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto* font_collection = FontCollection::Cast(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetFontCollection(
				font_collection->dwrite_font_collection.Get(),
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetFontCollection failed");

			return 0;
		}
		static int api_SetFontFamilyName(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const font_family_name = luaL_check_string_view(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			std::wstring wide_font_family_name(utility::encoding::to_wide(font_family_name));

			HRESULT hr = gHR = self->dwrite_text_layout->SetFontFamilyName(
				wide_font_family_name.c_str(),
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetFontFamilyName failed");

			return 0;
		}
		static int api_SetLocaleName(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const locale_name = luaL_check_string_view(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			std::wstring wide_locale_name(utility::encoding::to_wide(locale_name));

			HRESULT hr = gHR = self->dwrite_text_layout->SetLocaleName(
				wide_locale_name.c_str(),
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetLocaleName failed");

			return 0;
		}
		static int api_SetFontSize(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const font_size = luaL_check_float(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetFontSize(
				font_size,
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetFontSize failed");

			return 0;
		}
		static int api_SetFontStyle(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const font_style = luaL_check_C_enum<DWRITE_FONT_STYLE>(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetFontStyle(
				font_style,
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetFontStyle failed");

			return 0;
		}
		static int api_SetFontWeight(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const font_weight = luaL_check_C_enum<DWRITE_FONT_WEIGHT>(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetFontWeight(
				font_weight,
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetFontWeight failed");

			return 0;
		}
		static int api_SetFontStretch(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const font_stretch = luaL_check_C_enum<DWRITE_FONT_STRETCH>(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetFontStretch(
				font_stretch,
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetFontStretch failed");

			return 0;
		}

		static int api_SetStrikethrough(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const enable = lua_to_boolean(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetStrikethrough(
				enable,
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetStrikethrough failed");

			return 0;
		}
		static int api_SetUnderline(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const enable = lua_to_boolean(L, 2);
			auto const position = luaL_check_uint32(L, 3);
			auto const length = luaL_check_uint32(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetUnderline(
				enable,
				DWRITE_TEXT_RANGE{
					.startPosition = position,
					.length = length,
				});
			if (FAILED(hr))
				return luaL_error(L, "SetUnderline failed");

			return 0;
		}

		static int api_SetIncrementalTabStop(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const tab_size = luaL_check_float(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetIncrementalTabStop(tab_size);
			if (FAILED(hr))
				return luaL_error(L, "SetIncrementalTabStop failed");

			return 0;
		}
		static int api_SetLineSpacing(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const line_spacing_method = luaL_check_C_enum<DWRITE_LINE_SPACING_METHOD>(L, 2);
			auto const line_spacing = luaL_check_float(L, 3);
			auto const baseline = luaL_check_float(L, 4);

			HRESULT hr = gHR = self->dwrite_text_layout->SetLineSpacing(line_spacing_method, line_spacing, baseline);
			if (FAILED(hr))
				return luaL_error(L, "SetLineSpacing failed");

			return 0;
		}
		static int api_SetTextAlignment(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const align = luaL_check_C_enum<DWRITE_TEXT_ALIGNMENT>(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetTextAlignment(align);
			if (FAILED(hr))
				return luaL_error(L, "SetTextAlignment failed");

			return 0;
		}
		static int api_SetParagraphAlignment(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const align = luaL_check_C_enum<DWRITE_PARAGRAPH_ALIGNMENT>(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetParagraphAlignment(align);
			if (FAILED(hr))
				return luaL_error(L, "SetParagraphAlignment failed");

			return 0;
		}
		static int api_SetFlowDirection(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const dir = luaL_check_C_enum<DWRITE_FLOW_DIRECTION>(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetFlowDirection(dir);
			if (FAILED(hr))
				return luaL_error(L, "SetFlowDirection failed");

			return 0;
		}
		static int api_SetReadingDirection(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const dir = luaL_check_C_enum<DWRITE_READING_DIRECTION>(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetReadingDirection(dir);
			if (FAILED(hr))
				return luaL_error(L, "SetReadingDirection failed");

			return 0;
		}
		static int api_SetWordWrapping(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const wrap = luaL_check_C_enum<DWRITE_WORD_WRAPPING>(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetWordWrapping(wrap);
			if (FAILED(hr))
				return luaL_error(L, "SetWordWrapping failed");

			return 0;
		}

		static int api_SetMaxWidth(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const max_width = luaL_check_float(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetMaxWidth(max_width);
			if (FAILED(hr))
				return luaL_error(L, "SetMaxWidth failed");

			return 0;
		}
		static int api_SetMaxHeight(lua_State* L)
		{
			auto* self = Cast(L, 1);
			auto const max_height = luaL_check_float(L, 2);

			HRESULT hr = gHR = self->dwrite_text_layout->SetMaxHeight(max_height);
			if (FAILED(hr))
				return luaL_error(L, "SetMaxHeight failed");

			return 0;
		}

		static int api_DetermineMinWidth(lua_State* L)
		{
			auto* self = Cast(L, 1);
			FLOAT min_width = 0.0f;

			HRESULT hr = gHR = self->dwrite_text_layout->DetermineMinWidth(&min_width);
			if (FAILED(hr))
				return luaL_error(L, "DetermineMinWidth failed");

			lua_push_float(L, min_width);
			return 1;
		}

		static int api___tostring(lua_State* L)
		{
			Cast(L, 1);
			lua_pushlstring(L, ClassID.data(), ClassID.size());
			return 1;
		}
		static int api___gc(lua_State* L)
		{
			TextLayout* self = Cast(L, 1);
			self->~TextLayout();
			return 0;
		}

		static TextLayout* Cast(lua_State* L, int idx)
		{
			return (TextLayout*)luaL_checkudata(L, idx, ClassID.data());
		}
		static TextLayout* Create(lua_State* L)
		{
			TextLayout* self = (TextLayout*)lua_newuserdata(L, sizeof(TextLayout));
			new(self) TextLayout();
			luaL_getmetatable(L, ClassID.data()); // ??? udata mt
			lua_setmetatable(L, -2);              // ??? udata
			return self;
		}
		static void Register(lua_State* L)
		{
			luaL_Reg const mt[] = {
				{ "__tostring", &api___tostring },
				{ "__gc", &api___gc },
				{ NULL, NULL },
			};
			luaL_Reg const lib[] = {
				{ "SetFontCollection", &api_SetFontCollection },
				{ "SetFontFamilyName", &api_SetFontFamilyName },
				{ "SetLocaleName", &api_SetLocaleName },
				{ "SetFontSize", &api_SetFontSize },
				{ "SetFontStyle", &api_SetFontStyle },
				{ "SetFontWeight", &api_SetFontWeight },
				{ "SetFontStretch", &api_SetFontStretch },

				{ "SetStrikethrough", &api_SetStrikethrough },
				{ "SetUnderline", &api_SetUnderline },

				{ "SetIncrementalTabStop", &api_SetIncrementalTabStop },
				{ "SetLineSpacing", &api_SetLineSpacing },
				{ "SetTextAlignment", &api_SetTextAlignment },
				{ "SetParagraphAlignment", &api_SetParagraphAlignment },
				{ "SetFlowDirection", &api_SetFlowDirection },
				{ "SetReadingDirection", &api_SetReadingDirection },
				{ "SetWordWrapping", &api_SetWordWrapping },

				{ "SetMaxWidth", &api_SetMaxWidth },
				{ "SetMaxHeight", &api_SetMaxHeight },

				{ "DetermineMinWidth", &api_DetermineMinWidth },

				{ NULL, NULL },
			};
			luaL_newmetatable(L, ClassID.data()); // mt
			luaL_register(L, NULL, mt);
			lua_createtable(L, 0, 1);             // mt lib
			luaL_register(L, NULL, lib);
			lua_setfield(L, -2, "__index");       // mt
			lua_pop(L, 1);
		}
	};
	std::string_view const TextLayout::ClassID("DirectWrite.TextLayout");

	struct Factory
	{
		Microsoft::WRL::ComPtr<IWICImagingFactory> wic_factory;
		Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_factory;
		Microsoft::WRL::ComPtr<ID2D1Factory> d2d1_factory;
		Microsoft::WRL::ComPtr<DWriteFontFileLoaderImplement> dwrite_font_file_loader;

		void _test()
		{
			
		}

		bool InitComponents()
		{
			HRESULT hr = S_OK;

			hr = gHR = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&wic_factory)
			);
			if (FAILED(hr))
				return false;

			hr = gHR = DLL.api_DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				&dwrite_factory
			);
			if (FAILED(hr))
				return false;

			D2D1_FACTORY_OPTIONS d2d1_options = {
			#ifdef _DEBUG
				.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION,
			#else
				.debugLevel = D2D1_DEBUG_LEVEL_NONE,
			#endif
			};
			hr = gHR = DLL.api_D2D1CreateFactory(
				D2D1_FACTORY_TYPE_SINGLE_THREADED,
				__uuidof(ID2D1Factory),
				&d2d1_options,
				&d2d1_factory
			);
			if (FAILED(hr))
				return false;

			dwrite_font_file_loader.Attach(new DWriteFontFileLoaderImplement());
			hr = gHR = dwrite_factory->RegisterFontFileLoader(dwrite_font_file_loader.Get());
			if (FAILED(hr))
				return false;

			return true;
		}

		Factory() {}
		~Factory()
		{
			if (dwrite_factory && dwrite_font_file_loader)
			{
				gHR = dwrite_factory->UnregisterFontFileLoader(dwrite_font_file_loader.Get());
			}
		}

		static std::string_view const ClassID;

		static int api___tostring(lua_State* L)
		{
			Cast(L, 1);
			lua_pushlstring(L, ClassID.data(), ClassID.size());
			return 1;
		}
		static int api___gc(lua_State* L)
		{
			Factory* self = Cast(L, 1);
			self->~Factory();
			return 0;
		}

		static Factory* Cast(lua_State* L, int idx)
		{
			return (Factory*)luaL_checkudata(L, idx, ClassID.data());
		}
		static Factory* Create(lua_State* L)
		{
			Factory* self = (Factory*)lua_newuserdata(L, sizeof(Factory));
			new(self) Factory();
			luaL_getmetatable(L, ClassID.data()); // ??? udata mt
			lua_setmetatable(L, -2);              // ??? udata
			return self;
		}
		static void Register(lua_State* L)
		{
			luaL_Reg const mt[] = {
				{ "__tostring", &api___tostring },
				{ "__gc", &api___gc },
				{ NULL, NULL },
			};
			luaL_newmetatable(L, ClassID.data());
			luaL_register(L, NULL, mt);
			lua_pop(L, 1);
		}

		static Factory* Get(lua_State* L)
		{
			lua_pushlightuserdata(L, &LUA_KEY);
			lua_gettable(L, LUA_REGISTRYINDEX);
			Factory* core = Cast(L, -1);
			lua_pop(L, 1);
			return core;
		}
	};
	std::string_view const Factory::ClassID("DirectWrite.Factory");

	static int api_CreateFontCollection(lua_State* L)
	{
		luaL_argcheck(L, lua_istable(L, 1), 1, "");

		Factory* core = Factory::Get(L);
		FontCollection* font_collection = FontCollection::Create(L);

		font_collection->dwrite_factory = core->dwrite_factory;
		font_collection->dwrite_font_file_loader = core->dwrite_font_file_loader;
		font_collection->dwrite_font_collection_loader.Attach(new DWriteFontCollectionLoaderImplement());
		font_collection->font_file_name_list = std::make_shared<std::vector<std::string>>();
		size_t const file_count = lua_objlen(L, 1);
		font_collection->font_file_name_list->reserve(file_count);
		for (size_t i = 0; i < file_count; i += 1)
		{
			lua_rawgeti(L, 1, (int)i + 1);
			auto const file_path = luaL_check_string_view(L, -1);
			font_collection->font_file_name_list->emplace_back(file_path);
			lua_pop(L, 1);
		}
		if (!font_collection->InitComponents())
			return luaL_error(L, "[DirectWrite.CreateFontCollection] init failed");

		return 1;
	}
	static int api_CreateTextFormat(lua_State* L)
	{
		auto const font_family_name = luaL_check_string_view(L, 1);
		FontCollection* font_collection{}; if (lua_isuserdata(L, 2)) font_collection = FontCollection::Cast(L, 2);
		auto const font_weight = luaL_check_C_enum<DWRITE_FONT_WEIGHT>(L, 3);
		auto const font_style = luaL_check_C_enum<DWRITE_FONT_STYLE>(L, 4);
		auto const font_stretch = luaL_check_C_enum<DWRITE_FONT_STRETCH>(L, 5);
		auto const font_size = luaL_check_float(L, 6);
		auto const locale_name = luaL_check_string_view(L, 7);

		std::wstring wide_font_family_name(utility::encoding::to_wide(font_family_name));
		std::wstring wide_locale_name(utility::encoding::to_wide(locale_name));

		Factory* core = Factory::Get(L);
		TextFormat* text_format = TextFormat::Create(L);

		HRESULT hr = gHR = core->dwrite_factory->CreateTextFormat(
			wide_font_family_name.c_str(),
			font_collection ? font_collection->dwrite_font_collection.Get() : NULL,
			font_weight,
			font_style,
			font_stretch,
			font_size,
			wide_locale_name.c_str(),
			&text_format->dwrite_text_format);
		if (FAILED(hr))
		{
			return luaL_error(L, "[DirectWrite.CreateTextFormat] IDWriteFactory::CreateTextFormat failed");
		}

		return 1;
	}
	static int api_CreateTextLayout(lua_State* L)
	{
		auto const string = luaL_check_string_view(L, 1);
		auto* text_format = TextFormat::Cast(L, 2);
		auto const max_width = luaL_check_float(L, 3);
		auto const max_height = luaL_check_float(L, 4);

		std::wstring wide_string(utility::encoding::to_wide(string));

		Factory* core = Factory::Get(L);
		TextLayout* text_layout = TextLayout::Create(L);

		HRESULT hr = gHR = core->dwrite_factory->CreateTextLayout(
			wide_string.data(),
			(UINT32)wide_string.size(),
			text_format->dwrite_text_format.Get(),
			max_width,
			max_height,
			&text_layout->dwrite_text_layout);
		if (FAILED(hr))
		{
			return luaL_error(L, "[DirectWrite.CreateTextLayout] IDWriteFactory::CreateTextLayout failed");
		}

		text_layout->text = std::move(wide_string);

		return 1;
	}

	static int api_CreateTextureFromTextLayout(lua_State* L)
	{
		HRESULT hr = S_OK;

		Factory* core = Factory::Get(L);
		auto* text_layout = TextLayout::Cast(L, 1);
		auto const pool_type = luaL_check_string_view(L, 2);
		auto const texture_name = luaL_check_string_view(L, 3);
		auto const outline_width = luaL_optional_float(L, 4, 0.0f);
		Core::Color4B font_color = Core::Color4B(255, 255, 255, 255);
		Core::Color4B outline_color = Core::Color4B(0, 0, 0, 255);
		if (lua_gettop(L) >= 5)
			font_color = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(L, 5);
		if (lua_gettop(L) >= 6)
			outline_color = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(L, 6);

		// pre check

		LuaSTGPlus::ResourcePool* pool{};
		if (pool_type == "global")
			pool = LRES.GetResourcePool(LuaSTGPlus::ResourcePoolType::Global);
		else if (pool_type == "stage")
			pool = LRES.GetResourcePool(LuaSTGPlus::ResourcePoolType::Stage);
		else
			return luaL_error(L, "invalid resource pool type");
		if (pool->GetTexture(texture_name.data()))
			return luaL_error(L, "texture '%s' already exists", texture_name.data());

		// bitmap

		//auto const texture_width = std::ceil(text_layout->dwrite_text_layout->GetMaxWidth());
		//auto const texture_height = std::ceil(text_layout->dwrite_text_layout->GetMaxHeight());
		auto const texture_canvas_width = std::ceil(text_layout->dwrite_text_layout->GetMaxWidth() + 2.0f * outline_width);
		auto const texture_canvas_height = std::ceil(text_layout->dwrite_text_layout->GetMaxHeight() + 2.0f * outline_width);

		Microsoft::WRL::ComPtr<IWICBitmap> wic_bitmap;
		hr = gHR = core->wic_factory->CreateBitmap(
			(UINT)texture_canvas_width,
			(UINT)texture_canvas_height,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnDemand,
			&wic_bitmap);
		if (FAILED(hr))
			return luaL_error(L, "create bitmap failed");

		// d2d1 rasterizer

		Microsoft::WRL::ComPtr<ID2D1RenderTarget> d2d1_rt;
		hr = gHR = core->d2d1_factory->CreateWicBitmapRenderTarget(
			wic_bitmap.Get(),
			D2D1::RenderTargetProperties(),
			&d2d1_rt);
		if (FAILED(hr))
			return luaL_error(L, "create rasterizer failed");

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d1_pen;
		hr = gHR = d2d1_rt->CreateSolidColorBrush(Color4BToColorF(font_color), &d2d1_pen);
		if (FAILED(hr))
			return luaL_error(L, "create rasterizer color failed");

		// rasterize

		if (lua_gettop(L) >= 4)
		{
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d1_pen2;
			hr = gHR = d2d1_rt->CreateSolidColorBrush(Color4BToColorF(outline_color), &d2d1_pen2);
			if (FAILED(hr))
				return luaL_error(L, "create rasterizer color failed");

			DWriteTextRendererImplement renderer(
				core->d2d1_factory.Get(),
				d2d1_rt.Get(),
				text_layout->dwrite_text_layout.Get(),
				d2d1_pen2.Get(),
				d2d1_pen.Get(),
				outline_width);

			d2d1_rt->BeginDraw();
			d2d1_rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
			hr = gHR = text_layout->dwrite_text_layout->Draw(NULL, &renderer, outline_width, outline_width);
			if (FAILED(hr))
				return luaL_error(L, "render failed");
			hr = gHR = d2d1_rt->EndDraw();
			if (FAILED(hr))
				return luaL_error(L, "rasterize failed");
		}
		else
		{
			d2d1_rt->BeginDraw();
			d2d1_rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
			d2d1_rt->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), text_layout->dwrite_text_layout.Get(), d2d1_pen.Get());
			if (FAILED(hr))
				return luaL_error(L, "render failed");
			hr = gHR = d2d1_rt->EndDraw();
			if (FAILED(hr))
				return luaL_error(L, "rasterize failed");
		}

		// lock result

		WICRect lock_rect = {
			.X = 0,
			.Y = 0,
			.Width = (INT)texture_canvas_width,
			.Height = (INT)texture_canvas_height,
		};
		Microsoft::WRL::ComPtr<IWICBitmapLock> wic_bitmap_lock;
		hr = gHR = wic_bitmap->Lock(&lock_rect, WICBitmapLockRead, &wic_bitmap_lock);
		if (FAILED(hr)) return luaL_error(L, "read rasterize result failed");
		UINT buffer_size = 0;
		WICInProcPointer buffer = NULL;
		hr = gHR = wic_bitmap_lock->GetDataPointer(&buffer_size, &buffer);
		if (FAILED(hr)) return luaL_error(L, "read rasterize result failed");
		UINT buffer_stride = 0;
		hr = gHR = wic_bitmap_lock->GetStride(&buffer_stride);
		if (FAILED(hr)) return luaL_error(L, "read rasterize result failed");

		// create texture

		if (!pool->CreateTexture(texture_name.data(), (int)texture_canvas_width, (int)texture_canvas_height))
			return luaL_error(L, "create texture failed");

		auto p_texres = pool->GetTexture(texture_name.data());
		auto* p_texture = p_texres->GetTexture();

		// upload data

		p_texture->setPremultipliedAlpha(true);
		if (!p_texture->uploadPixelData(
			Core::RectU(0, 0, (uint32_t)texture_canvas_width, (uint32_t)texture_canvas_height),
			buffer, buffer_stride))
			return luaL_error(L, "upload texture data failed");

		// copy and store pixel data

		Core::ScopeObject<Core::IData> p_pixel_data;
		if (!Core::IData::create(4 * (uint32_t)texture_canvas_width * (uint32_t)texture_canvas_height, ~p_pixel_data))
			return luaL_error(L, "copy texture data failed");
		uint8_t* dst_ptr = (uint8_t*)p_pixel_data->data();
		uint8_t* src_ptr = buffer;
		for (uint32_t y = 0; y < (uint32_t)texture_canvas_height; y += 1)
		{
			std::memcpy(dst_ptr, src_ptr, 4 * (uint32_t)texture_canvas_width);
			src_ptr += buffer_stride;
			dst_ptr += 4 * (uint32_t)texture_canvas_width;
		}
		p_texture->setPixelData(p_pixel_data.get());

		return 0;
	}
	static int api_SaveTextLayoutToFile(lua_State* L)
	{
		HRESULT hr = S_OK;

		Factory* core = Factory::Get(L);
		auto* text_layout = TextLayout::Cast(L, 1);
		auto const file_path = luaL_check_string_view(L, 2);
		auto const outline_width = luaL_optional_float(L, 3, 0.0f);

		// bitmap

		//auto const texture_width = std::ceil(text_layout->dwrite_text_layout->GetMaxWidth());
		//auto const texture_height = std::ceil(text_layout->dwrite_text_layout->GetMaxHeight());
		auto const texture_canvas_width = std::ceil(text_layout->dwrite_text_layout->GetMaxWidth() + 2.0f * outline_width);
		auto const texture_canvas_height = std::ceil(text_layout->dwrite_text_layout->GetMaxHeight() + 2.0f * outline_width);

		Microsoft::WRL::ComPtr<IWICBitmap> wic_bitmap;
		hr = gHR = core->wic_factory->CreateBitmap(
			(UINT)texture_canvas_width,
			(UINT)texture_canvas_height,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnDemand,
			&wic_bitmap);
		if (FAILED(hr))
			return luaL_error(L, "create bitmap failed");

		// d2d1 rasterizer

		Microsoft::WRL::ComPtr<ID2D1RenderTarget> d2d1_rt;
		hr = gHR = core->d2d1_factory->CreateWicBitmapRenderTarget(
			wic_bitmap.Get(),
			D2D1::RenderTargetProperties(),
			&d2d1_rt);
		if (FAILED(hr))
			return luaL_error(L, "create rasterizer failed");

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d1_pen;
		hr = gHR = d2d1_rt->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &d2d1_pen);
		if (FAILED(hr))
			return luaL_error(L, "create rasterizer color failed");

		// rasterize

		if (lua_gettop(L) >= 3)
		{
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d1_pen2;
			hr = gHR = d2d1_rt->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &d2d1_pen2);
			if (FAILED(hr))
				return luaL_error(L, "create rasterizer color failed");

			DWriteTextRendererImplement renderer(
				core->d2d1_factory.Get(),
				d2d1_rt.Get(),
				text_layout->dwrite_text_layout.Get(),
				d2d1_pen2.Get(),
				d2d1_pen.Get(),
				outline_width);

			d2d1_rt->BeginDraw();
			d2d1_rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
			hr = gHR = text_layout->dwrite_text_layout->Draw(NULL, &renderer, outline_width, outline_width);
			if (FAILED(hr))
				return luaL_error(L, "render failed");
			hr = gHR = d2d1_rt->EndDraw();
			if (FAILED(hr))
				return luaL_error(L, "rasterize failed");
		}
		else
		{
			d2d1_rt->BeginDraw();
			d2d1_rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
			d2d1_rt->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), text_layout->dwrite_text_layout.Get(), d2d1_pen.Get());
			if (FAILED(hr))
				return luaL_error(L, "render failed");
			hr = gHR = d2d1_rt->EndDraw();
			if (FAILED(hr))
				return luaL_error(L, "rasterize failed");
		}

		// save

		Microsoft::WRL::ComPtr<IWICStream> wic_stream;
		hr = gHR = core->wic_factory->CreateStream(&wic_stream);
		if (FAILED(hr))
			return luaL_error(L, "create stream failed");

		std::wstring wide_file_path(utility::encoding::to_wide(file_path));
		hr = gHR = wic_stream->InitializeFromFilename(wide_file_path.c_str(), GENERIC_WRITE);
		if (FAILED(hr))
			return luaL_error(L, "initialize stream failed");

		AutoDeleteFileWIC auto_delete(wic_stream, wide_file_path);

		Microsoft::WRL::ComPtr<IWICBitmapEncoder> wic_bitmap_encoder;
		hr = gHR = core->wic_factory->CreateEncoder(GUID_ContainerFormatPng, NULL, &wic_bitmap_encoder);
		if (FAILED(hr))
			return luaL_error(L, "create encoder failed");

		hr = gHR = wic_bitmap_encoder->Initialize(wic_stream.Get(), WICBitmapEncoderNoCache);
		if (FAILED(hr))
			return luaL_error(L, "initialize encoder failed");

		Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> wic_bitmap_frame_encode;
		Microsoft::WRL::ComPtr<IPropertyBag2> property_bag;
		hr = gHR = wic_bitmap_encoder->CreateNewFrame(&wic_bitmap_frame_encode, &property_bag);
		if (FAILED(hr))
			return luaL_error(L, "create frame encode failed");

		hr = gHR = wic_bitmap_frame_encode->Initialize(property_bag.Get());
		if (FAILED(hr))
			return luaL_error(L, "initialize frame encode failed");

		hr = gHR = wic_bitmap_frame_encode->SetSize((UINT)texture_canvas_width, (UINT)texture_canvas_height);
		if (FAILED(hr))
			return luaL_error(L, "create frame encode failed");

		hr = gHR = wic_bitmap_frame_encode->SetResolution(72, 72);
		if (FAILED(hr))
			return luaL_error(L, "create frame encode failed");

		WICPixelFormatGUID wic_pixel_format = GUID_WICPixelFormat32bppBGRA;
		hr = gHR = wic_bitmap_frame_encode->SetPixelFormat(&wic_pixel_format);
		if (FAILED(hr))
			return luaL_error(L, "create frame encode failed");

		Microsoft::WRL::ComPtr<IWICMetadataQueryWriter> metawriter;
		if (SUCCEEDED(wic_bitmap_frame_encode->GetMetadataQueryWriter(&metawriter)))
		{
			PROPVARIANT value;
			PropVariantInit(&value);
			
			// Set Software name
			value.vt = VT_LPSTR;
			value.pszVal = const_cast<char*>("DirectXTK");
			std::ignore = metawriter->SetMetadataByName(L"/tEXt/{str=Software}", &value);

			// Set sRGB chunk
			if constexpr (false) // sRGB
			{
				value.vt = VT_UI1;
				value.bVal = 0;
				std::ignore = metawriter->SetMetadataByName(L"/sRGB/RenderingIntent", &value);
			}
			else
			{
				// add gAMA chunk with gamma 1.0
				value.vt = VT_UI4;
				value.uintVal = 100000; // gama value * 100,000 -- i.e. gamma 1.0
				std::ignore = metawriter->SetMetadataByName(L"/gAMA/ImageGamma", &value);

				// remove sRGB chunk which is added by default.
				std::ignore = metawriter->RemoveMetadataByName(L"/sRGB/RenderingIntent");
			}
		}

		hr = gHR = wic_bitmap_frame_encode->WriteSource(wic_bitmap.Get(), NULL);
		if (FAILED(hr))
			return luaL_error(L, "encode frame failed");

		hr = gHR = wic_bitmap_frame_encode->Commit();
		if (FAILED(hr))
			return luaL_error(L, "encode frame failed");

		hr = gHR = wic_bitmap_encoder->Commit();
		if (FAILED(hr))
			return luaL_error(L, "encode failed");

		auto_delete.clear();

		return 0;
	}

	template<typename T>
	struct CEnumEntry
	{
		std::string_view name;
		T value;
	};

	template<typename T, size_t N>
	struct CEnum
	{
		std::string_view name;
		std::array<CEnumEntry<T>, N> entry;
	};

	static void register_C_enum(lua_State* L)
	{
		auto regf = [&L](auto data) -> void {
			lua_push_string_view(L, data.name);
			lua_createtable(L, 0, (int)data.entry.size());
			//spdlog::info("---@class DirectWrite.{}", data.name);
			//spdlog::info("local {} = {{}}", data.name);
			//spdlog::info("M.{} = {}", data.name, data.name);
			for (auto const& v : data.entry)
			{
				//spdlog::info("{}.{} = {}", data.name, v.name, (int)v.value);
				lua_push_string_view(L, v.name);
				lua_pushinteger(L, (lua_Integer)v.value);
				lua_settable(L, -3);
			}
			lua_settable(L, -3);
		};

		CEnum<DWRITE_FONT_STRETCH, 11> const C_enum_FontStretch = {"FontStretch",{
			"Undefined"     , DWRITE_FONT_STRETCH_UNDEFINED      ,
			"UltraCondensed", DWRITE_FONT_STRETCH_ULTRA_CONDENSED,
			"ExtraCondensed", DWRITE_FONT_STRETCH_EXTRA_CONDENSED,
			"Condensed"     , DWRITE_FONT_STRETCH_CONDENSED      ,
			"SemiCondensed" , DWRITE_FONT_STRETCH_SEMI_CONDENSED ,
			"Normal"        , DWRITE_FONT_STRETCH_NORMAL         ,
			"Medium"        , DWRITE_FONT_STRETCH_MEDIUM         ,
			"SemiExpanded"  , DWRITE_FONT_STRETCH_SEMI_EXPANDED  ,
			"Expanded"      , DWRITE_FONT_STRETCH_EXPANDED       ,
			"ExtraExpanded" , DWRITE_FONT_STRETCH_EXTRA_EXPANDED ,
			"UltraExpanded" , DWRITE_FONT_STRETCH_ULTRA_EXPANDED ,
		}};
		regf(C_enum_FontStretch);

		CEnum<DWRITE_FONT_STYLE, 3> const C_enum_FontStyle = {"FontStyle",{
			"Normal"  , DWRITE_FONT_STYLE_NORMAL ,
			"Oblique" , DWRITE_FONT_STYLE_OBLIQUE,
			"Italic"  , DWRITE_FONT_STYLE_ITALIC ,
		}};
		regf(C_enum_FontStyle);

		CEnum<DWRITE_FONT_WEIGHT, 17> const C_enum_FontWeight = {"FontWeight",{
			"Thin"       , DWRITE_FONT_WEIGHT_THIN       ,
			"ExtraLight" , DWRITE_FONT_WEIGHT_EXTRA_LIGHT,
			"UltraLight" , DWRITE_FONT_WEIGHT_ULTRA_LIGHT,
			"Light"      , DWRITE_FONT_WEIGHT_LIGHT      ,
			"SemiLight"  , DWRITE_FONT_WEIGHT_SEMI_LIGHT ,
			"Normal"     , DWRITE_FONT_WEIGHT_NORMAL     ,
			"Regular"    , DWRITE_FONT_WEIGHT_REGULAR    ,
			"Medium"     , DWRITE_FONT_WEIGHT_MEDIUM     ,
			"DemiBold"   , DWRITE_FONT_WEIGHT_DEMI_BOLD  ,
			"SemiBold"   , DWRITE_FONT_WEIGHT_SEMI_BOLD  ,
			"Bold"       , DWRITE_FONT_WEIGHT_BOLD       ,
			"ExtraBold"  , DWRITE_FONT_WEIGHT_EXTRA_BOLD ,
			"UltraBold"  , DWRITE_FONT_WEIGHT_ULTRA_BOLD ,
			"Black"      , DWRITE_FONT_WEIGHT_BLACK      ,
			"Heacy"      , DWRITE_FONT_WEIGHT_HEAVY      ,
			"ExtraBlack" , DWRITE_FONT_WEIGHT_EXTRA_BLACK,
			"UltraBlack" , DWRITE_FONT_WEIGHT_ULTRA_BLACK,
		}};
		regf(C_enum_FontWeight);

		CEnum<DWRITE_LINE_SPACING_METHOD, 2> const C_enum_LineSpacingMethod = {"LineSpacingMethod",{
			"Default", DWRITE_LINE_SPACING_METHOD_DEFAULT,
			"Uniform", DWRITE_LINE_SPACING_METHOD_UNIFORM,
			//"Proportional", DWRITE_LINE_SPACING_METHOD_PROPORTIONAL, // Windows 10
		}};
		regf(C_enum_LineSpacingMethod);

		CEnum<DWRITE_TEXT_ALIGNMENT, 4> const C_enum_TextAlignment = {"TextAlignment",{
			"Leading"  , DWRITE_TEXT_ALIGNMENT_LEADING  ,
			"Trailing" , DWRITE_TEXT_ALIGNMENT_TRAILING ,
			"Center"   , DWRITE_TEXT_ALIGNMENT_CENTER   ,
			"Justified", DWRITE_TEXT_ALIGNMENT_JUSTIFIED,
		}};
		regf(C_enum_TextAlignment);

		CEnum<DWRITE_PARAGRAPH_ALIGNMENT, 3> const C_enum_ParagraphAlignment = {"ParagraphAlignment",{
			"Near"  , DWRITE_PARAGRAPH_ALIGNMENT_NEAR  ,
			"Far"   , DWRITE_PARAGRAPH_ALIGNMENT_FAR   ,
			"Center", DWRITE_PARAGRAPH_ALIGNMENT_CENTER,
		}};
		regf(C_enum_ParagraphAlignment);

		CEnum<DWRITE_FLOW_DIRECTION, 4> const C_enum_FlowDirection = {"FlowDirection",{
			"TopToBottom", DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM,
			"BottomToTop", DWRITE_FLOW_DIRECTION_BOTTOM_TO_TOP,
			"LeftToRight", DWRITE_FLOW_DIRECTION_LEFT_TO_RIGHT,
			"RightToLeft", DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT,
		}};
		regf(C_enum_FlowDirection);

		CEnum<DWRITE_READING_DIRECTION, 2> const C_enum_ReadingDirection = {"ReadingDirection",{
			"LeftToRight", DWRITE_READING_DIRECTION_LEFT_TO_RIGHT,
			"RightToLeft", DWRITE_READING_DIRECTION_RIGHT_TO_LEFT,
			//"TopToBottom", DWRITE_READING_DIRECTION_TOP_TO_BOTTOM, // Windows 8.1
			//"BottomToTop", DWRITE_READING_DIRECTION_BOTTOM_TO_TOP, // Windows 8.1
		}};
		regf(C_enum_ReadingDirection);

		CEnum<DWRITE_WORD_WRAPPING, 2> const C_enum_WordWrapping = {"WordWrapping",{
			"Wrap"          , DWRITE_WORD_WRAPPING_WRAP           ,
			"NoWrap"        , DWRITE_WORD_WRAPPING_NO_WRAP        ,
			//"EmergencyBreak", DWRITE_WORD_WRAPPING_EMERGENCY_BREAK, // Windows 8.1
			//"WholeWord"     , DWRITE_WORD_WRAPPING_WHOLE_WORD     , // Windows 8.1
			//"Character"     , DWRITE_WORD_WRAPPING_CHARACTER      , // Windows 8.1
		}};
		regf(C_enum_WordWrapping);
	}
}

int luaopen_dwrite(lua_State* L)
{
	// register module

	luaL_Reg const lib[] = {
		{ "CreateFontCollection", &DirectWrite::api_CreateFontCollection },
		{ "CreateTextFormat", &DirectWrite::api_CreateTextFormat },
		{ "CreateTextLayout", &DirectWrite::api_CreateTextLayout },
		{ "CreateTextureFromTextLayout", &DirectWrite::api_CreateTextureFromTextLayout },
		{ "SaveTextLayoutToFile", &DirectWrite::api_SaveTextLayoutToFile },
		{ NULL, NULL },
	};
	luaL_register(L, "DirectWrite", lib);
	DirectWrite::register_C_enum(L);
	DirectWrite::Factory::Register(L);
	DirectWrite::FontCollection::Register(L);
	DirectWrite::TextFormat::Register(L);
	DirectWrite::TextLayout::Register(L);

	// create core

	lua_pushlightuserdata(L, &DirectWrite::LUA_KEY);
	DirectWrite::Factory* core = DirectWrite::Factory::Create(L);
	lua_settable(L, LUA_REGISTRYINDEX);
	if (!core->InitComponents())
		return luaL_error(L, "DirectWrite initialization failed");

	return 1;
}
