#include "Shared.hpp"
#include "Keyboard.hpp"

/*
    [0|0|0|0000|0|00000000|0000000000000000]
     | | | |    | |        |
     | | | |    | |        +-- The repeat count for the current message.
     | | | |    | |            The value is the number of times the keystroke is autorepeated
     | | | |    | |            as a result of the user holding down the key.
     | | | |    | |            If the keystroke is held long enough, multiple messages are sent.
     | | | |    | |            However, the repeat count is not cumulative.
     | | | |    | |            The repeat count is always 1 for a WM_KEYUP message.
     | | | |    | |
     | | | |    | +----------- The scan code. The value depends on the OEM.
     | | | |    |
     | | | |    +------------- Indicates whether the key is an extended key,
     | | | |                   such as the right-hand ALT and CTRL keys that
     | | | |                   that appear on an enhanced 101- or 102-key keyboard.
     | | | |                   The value is 1 if it is an extended key; otherwise, it is 0.
     | | | |
     | | | +------------------ Reserved; do not use.
     | | |
     | | +-------------------- The context code. The value is always 0 for a WM_KEYUP/WM_KEYDOWN message.
     | |
     | +---------------------- The previous key state. The value is always 1 for a WM_KEYUP message.
     |                         The value is 1 if the key is down before the message is sent,
     |                         or it is zero if the key is up.
     |
     +------------------------ The transition state. The value is always 0 for a WM_KEYDOWN message.
                             The value is always 1 for a WM_KEYUP message.
*/

namespace Platform
{
    static_assert(sizeof(unsigned int) == 4);
    static_assert(sizeof(Keyboard::State::KeyStateData) == 32);
    static_assert(sizeof(Keyboard::State) == (32 * 3 + 4));

    inline bool IsExtendedKey(LPARAM lParam)
    {
        constexpr LPARAM const bit_mask = KF_EXTENDED << 16;
        return (lParam & bit_mask) == bit_mask;
    }

    inline bool GetKey(uint32_t state_[8], uint8_t vk)
    {
        uint32_t const index = (vk & 0x7FFFFFE0u) >> 5u;
        uint32_t const bitmk = 1u << (vk & 0x0000001Fu);
        return (state_[index] & bitmk) == bitmk;
    }
#if PLATFORM_KEYBOARD_USING_ATOMIC
    inline void AtomicSetKey(std::atomic_uint32_t state_[8], uint8_t vk, bool down)
    {
        uint32_t const index = (vk & 0x7FFFFFE0u) >> 5u;
        uint32_t const bitmk = 1u << (vk & 0x0000001Fu);
        if (down)
            state_[index].fetch_or(bitmk);
        else
            state_[index].fetch_and(~bitmk);
    }
#else
    inline void AtomicSetKey(uint32_t state_[8], uint8_t vk, bool down)
    {
        uint32_t const index = (vk & 0x7FFFFFE0u) >> 5u;
        uint32_t const bitmk = 1u << (vk & 0x0000001Fu);
        if (down)
            state_[index] |= bitmk;
        else
            state_[index] &= (~bitmk);
    }
#endif
    
    constexpr uint8_t const VK_NUMPADENTER = 0xE8u; // Unassigned Virtual-Key Code

    void Keyboard::State::Reset()
    {
        std::memset(this, 0, sizeof(State));
    }
    bool Keyboard::State::IsKeyDown(Keyboard::Key key, bool include_history)
    {
        return GetKey(include_history ? KeyDown.Data : KeyState.Data, static_cast<uint8_t>(key));
    }
    bool Keyboard::State::IsKeyUp(Keyboard::Key key, bool include_history)
    {
        return GetKey(include_history ? KeyUp.Data : KeyState.Data, static_cast<uint8_t>(key));
    }

    void Keyboard::Reset()
    {
    #if PLATFORM_KEYBOARD_USING_ATOMIC
        KeyState[0].store(0u);
        KeyState[1].store(0u);
        KeyState[2].store(0u);
        KeyState[3].store(0u);
        KeyState[4].store(0u);
        KeyState[5].store(0u);
        KeyState[6].store(0u);
        KeyState[7].store(0u);
    #else
        KeyState[0] = (0u);
        KeyState[1] = (0u);
        KeyState[2] = (0u);
        KeyState[3] = (0u);
        KeyState[4] = (0u);
        KeyState[5] = (0u);
        KeyState[6] = (0u);
        KeyState[7] = (0u);
    #endif
        ResetFrame();
        _Padding[0] = 0;
        _Padding[1] = 0;
    }
    void Keyboard::ResetFrame()
    {
    #if PLATFORM_KEYBOARD_USING_ATOMIC
        KeyDown[0].store(0u);
        KeyDown[1].store(0u);
        KeyDown[2].store(0u);
        KeyDown[3].store(0u);
        KeyDown[4].store(0u);
        KeyDown[5].store(0u);
        KeyDown[6].store(0u);
        KeyDown[7].store(0u);
        KeyUp[0].store(0u);
        KeyUp[1].store(0u);
        KeyUp[2].store(0u);
        KeyUp[3].store(0u);
        KeyUp[4].store(0u);
        KeyUp[5].store(0u);
        KeyUp[6].store(0u);
        KeyUp[7].store(0u);
        LastKeyDown.store(0u);
        LastKeyUp.store(0u);
    #else
        KeyDown[0] = (0u);
        KeyDown[1] = (0u);
        KeyDown[2] = (0u);
        KeyDown[3] = (0u);
        KeyDown[4] = (0u);
        KeyDown[5] = (0u);
        KeyDown[6] = (0u);
        KeyDown[7] = (0u);
        KeyUp[0] = (0u);
        KeyUp[1] = (0u);
        KeyUp[2] = (0u);
        KeyUp[3] = (0u);
        KeyUp[4] = (0u);
        KeyUp[5] = (0u);
        KeyUp[6] = (0u);
        KeyUp[7] = (0u);
        LastKeyDown = (0u);
        LastKeyUp = (0u);
    #endif
    }

    Keyboard::State Keyboard::GetState(bool new_frame)
    {
        Keyboard::State Copy = {};
        GetState(Copy, new_frame);
        return Copy;
    }
    void Keyboard::GetState(Keyboard::State& Ref, bool new_frame)
    {
    #if PLATFORM_KEYBOARD_USING_ATOMIC
        Ref.KeyState.Data[0] = KeyState[0].load();
        Ref.KeyState.Data[1] = KeyState[1].load();
        Ref.KeyState.Data[2] = KeyState[2].load();
        Ref.KeyState.Data[3] = KeyState[3].load();
        Ref.KeyState.Data[4] = KeyState[4].load();
        Ref.KeyState.Data[5] = KeyState[5].load();
        Ref.KeyState.Data[6] = KeyState[6].load();
        Ref.KeyState.Data[7] = KeyState[7].load();
        if (new_frame)
        {
            Ref.KeyDown.Data[0] = KeyDown[0].exchange(0u);
            Ref.KeyDown.Data[1] = KeyDown[1].exchange(0u);
            Ref.KeyDown.Data[2] = KeyDown[2].exchange(0u);
            Ref.KeyDown.Data[3] = KeyDown[3].exchange(0u);
            Ref.KeyDown.Data[4] = KeyDown[4].exchange(0u);
            Ref.KeyDown.Data[5] = KeyDown[5].exchange(0u);
            Ref.KeyDown.Data[6] = KeyDown[6].exchange(0u);
            Ref.KeyDown.Data[7] = KeyDown[7].exchange(0u);
            Ref.KeyUp.Data[0] = KeyUp[0].exchange(0u);
            Ref.KeyUp.Data[1] = KeyUp[1].exchange(0u);
            Ref.KeyUp.Data[2] = KeyUp[2].exchange(0u);
            Ref.KeyUp.Data[3] = KeyUp[3].exchange(0u);
            Ref.KeyUp.Data[4] = KeyUp[4].exchange(0u);
            Ref.KeyUp.Data[5] = KeyUp[5].exchange(0u);
            Ref.KeyUp.Data[6] = KeyUp[6].exchange(0u);
            Ref.KeyUp.Data[7] = KeyUp[7].exchange(0u);
            Ref.LastKeyDown = static_cast<Key>(LastKeyDown.exchange(0));
            Ref.LastKeyUp = static_cast<Key>(LastKeyUp.exchange(0));
        }
        else
        {
            Ref.KeyDown.Data[0] = KeyDown[0].load();
            Ref.KeyDown.Data[1] = KeyDown[1].load();
            Ref.KeyDown.Data[2] = KeyDown[2].load();
            Ref.KeyDown.Data[3] = KeyDown[3].load();
            Ref.KeyDown.Data[4] = KeyDown[4].load();
            Ref.KeyDown.Data[5] = KeyDown[5].load();
            Ref.KeyDown.Data[6] = KeyDown[6].load();
            Ref.KeyDown.Data[7] = KeyDown[7].load();
            Ref.KeyUp.Data[0] = KeyUp[0].load();
            Ref.KeyUp.Data[1] = KeyUp[1].load();
            Ref.KeyUp.Data[2] = KeyUp[2].load();
            Ref.KeyUp.Data[3] = KeyUp[3].load();
            Ref.KeyUp.Data[4] = KeyUp[4].load();
            Ref.KeyUp.Data[5] = KeyUp[5].load();
            Ref.KeyUp.Data[6] = KeyUp[6].load();
            Ref.KeyUp.Data[7] = KeyUp[7].load();
            Ref.LastKeyDown = static_cast<Key>(LastKeyDown.load());
            Ref.LastKeyUp = static_cast<Key>(LastKeyUp.load());
        }
    #else
        Ref.KeyState.Data[0] = KeyState[0];
        Ref.KeyState.Data[1] = KeyState[1];
        Ref.KeyState.Data[2] = KeyState[2];
        Ref.KeyState.Data[3] = KeyState[3];
        Ref.KeyState.Data[4] = KeyState[4];
        Ref.KeyState.Data[5] = KeyState[5];
        Ref.KeyState.Data[6] = KeyState[6];
        Ref.KeyState.Data[7] = KeyState[7];
        Ref.KeyDown.Data[0] = KeyDown[0];
        Ref.KeyDown.Data[1] = KeyDown[1];
        Ref.KeyDown.Data[2] = KeyDown[2];
        Ref.KeyDown.Data[3] = KeyDown[3];
        Ref.KeyDown.Data[4] = KeyDown[4];
        Ref.KeyDown.Data[5] = KeyDown[5];
        Ref.KeyDown.Data[6] = KeyDown[6];
        Ref.KeyDown.Data[7] = KeyDown[7];
        Ref.KeyUp.Data[0] = KeyUp[0];
        Ref.KeyUp.Data[1] = KeyUp[1];
        Ref.KeyUp.Data[2] = KeyUp[2];
        Ref.KeyUp.Data[3] = KeyUp[3];
        Ref.KeyUp.Data[4] = KeyUp[4];
        Ref.KeyUp.Data[5] = KeyUp[5];
        Ref.KeyUp.Data[6] = KeyUp[6];
        Ref.KeyUp.Data[7] = KeyUp[7];
        Ref.LastKeyDown = static_cast<Key>(LastKeyDown);
        Ref.LastKeyUp = static_cast<Key>(LastKeyUp);
        if (new_frame)
        {
            ResetFrame();
        }
    #endif
    }

    void Keyboard::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        std::ignore = hWnd;
        bool down = false;
        switch (uMsg)
        {
        default:
            return; // ignore other message
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
            Reset();
            return; // skip
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            down = true;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            down = false;
            break;
        }
        if (wParam > 0xFFu)
        {
            return; // ignore unknown key code
        }
        uint8_t const vk = wParam & 0xFFu;
        uint8_t vk_translated = vk;
        switch (vk)
        {
        case VK_RETURN:
            vk_translated = IsExtendedKey(lParam) ? VK_NUMPADENTER : VK_RETURN;
            break;
        case VK_SHIFT:
            vk_translated = IsExtendedKey(lParam) ? VK_RSHIFT : VK_LSHIFT;
            if (!down)
            {
                // Windows system feature :(
                AtomicSetKey(KeyState, VK_RSHIFT, false);
                AtomicSetKey(KeyState, VK_LSHIFT, false);
            }
            break;
        case VK_CONTROL:
            vk_translated = IsExtendedKey(lParam) ? VK_RCONTROL : VK_LCONTROL;
            break;
        case VK_MENU:
            vk_translated = IsExtendedKey(lParam) ? VK_RMENU : VK_LMENU;
            break;
        }
        AtomicSetKey(KeyState, vk, down);
        if (vk != vk_translated)
        {
            AtomicSetKey(KeyState, vk_translated, down);
        }
        if (down)
        {
            AtomicSetKey(KeyDown, vk_translated, true);
        #if PLATFORM_KEYBOARD_USING_ATOMIC
            LastKeyDown.store(vk_translated);
        #else
            LastKeyDown = (vk_translated);
        #endif
        }
        else
        {
            AtomicSetKey(KeyUp, vk_translated, true);
        #if PLATFORM_KEYBOARD_USING_ATOMIC
            LastKeyUp.store(vk_translated);
        #else
            LastKeyUp = (vk_translated);
        #endif
        }
    }
}
