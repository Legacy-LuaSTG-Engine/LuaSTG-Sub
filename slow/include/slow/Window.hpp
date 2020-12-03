#pragma once
#include "slow/Ref.hpp"
#include "slow/Type.hpp"
#include <cstdint>

namespace slow {
    // note that the rect using Window coord system
    struct MonitorInfo {
        bool primary;
        struct {
            int32_t left;
            int32_t right;
            int32_t top;
            int32_t bottom;
        } rect;
        struct {
            int32_t left;
            int32_t right;
            int32_t top;
            int32_t bottom;
        } work;
    };
    
    struct MonitorHelper : public Ref {
        virtual bool refresh() = 0;
        virtual uint32_t getSize() = 0;
        virtual MonitorInfo getInfo(uint32_t index) = 0;
        
        static bool create(MonitorHelper** output);
        static MonitorHelper* create();
    };
    
    enum class WindowStyle : uint32_t {
        Normal = 0x00000000,
        Fixed = 0x00000001,
        Borderless = 0x00000002,
        Invalid = 0xFFFFFFFF,
    };
    
    enum class WindowLayer : uint32_t {
        Normal = 0x00000000,
        Top = 0x00000001,
        Bottom = 0x00000002,
        Topmost = 0x00000004,
        Invalid = 0xFFFFFFFF,
    };
    
    enum class PointerBotton : uint8_t {
        None = 0x00,
        
        Left = 0x01,
        Right = 0x02,
        Middle = 0x04,
        X1 = 0x05,
        X2 = 0x06,
    };
    
    enum class KeyboardBotton : uint8_t {
        None = 0x00,
        
        Cancel = 0x03,
        Backspace = 0x08,
        Tab = 0x09,
        Clear = 0x0C,
        Enter = 0x0D,
        //Shift = 0x10,
        //Control = 0x11,
        //Menu = 0x12,
        Pause = 0x13,
        CapsLock = 0x14,
        Escape = 0x1B,
        Space = 0x20,
        PageUp = 0x21,
        PageDown = 0x22,
        End = 0x23,
        Home = 0x24,
        
        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,
        
        Select = 0x29,
        Print = 0x2A,
        Execute = 0x2B,
        PrintScreen = 0x2C,
        Insert = 0x2D,
        Delete = 0x2E,
        Help = 0x2F,
        
        Number0 = 0x30,
        Number1 = 0x31,
        Number2 = 0x32,
        Number3 = 0x33,
        Number4 = 0x34,
        Number5 = 0x35,
        Number6 = 0x36,
        Number7 = 0x37,
        Number8 = 0x38,
        Number9 = 0x39,
        
        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4A,
        K = 0x4B,
        L = 0x4C,
        M = 0x4D,
        N = 0x4E,
        O = 0x4F,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5A,
        
        LeftWindow = 0x5B,
        RightWindow = 0x5C,
        Applications = 0x5D,
        Sleep = 0x5F,
        
        NumPad0 = 0x60,
        NumPad1 = 0x61,
        NumPad2 = 0x62,
        NumPad3 = 0x63,
        NumPad4 = 0x64,
        NumPad5 = 0x65,
        NumPad6 = 0x66,
        NumPad7 = 0x67,
        NumPad8 = 0x68,
        NumPad9 = 0x69,
        
        // *
        Multiply = 0x6A,
        // +
        Add = 0x6B,
        Separator = 0x6C,
        // -
        Subtract = 0x6D,
        // .
        Decimal = 0x6E,
        // /
        Divide = 0x6F,
        
        F1 = 0x70,
        F2 = 0x71,
        F3 = 0x72,
        F4 = 0x73,
        F5 = 0x74,
        F6 = 0x75,
        F7 = 0x76,
        F8 = 0x77,
        F9 = 0x78,
        F10 = 0x79,
        F11 = 0x7A,
        F12 = 0x7B,
        F13 = 0x7C,
        F14 = 0x7D,
        F15 = 0x7E,
        F16 = 0x7F,
        F17 = 0x80,
        F18 = 0x81,
        F19 = 0x82,
        F20 = 0x83,
        F21 = 0x84,
        F22 = 0x85,
        F23 = 0x86,
        F24 = 0x87,
        
        NumLock = 0x90,
        ScrollLock = 0x91,
        LeftShift = 0xA0,
        RightShift = 0xA1,
        LeftControl = 0xA2,
        RightControl = 0xA3,
        LeftMenu = 0xA4,
        RightMenu = 0xA5,
        
        MediaNextTrack = 0xB0,
        MediaPreviousTrack = 0xB1,
        MediaStop = 0xB2,
        MediaPlayPause = 0xB3,
        
        // ;:
        Semicolon  = 0xBA,
        // =+
        Equal = 0xBB,
        // ,<
        Comma = 0xBC,
        // -_
        Minus = 0xBD,
        // .>
        Period = 0xBE,
        // /?
        Slash = 0xBF,
        // `~
        GraveAccent = 0xC0,
        // [{
        LeftBracket = 0xDB,
        // \\|
        Backslash = 0xDC,
        // ]}
        RightBracket = 0xDD,
        // '"
        Quote = 0xDE,
    };
    
    struct WindowCallback {
        virtual void onTimer() = 0;
        virtual void onSize(uint32_t width, uint32_t height) = 0;
    };
    
    struct Window : public Ref {
        // window
        
        virtual bool setTitle(const char* title) = 0;
        virtual const char* getTitle() = 0;
        virtual bool setStyle(WindowStyle style) = 0;
        virtual WindowStyle getStyle() = 0;
        virtual bool setSize(Vector2I size) = 0;
        virtual Vector2I getSize() = 0;
        virtual Vector2I getActualSize() = 0;
        virtual bool setLayer(WindowLayer layer) = 0;
        virtual WindowLayer getLayer() = 0;
        virtual bool setCentered() = 0;
        
        // input
        
        virtual bool setPointerHide(bool hide) = 0;
        virtual bool getPointerHide() = 0;
        virtual bool setPointerPosition(Vector2I position) = 0;
        virtual Vector2I getPointerPosition() = 0;
        virtual int32_t getPointerWheel() = 0;
        virtual bool getPointerBotton(PointerBotton botton) = 0;
        virtual bool getKeyboardBotton(KeyboardBotton botton) = 0;
        
        // window message loop
        
        virtual void setCallback(WindowCallback* callback) = 0;
        virtual void setWindowClose(bool v) = 0;
        virtual bool shouldWindowClose() = 0;
        virtual bool dispatchMessage() = 0;
        
        // window manage
        
        virtual bool open(const char* title, Vector2I size, WindowStyle style, WindowLayer layer) = 0;
        virtual void close() = 0;
        
        // native
        
        virtual bool open(const wchar_t* title, Vector2I size, WindowStyle style, WindowLayer layer) = 0;
        virtual bool setTitleW(const wchar_t* title) = 0;
        virtual const wchar_t* getTitleW() = 0;
        virtual ptrdiff_t getNativeWindow() = 0;
        
        static bool create(Window** output);
        static Window* create();
    };
};
