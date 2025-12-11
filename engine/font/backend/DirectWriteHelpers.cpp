#include "core/SmartReference.hpp"
#include "core/FileSystem.hpp"
#include "backend/DirectWriteHelpers.hpp"
#include "win32/base.hpp"
#include <cassert>

#pragma comment(lib, "dwrite.lib")

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr char collection_key_prefix[]{"LSTGSUB"};

    struct CollectionKey {
        char prefix[8]{};
        core::IImmutableStringList* files{};
    };

    template<typename T>
    class SimpleSingletonUnknown : public T {
    public:
        HRESULT WINAPI QueryInterface(const IID& riid, void** const ppvObject) override {
            if (ppvObject == nullptr) {
                return E_POINTER;
            }
            if (riid == __uuidof(IUnknown)) {
                *ppvObject = static_cast<IUnknown*>(this);
                return S_OK;
            }
            static_assert(std::is_base_of_v<IUnknown, T>, "T must inherit from IUnknown");
            if (riid == __uuidof(T)) {
                *ppvObject = static_cast<T*>(this);
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        ULONG WINAPI AddRef() override {
            return 2;
        }

        ULONG WINAPI Release() override {
            return 1;
        }
    };

    template<typename T>
    class SimpleUnknown : public T {
    public:
        HRESULT WINAPI QueryInterface(const IID& riid, void** const ppvObject) override {
            if (ppvObject == nullptr) {
                return E_POINTER;
            }
            if (riid == __uuidof(IUnknown)) {
                AddRef();
                *ppvObject = static_cast<IUnknown*>(this);
                return S_OK;
            }
            static_assert(std::is_base_of_v<IUnknown, T>, "T must inherit from IUnknown");
            if (riid == __uuidof(T)) {
                AddRef();
                *ppvObject = static_cast<T*>(this);
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        ULONG WINAPI AddRef() override {
            return InterlockedIncrement(&m_ref);
        }

        ULONG WINAPI Release() override {
            const auto ret = InterlockedDecrement(&m_ref);
            if (ret == 0) {
                delete this;
            }
            return ret;
        }

        virtual ~SimpleUnknown() = default;

    private:
        volatile ULONG m_ref{1};
    };

    class DirectWriteFontFileStream final : public SimpleUnknown<IDWriteFontFileStream> {
	public:
        // IDWriteFontFileStream

		HRESULT WINAPI ReadFileFragment(void const** fragmentStart, UINT64 fileOffset, UINT64 fragmentSize, void** fragmentContext) noexcept {
			if (fragmentStart == nullptr) {
                return E_POINTER;
            }
            if (fileOffset >= m_data->size() || (fileOffset + fragmentSize) > m_data->size()) {
                return E_INVALIDARG;
            }
            const auto pointer = static_cast<uint8_t*>(m_data->data()) + fileOffset;
			*fragmentStart = pointer;
			*fragmentContext = pointer; // for identification only
			return S_OK;
		}
		void WINAPI ReleaseFileFragment(void* fragmentContext) noexcept {
			UNREFERENCED_PARAMETER(fragmentContext);
		}
		HRESULT WINAPI GetFileSize(UINT64* fileSize) noexcept {
            if (fileSize == nullptr) {
                return E_POINTER;
            }
			*fileSize = m_data->size();
			return S_OK;
		}
		HRESULT WINAPI GetLastWriteTime(UINT64* lastWriteTime) noexcept {
			if (lastWriteTime == nullptr) {
                return E_POINTER;
            }
            *lastWriteTime = 0;
			return S_OK;
		}

        // DirectWriteFontFileStream

        explicit DirectWriteFontFileStream(core::IData* const data) noexcept : m_data(data) {}

    private:
		core::SmartReference<core::IData> m_data;
	};

    class DirectWriteFontFileLoader final : public SimpleSingletonUnknown<IDWriteFontFileLoader> {
    public:
        // IDWriteFontFileLoader

        HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
            _In_reads_bytes_(fontFileReferenceKeySize) void const* fontFileReferenceKey,
            UINT32 fontFileReferenceKeySize,
            _COM_Outptr_ IDWriteFontFileStream** fontFileStream
        ) override {
            if (fontFileReferenceKey == nullptr || fontFileStream == nullptr) {
                return E_POINTER;
            }
            if (fontFileReferenceKeySize == 0) {
                return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            const std::string_view path(static_cast<const char*>(fontFileReferenceKey), fontFileReferenceKeySize);
            core::SmartReference<core::IData> data;
            if (!core::FileSystemManager::readFile(path, data.put())) {
                return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            *fontFileStream = new DirectWriteFontFileStream(data.get());
            return S_OK;
        }
    };

    DirectWriteFontFileLoader s_font_file_loader;

    class DirectWriteFontFileEnumerator final : public SimpleUnknown<IDWriteFontFileEnumerator> {
    public:
        // IDWriteFontFileEnumerator

        HRESULT STDMETHODCALLTYPE MoveNext(
            _Out_ BOOL* hasCurrentFile
        ) override {
            if (hasCurrentFile == nullptr) {
                return E_POINTER;
            }
            const auto count = static_cast<LONG>(m_files->getCount());
            if (count == 0) {
                *hasCurrentFile = FALSE;
                return S_OK;
            }
            m_index += 1;
            *hasCurrentFile = (m_index >= 0 && m_index < count);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetCurrentFontFile(
            _COM_Outptr_ IDWriteFontFile** fontFile
        ) override {
            if (fontFile == nullptr) {
                return E_POINTER;
            }
            const auto count = static_cast<LONG>(m_files->getCount());
            if (m_index < 0 || m_index >= count) {
                return E_FAIL;
            }
            const auto path = m_files->getView(static_cast<size_t>(m_index));
            return m_factory->CreateCustomFontFileReference(
                path.data(),
                static_cast<UINT32>(path.size()),
                &s_font_file_loader,
                fontFile
            );
        }

        // DirectWriteFontFileEnumerator

        DirectWriteFontFileEnumerator(IDWriteFactory* const factory, core::IImmutableStringList* const files)
            : m_factory(factory), m_files(files), m_index(-1) {
        }

    private:
        win32::com_ptr<IDWriteFactory> m_factory;
        core::SmartReference<core::IImmutableStringList> m_files;
        LONG m_index{-1};
    };

    class DirectWriteFontCollectionLoader final : public SimpleSingletonUnknown<IDWriteFontCollectionLoader> {
    public:
        // IDWriteFontCollectionLoader

        HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
            _In_ IDWriteFactory* factory,
            _In_reads_bytes_(collectionKeySize) void const* collectionKey,
            UINT32 collectionKeySize,
            _COM_Outptr_ IDWriteFontFileEnumerator** fontFileEnumerator
        ) override {
            if (collectionKeySize != sizeof(CollectionKey)) {
                return E_INVALIDARG;
            }
            const auto collection_key = static_cast<const CollectionKey*>(collectionKey);
            if (std::memcmp(collection_key_prefix, collection_key->prefix, 8) != 0) {
                return E_INVALIDARG;
            }
            if (collection_key->files == nullptr) {
                return E_INVALIDARG;
            }
            *fontFileEnumerator = new DirectWriteFontFileEnumerator(factory, collection_key->files);
            return S_OK;
        }
    };

    DirectWriteFontCollectionLoader s_font_collection_loader; 

    class Instance final {
    public:
        Instance() = default;
        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;
        ~Instance() {
            if (m_factory != nullptr) {
                m_factory->UnregisterFontCollectionLoader(&s_font_collection_loader);
                m_factory->UnregisterFontFileLoader(&s_font_file_loader);
                m_factory->Release();
            }
        }
    
        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) = delete;

        IDWriteFactory* get() {
            if (m_factory == nullptr) {
                IDWriteFactory* factory{};
                if (!win32::check_hresult_as_boolean(
                    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory)),
                    "DWriteCreateFactory"sv
                )) {
                    return nullptr;
                }
                if (m_factory != nullptr) {
                    factory->Release();
                    return m_factory;
                }
                const auto original = InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(&m_factory), factory, nullptr);
                if (original != nullptr) {
                    factory->Release();
                }
                m_factory->RegisterFontCollectionLoader(&s_font_collection_loader);
                m_factory->RegisterFontFileLoader(&s_font_file_loader);
            }
            return m_factory;
        }

    private:
        IDWriteFactory* volatile m_factory{};
    };

    Instance s_instance;
}

namespace core {
    IDWriteFactory* DirectWriteHelpers::getFactory() {
        return s_instance.get();
    }
    HRESULT DirectWriteHelpers::createCustomFontCollection(core::IImmutableStringList* files, IDWriteFontCollection** const output) {
        assert(files != nullptr);
        assert(output != nullptr);
        CollectionKey collection_key{};
        std::memcpy(collection_key.prefix, collection_key_prefix, 8);
        collection_key.files = files;
        return getFactory()->CreateCustomFontCollection(
            &s_font_collection_loader,
            &collection_key,
            sizeof(collection_key),
            output
        );
    }

    DWRITE_FONT_WEIGHT DirectWriteHelpers::toFontWeight(const FontWeight weight) noexcept {
        switch (weight) {
        case FontWeight::thin:        return DWRITE_FONT_WEIGHT_THIN;
        case FontWeight::extra_light: return DWRITE_FONT_WEIGHT_EXTRA_BLACK;
        case FontWeight::light:       return DWRITE_FONT_WEIGHT_LIGHT;
        case FontWeight::normal:      return DWRITE_FONT_WEIGHT_NORMAL;
        case FontWeight::medium:      return DWRITE_FONT_WEIGHT_MEDIUM;
        case FontWeight::semi_bold:   return DWRITE_FONT_WEIGHT_SEMI_BOLD;
        case FontWeight::bold:        return DWRITE_FONT_WEIGHT_BOLD;
        case FontWeight::extra_bold:  return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
        case FontWeight::black:       return DWRITE_FONT_WEIGHT_BLACK;
        default: return {}; // unlikely
        }
    }
    DWRITE_FONT_STYLE DirectWriteHelpers::toFontStyle(const FontStyle style) noexcept {
        switch (style) {
        case FontStyle::normal:  return DWRITE_FONT_STYLE_NORMAL;
        case FontStyle::oblique: return DWRITE_FONT_STYLE_OBLIQUE;
        case FontStyle::italic:  return DWRITE_FONT_STYLE_ITALIC;
        default: return {}; // unlikely
        }
    }
    DWRITE_FONT_STRETCH DirectWriteHelpers::toFontWidth(const FontWidth width) noexcept {
        switch (width) {
        case FontWidth::ultra_condensed: return DWRITE_FONT_STRETCH_ULTRA_CONDENSED;
        case FontWidth::extra_condensed: return DWRITE_FONT_STRETCH_EXTRA_CONDENSED;
        case FontWidth::condensed:       return DWRITE_FONT_STRETCH_CONDENSED;
        case FontWidth::semi_condensed:  return DWRITE_FONT_STRETCH_SEMI_CONDENSED;
        case FontWidth::normal:          return DWRITE_FONT_STRETCH_NORMAL;
        case FontWidth::semi_expanded:   return DWRITE_FONT_STRETCH_SEMI_EXPANDED;
        case FontWidth::expanded:        return DWRITE_FONT_STRETCH_EXPANDED;
        case FontWidth::extra_expanded:  return DWRITE_FONT_STRETCH_EXTRA_EXPANDED;
        case FontWidth::ultra_expanded:  return DWRITE_FONT_STRETCH_ULTRA_EXPANDED;
        default: return {}; // unlikely
        }
    }
}
