#include "core/Clipboard.hpp"
#include <windows.h>
#include <wil/resource.h>

namespace core {
    bool Clipboard::hasText() {
        return IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE;
    }
    bool Clipboard::setText(std::string_view const& text) {
        if (text.empty()) {
            return true;
        }

        [[maybe_unused]] auto auto_close_clipboard = wil::open_clipboard(nullptr);
        if (!auto_close_clipboard) {
            return false;
        }

        if (EmptyClipboard() != TRUE) {
            return false;
        }

        auto const count = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (count <= 0) {
            return false;
        }

        wil::unique_hglobal handle;
        handle.reset(GlobalAlloc(GMEM_MOVEABLE, static_cast<size_t>(count + 1) * sizeof(wchar_t))); // null-terminate
        if (!handle) {
            return false;
        }

        {
            wil::unique_hglobal_locked locked(handle.get());
            if (!locked) {
                return false;
            }
            auto const buffer = static_cast<wchar_t*>(locked.get());
            auto const result = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), buffer, count);
            if (result != count) {
                return false;
            }
            buffer[count] = L'\0'; // fill null-terminate
        }

        if (SetClipboardData(CF_UNICODETEXT, handle.get()) == nullptr) {
            return false;
        }

        // now system own the memory block
        *handle.addressof() = nullptr;

        // manually close clipboard
        auto_close_clipboard.release();
        return CloseClipboard() == TRUE;
    }
    bool Clipboard::getText(std::string& buffer) {
        buffer.clear();

        if (!hasText()) {
            return false;
        }

        [[maybe_unused]] auto auto_close_clipboard = wil::open_clipboard(nullptr);
        if (!auto_close_clipboard) {
            return false;
        }

        auto const handle = static_cast<HGLOBAL>(GetClipboardData(CF_UNICODETEXT));
        if (handle == nullptr) {
            return false;
        }

        {
            wil::unique_hglobal_locked locked(handle);
            if (!locked) {
                return false;
            }
            auto const text = static_cast<wchar_t*>(locked.get());
            auto const count = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
            if (count <= 0) {
                return false;
            }
            buffer.resize(static_cast<size_t>(count));
            auto const result = WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer.data(), count, nullptr, nullptr);
            if (result != count) {
                return false;
            }
        }

        return true;
    }
}
