#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#include <sdkddkver.h>

#include <string>
#include <string_view>
#include <atomic>
#include <algorithm>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Unknwn.h>
#include <VersionHelpers.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include "winrt/base.h" // TODO: New Windows SDK
#include <wil/resource.h>
#include <timeapi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <DirectXMath.h>

#include <xaudio2.h>

#include "spdlog/spdlog.h"

#include "Tracy.hpp"
#include "TracyD3D11.hpp"
#include "TracyD3D11Ctx.hpp"

#include "Platform/HResultChecker.hpp"

namespace Microsoft::WRL::Wrappers
{
	namespace HandleTraits
	{
		struct ThreadHandleTraits : HANDLENullTraits
		{
		};
	}
	using ThreadHandle = HandleT<HandleTraits::ThreadHandleTraits>;
}

namespace winrt
{
    template<typename T>
    struct xaudio2_voice_ptr
    {
        using type = impl::abi_t<T>;

        xaudio2_voice_ptr(std::nullptr_t = nullptr) noexcept {}

        /*
        xaudio2_voice_ptr(xaudio2_voice_ptr const& other) noexcept : m_ptr(other.m_ptr)
        {
            add_ref();
        }

        template <typename U>
        xaudio2_voice_ptr(xaudio2_voice_ptr<U> const& other) noexcept : m_ptr(other.m_ptr)
        {
            add_ref();
        }
        */

        xaudio2_voice_ptr(xaudio2_voice_ptr const&) = delete;

        template <typename U>
        xaudio2_voice_ptr(xaudio2_voice_ptr<U> const&) = delete;

        template <typename U>
        xaudio2_voice_ptr(xaudio2_voice_ptr<U>&& other) noexcept : m_ptr(std::exchange(other.m_ptr, {}))
        {
        }

        ~xaudio2_voice_ptr() noexcept
        {
            release_ref();
        }

        /*
        xaudio2_voice_ptr& operator=(xaudio2_voice_ptr const& other) noexcept
        {
            copy_ref(other.m_ptr);
            return*this;
        }
        */

        xaudio2_voice_ptr& operator=(xaudio2_voice_ptr&& other) noexcept
        {
            if (this != &other)
            {
                release_ref();
                m_ptr = std::exchange(other.m_ptr, {});
            }

            return*this;
        }

        /*
        template <typename U>
        xaudio2_voice_ptr& operator=(xaudio2_voice_ptr<U> const& other) noexcept
        {
            copy_ref(other.m_ptr);
            return*this;
        }
        */

        template <typename U>
        xaudio2_voice_ptr& operator=(xaudio2_voice_ptr<U>&& other) noexcept
        {
            release_ref();
            m_ptr = std::exchange(other.m_ptr, {});
            return*this;
        }

        explicit operator bool() const noexcept
        {
            return m_ptr != nullptr;
        }

        auto operator->() const noexcept
        {
            return m_ptr;
        }

        T& operator*() const noexcept
        {
            return *m_ptr;
        }

        type* get() const noexcept
        {
            return m_ptr;
        }

        type** put() noexcept
        {
            WINRT_ASSERT(m_ptr == nullptr);
            return &m_ptr;
        }

        void** put_void() noexcept
        {
            return reinterpret_cast<void**>(put());
        }

        void attach(type* value) noexcept
        {
            release_ref();
            *put() = value;
        }

        type* detach() noexcept
        {
            return std::exchange(m_ptr, {});
        }

        friend void swap(xaudio2_voice_ptr& left, xaudio2_voice_ptr& right) noexcept
        {
            std::swap(left.m_ptr, right.m_ptr);
        }

        /*

        template <typename To>
        auto as() const
        {
            return impl::as<To>(m_ptr);
        }

        template <typename To>
        auto try_as() const noexcept
        {
            return impl::try_as<To>(m_ptr);
        }

        template <typename To>
        void as(To& to) const
        {
            to = as<impl::wrapped_type_t<To>>();
        }

        template <typename To>
        bool try_as(To& to) const noexcept
        {
            if constexpr (impl::is_com_interface_v<To> || !std::is_same_v<To, impl::wrapped_type_t<To>>)
            {
                to = try_as<impl::wrapped_type_t<To>>();
                return static_cast<bool>(to);
            }
            else
            {
                auto result = try_as<To>();
                to = result.has_value() ? result.value() : impl::empty_value<To>();
                return result.has_value();
            }
        }

        hresult as(guid const& id, void** result) const noexcept
        {
            return m_ptr->QueryInterface(id, result);
        }

        void copy_from(type* other) noexcept
        {
            copy_ref(other);
        }

        void copy_to(type** other) const noexcept
        {
            add_ref();
            *other = m_ptr;
        }

        template <typename F, typename...Args>
        bool try_capture(F function, Args&&...args)
        {
            return function(args..., guid_of<T>(), put_void()) >= 0;
        }

        template <typename O, typename M, typename...Args>
        bool try_capture(xaudio2_voice_ptr<O> const& object, M method, Args&&...args)
        {
            return (object.get()->*(method))(args..., guid_of<T>(), put_void()) >= 0;
        }

        template <typename F, typename...Args>
        void capture(F function, Args&&...args)
        {
            check_hresult(function(args..., guid_of<T>(), put_void()));
        }

        template <typename O, typename M, typename...Args>
        void capture(xaudio2_voice_ptr<O> const& object, M method, Args&&...args)
        {
            check_hresult((object.get()->*(method))(args..., guid_of<T>(), put_void()));
        }
        */

    private:

        /*
        void copy_ref(type* other) noexcept
        {
            if (m_ptr != other)
            {
                release_ref();
                m_ptr = other;
                add_ref();
            }
        }

        void add_ref() const noexcept
        {
            if (m_ptr)
            {
                const_cast<std::remove_const_t<type>*>(m_ptr)->AddRef();
            }
        }
        */

        void release_ref() noexcept
        {
            if (m_ptr)
            {
                unconditional_release_ref();
            }
        }

        __declspec(noinline) void unconditional_release_ref() noexcept
        {
            std::exchange(m_ptr, {})->DestroyVoice();
        }

        template <typename U>
        friend struct xaudio2_voice_ptr;

        type* m_ptr{};
    };
}

#ifdef _DEBUG
inline void F_D3D_SET_DEBUG_NAME(IDXGIObject* ptr, std::string_view file, int line, std::string_view name)
{
	if (ptr)
	{
		std::stringstream strstr;
		strstr << "File: " << file << " ";
		strstr << "Line: " << line << " ";
		strstr << "DXGIObject: " << name;
		std::string str = strstr.str();
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
	}
}
inline void F_D3D_SET_DEBUG_NAME(ID3D11Device* ptr, std::string_view file, int line, std::string_view name)
{
	if (ptr)
	{
		std::stringstream strstr;
		strstr << "File: " << file << " ";
		strstr << "Line: " << line << " ";
		strstr << "DXGIObject: " << name;
		std::string str = strstr.str();
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
	}
}
inline void F_D3D_SET_DEBUG_NAME(ID3D11DeviceChild* ptr, std::string_view file, int line, std::string_view name)
{
	if (ptr)
	{
		std::stringstream strstr;
		strstr << "File: " << file << " ";
		strstr << "Line: " << line << " ";
		strstr << "DXGIObject: " << name;
		std::string str = strstr.str();
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
	}
}
#define M_D3D_SET_DEBUG_NAME(OBJ, STR) F_D3D_SET_DEBUG_NAME(OBJ, __FILE__, __LINE__, STR)
#define M_D3D_SET_DEBUG_NAME_SIMPLE(OBJ) M_D3D_SET_DEBUG_NAME(OBJ, #OBJ)
#else
#define M_D3D_SET_DEBUG_NAME(OBJ, STR)
#define M_D3D_SET_DEBUG_NAME_SIMPLE(OBJ)
#endif
