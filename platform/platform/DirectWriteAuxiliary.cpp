#include "DirectWriteAuxiliary.hpp"
#include "HResultChecker.hpp"

#define HR gHR

namespace platform
{
	static std::string_view const g_font_collection_name("Custom");

	void printFontCollectionInfo(IDWriteFontCollection* dwrite_font_collection, std::stringstream& string_buffer)
	{
		for (UINT32 ff_idx = 0; ff_idx < dwrite_font_collection->GetFontFamilyCount(); ff_idx += 1)
		{
			string_buffer << '[' << ff_idx << "] Font Family\n";

			Microsoft::WRL::ComPtr<IDWriteFontFamily> dwrite_font_family;
			HR = dwrite_font_collection->GetFontFamily(ff_idx, &dwrite_font_family);

			string_buffer << "    Name:\n";
			Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> dwrite_font_family_names;
			HR = dwrite_font_family->GetFamilyNames(&dwrite_font_family_names);
			for (UINT32 name_idx = 0; name_idx < dwrite_font_family_names->GetCount(); name_idx += 1)
			{
				UINT32 str_len = 0;

				HR = dwrite_font_family_names->GetStringLength(name_idx, &str_len);
				std::wstring name(str_len + 1, L'\0');
				HR = dwrite_font_family_names->GetString(name_idx, name.data(), str_len + 1);
				if (name.back() == L'\0') name.pop_back();

				HR = dwrite_font_family_names->GetLocaleNameLength(name_idx, &str_len);
				std::wstring locale_name(str_len + 1, L'\0');
				HR = dwrite_font_family_names->GetLocaleName(name_idx, locale_name.data(), str_len + 1);
				if (locale_name.back() == L'\0') locale_name.pop_back();

				string_buffer << "        [" << name_idx << "] (" << to_utf8(locale_name) << ") " << to_utf8(name) << '\n';
			}

			string_buffer << "    Font:\n";
			for (UINT32 font_idx = 0; font_idx < dwrite_font_family->GetFontCount(); font_idx += 1)
			{
				string_buffer << "        [" << font_idx << "] Font\n";

				Microsoft::WRL::ComPtr<IDWriteFont> dwrite_font;
				HR = dwrite_font_family->GetFont(font_idx, &dwrite_font);

				string_buffer << "            Name:\n";
				Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> dwrite_font_face_names;
				HR = dwrite_font->GetFaceNames(&dwrite_font_face_names);
				for (UINT32 name_idx = 0; name_idx < dwrite_font_face_names->GetCount(); name_idx += 1)
				{
					UINT32 str_len = 0;

					HR = dwrite_font_face_names->GetStringLength(name_idx, &str_len);
					std::wstring name(str_len + 1, L'\0');
					HR = dwrite_font_face_names->GetString(name_idx, name.data(), str_len + 1);
					if (name.back() == L'\0') name.pop_back();

					HR = dwrite_font_face_names->GetLocaleNameLength(name_idx, &str_len);
					std::wstring locale_name(str_len + 1, L'\0');
					HR = dwrite_font_face_names->GetLocaleName(name_idx, locale_name.data(), str_len + 1);
					if (locale_name.back() == L'\0') locale_name.pop_back();

					string_buffer << "                [" << name_idx << "] (" << to_utf8(locale_name) << ") " << to_utf8(name) << '\n';
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

				switch (dwrite_font->GetWeight())
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
				default: assert(false); break;
				}

				string_buffer << "            Symbol Font: " << (dwrite_font->IsSymbolFont() ? "Yes" : "No") << "\n";
			}
		}
	}

	template<typename T>
	class IUnknownImplement : public T
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
		IUnknownImplement() : m_ref(1) {}
		virtual ~IUnknownImplement() {}
	};

	class DirectWriteFontFileStream : public IUnknownImplement<IDWriteFontFileStream>
	{
	private:
		std::string m_name;
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
			m_name = path; // OOM catch by factory
			Microsoft::WRL::Wrappers::FileHandle file;
			file.Attach(CreateFileW(
				to_wide(path).c_str(),
				FILE_GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			));
			if (!file.IsValid())
			{
				gHRLastError;
				return false;
			}
			LARGE_INTEGER size = {};
			if (!GetFileSizeEx(file.Get(), &size))
			{
				gHRLastError;
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
				gHRLastError;
				return false;
			}
			if (read_size != size.LowPart)
			{
				assert(false);
				return false;
			}
			return true;
		}
		std::string_view getName() { return m_name; }
	public:
		DirectWriteFontFileStream() {}
		virtual ~DirectWriteFontFileStream() {}
	};

	class DirectWriteFontFileLoader : public IUnknownImplement<IDWriteFontFileLoader>
	{
	private:
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<DirectWriteFontFileStream>> m_cache;
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
				Microsoft::WRL::ComPtr<DirectWriteFontFileStream> object;
				object.Attach(new DirectWriteFontFileStream());
				if (!object->loadFromFile(path))
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
		DirectWriteFontFileLoader() {}
		virtual ~DirectWriteFontFileLoader() {}
	};

	class DirectWriteFontFileEnumerator : public IUnknownImplement<IDWriteFontFileEnumerator>
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		Microsoft::WRL::ComPtr<IDWriteFontFileLoader> m_dwrite_font_file_loader;
		std::vector<std::string> m_font_file_name_list;
		LONG m_index{};
	public:
		HRESULT WINAPI MoveNext(BOOL* hasCurrentFile)
		{
			assert(hasCurrentFile);
			m_index += 1;
			if (m_index >= 0 && m_index < (LONG)m_font_file_name_list.size())
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
			assert(m_index >= 0 && m_index < (LONG)m_font_file_name_list.size());
			assert(fontFile);
			if (m_index < 0 || m_index >(LONG)m_font_file_name_list.size())
			{
				return E_FAIL;
			}
			std::wstring wide_path(to_wide(m_font_file_name_list[m_index]));
			std::error_code ec;
			if (std::filesystem::is_regular_file(wide_path, ec))
			{
				return m_dwrite_factory->CreateFontFileReference(
					wide_path.c_str(),
					NULL,
					fontFile
				);
			}
			else
			{
				return m_dwrite_factory->CreateCustomFontFileReference(
					m_font_file_name_list[m_index].data(),
					(UINT32)m_font_file_name_list[m_index].size(),
					m_dwrite_font_file_loader.Get(),
					fontFile
				);
			}
		}
	public:
		void reset(IDWriteFactory* factory, IDWriteFontFileLoader* loader, std::vector<std::string>& list)
		{
			assert(factory);
			assert(loader);
			m_dwrite_factory = factory;
			m_dwrite_font_file_loader = loader;
			m_font_file_name_list = list; // bulk copy operations, OOM catch by factory
			m_index = -1;
		}
	public:
		DirectWriteFontFileEnumerator() {}
		virtual ~DirectWriteFontFileEnumerator() {}
	};

	class DirectWriteFontCollectionLoader : public IUnknownImplement<IDWriteFontCollectionLoader>
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		Microsoft::WRL::ComPtr<IDWriteFontFileLoader> m_dwrite_font_file_loader;
		std::vector<std::string>* m_font_file_name_list{};
	public:
		HRESULT WINAPI CreateEnumeratorFromKey(IDWriteFactory* factory, void const* collectionKey, UINT32 collectionKeySize, IDWriteFontFileEnumerator** fontFileEnumerator)
		{
			assert(m_dwrite_factory);
			assert(m_dwrite_font_file_loader);
			assert(m_font_file_name_list);
			assert(collectionKey || collectionKeySize == 0);
			assert(factory);
			assert(fontFileEnumerator);
			if (std::string_view((char*)collectionKey, collectionKeySize) != g_font_collection_name)
				return E_INVALIDARG;
			try
			{
				Microsoft::WRL::ComPtr<DirectWriteFontFileEnumerator> object;
				object.Attach(new DirectWriteFontFileEnumerator());
				object->reset(
					m_dwrite_factory.Get(),
					m_dwrite_font_file_loader.Get(),
					*m_font_file_name_list
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
		void reset(IDWriteFactory* factory, IDWriteFontFileLoader* loader, std::vector<std::string>& list)
		{
			assert(factory);
			assert(loader);
			m_dwrite_factory = factory;
			m_dwrite_font_file_loader = loader;
			m_font_file_name_list = &list;
		}
	public:
		DirectWriteFontCollectionLoader() {}
		virtual ~DirectWriteFontCollectionLoader() {}
	};

	class DirectWriteAuxiliary : public IUnknownImplement<IDirectWriteAuxiliary>
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		Microsoft::WRL::ComPtr<DirectWriteFontFileLoader> m_dwrite_font_file_loader;
		Microsoft::WRL::ComPtr<DirectWriteFontCollectionLoader> m_dwrite_font_collection_loader;
		Microsoft::WRL::ComPtr<IDWriteFontCollection> m_dwrite_font_collection;
		std::vector<std::string> m_search_path;
		std::vector<std::string> m_font_file_name_list;
		std::string m_info;
	public:
		STDMETHOD_(IDWriteFontCollection*, GetCustomFontCollection)()
		{
			return m_dwrite_font_collection.Get();
		}
		STDMETHOD(AddSearchPath)(_In_z_ WCHAR const* filePath)
		{
			m_search_path.emplace_back(to_utf8(filePath));
			return S_OK;
		}
		STDMETHOD(Refresh)()
		{
			HRESULT hr;
			std::error_code ec;
		
			m_font_file_name_list.clear();
			for (auto const& v : m_search_path)
			{
				for (auto& entry : std::filesystem::directory_iterator(to_wide(v), std::filesystem::directory_options::skip_permission_denied, ec))
				{
					m_font_file_name_list.emplace_back(to_utf8(entry.path().generic_wstring()));
				}
			}

			hr = HR = m_dwrite_factory->CreateCustomFontCollection(
				m_dwrite_font_collection_loader.Get(),
				g_font_collection_name.data(),
				(UINT32)g_font_collection_name.size(),
				&m_dwrite_font_collection);
			if (FAILED(hr))
				return hr;

			std::stringstream sbuf;
			sbuf << "Search Path:\n";
			for (auto const& v : m_search_path)
				sbuf << "    " << v << '\n';
			sbuf << "Font File:\n";
			for (auto const& v : m_font_file_name_list)
				sbuf << "    " << v << '\n';
			sbuf << "Font Collection Information:\n";
			printFontCollectionInfo(m_dwrite_font_collection.Get(), sbuf);
			m_info = sbuf.str();

			return S_OK;
		}
	public:
		DirectWriteAuxiliary(IDWriteFactory* factory) : m_dwrite_factory(factory)
		{
			assert(factory);

			m_dwrite_font_file_loader.Attach(new DirectWriteFontFileLoader());
			HR = m_dwrite_factory->RegisterFontFileLoader(m_dwrite_font_file_loader.Get());

			m_dwrite_font_collection_loader.Attach(new DirectWriteFontCollectionLoader());
			m_dwrite_font_collection_loader->reset(
				m_dwrite_factory.Get(),
				m_dwrite_font_file_loader.Get(),
				m_font_file_name_list
			);
			HR = m_dwrite_factory->RegisterFontCollectionLoader(m_dwrite_font_collection_loader.Get());
		}
		virtual ~DirectWriteAuxiliary()
		{
			m_dwrite_font_collection.Reset();
			if (m_dwrite_factory)
			{
				m_dwrite_factory->UnregisterFontCollectionLoader(m_dwrite_font_collection_loader.Get());
				m_dwrite_factory->UnregisterFontFileLoader(m_dwrite_font_file_loader.Get());
			}
			m_dwrite_font_collection_loader.Reset();
			m_dwrite_font_file_loader.Reset();
			m_dwrite_factory.Reset();
			m_font_file_name_list.clear();
		}
	};
}

HRESULT WINAPI DirectWriteCreateAuxiliary(_In_ IDWriteFactory* pFactory, _COM_Outptr_ IDirectWriteAuxiliary** ppAuxiliary) WIN_NOEXCEPT
{
	try
	{
		Microsoft::WRL::ComPtr<platform::DirectWriteAuxiliary> object;
		object.Attach(new platform::DirectWriteAuxiliary(pFactory));
		*ppAuxiliary = object.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}
