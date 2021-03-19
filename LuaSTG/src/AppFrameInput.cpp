#include "AppFrame.h"
#include "f2dKeyCodeConvert.inl"

namespace LuaSTGPlus
{
    void AppFrame::resetKeyStatus()LNOEXCEPT
    {
        m_LastKey = 0;
        m_InputTextBuffer.clear();
        ::memset(m_KeyStateMap, 0, sizeof(m_KeyStateMap));
    }
    
    fBool AppFrame::GetKeyState(int VKCode)LNOEXCEPT
    {
        if (VKCode > 0 && VKCode < _countof(m_KeyStateMap))
        {
            if (m_Keyboard) {
                return m_Keyboard->IsKeyDown(VKKeyToF2DKey(VKCode));
            }
            else {
                //return m_KeyStateMap[VKCode] || GetAsyncKeyState(VKCode);
                return m_KeyStateMap[VKCode];
            }
        }
        return false;
    }
    
    int AppFrame::GetLastKey()LNOEXCEPT
    {
        return m_LastKey;
    }
    
    fBool AppFrame::GetKeyboardState(DWORD VKCode)LNOEXCEPT
    {
        if (m_Keyboard2) {
            return m_Keyboard2->KeyPress(VKCode);
        }
        else {
            return false;
        }
    }
    
    bool AppFrame::GetAsyncKeyState(int VKCode)LNOEXCEPT
    {
        SHORT KeyState = ::GetAsyncKeyState(VKCode);
        if (KeyState & 0x8000) {
            return true;
        }
        else {
            return false;
        }
    }
    
    fcStrW AppFrame::GetTextInput()LNOEXCEPT
    {
        return m_InputTextBuffer.c_str();
    }
    
    void AppFrame::ClearTextInput()LNOEXCEPT
    {
        m_InputTextBuffer.clear();
    }
    
    fcyVec2 AppFrame::GetMousePosition(bool no_flip)LNOEXCEPT
    {
        if (no_flip)
            return m_MousePosition;
        else
            return m_MousePosition_old;
    }
    
    fInt AppFrame::GetMouseWheelDelta()LNOEXCEPT
    {
        return m_Mouse->GetOffsetZ();
    }
    
    fBool AppFrame::GetMouseState(int button)LNOEXCEPT
    {
        switch (button) {
        case 0:
            return m_Mouse->IsLeftBtnDown();
        case 1:
            return m_Mouse->IsMiddleBtnDown();
        case 2:
            return m_Mouse->IsRightBtnDown();
        default:
            break;
        }
        if (button >= 3 && button <= 7) {
            return m_Mouse->IsAdditionBtnDown(button - 3);//对齐额外键索引（不包含左中右键）
        }
        return false;
    }
    
};
