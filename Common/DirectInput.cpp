#include "Common/DirectInput.h"

#include <vector>

#include <Windows.h>
#include <wrl.h>
#include <Xinput.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL          0x80000000
#endif

#include "Common/DebugLog.hpp"
#ifndef NDEBUG
#define _LOGDEBUG(fmt, ...) DebugLog(fmt, __VA_ARGS__)
#else
#define _LOGDEBUG(fmt, ...)
#endif

// MAGIC CODE
BOOL IsXInputDevice(const GUID* pGuidProductFromDirectInput);

// DirectInput Data & Format
namespace native
{
    // interface
    static const GUID g_IID_IDirectInput8W = {0xBF798031, 0x483A, 0x4DA2, {0xAA, 0x99, 0x5D, 0x64, 0xED, 0x36, 0x97, 0x00}};
    
    // device object
    static const GUID g_GUID_XAxis   = {0xA36D02E0, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_YAxis   = {0xA36D02E1, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_ZAxis   = {0xA36D02E2, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_RxAxis  = {0xA36D02F4, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_RyAxis  = {0xA36D02F5, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_RzAxis  = {0xA36D02E3, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_Slider  = {0xA36D02E4, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_Button  = {0xA36D02F0, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_Key     = {0x55728220, 0xD33C, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_POV     = {0xA36D02F2, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    static const GUID g_GUID_Unknown = {0xA36D02F3, 0xC9F3, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
    
    // common Keyboard data format
    static DIOBJECTDATAFORMAT g_dfDIKeyboardObject[256] = {
        { &g_GUID_Key, 0x00, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x00) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x01, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x01) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x02, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x02) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x03, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x03) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x04, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x04) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x05, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x05) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x06, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x06) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x07, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x07) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x08, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x08) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x09, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x09) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x0A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x0A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x0B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x0B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x0C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x0C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x0D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x0D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x0E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x0E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x0F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x0F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x10, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x10) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x11, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x11) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x12, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x12) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x13, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x13) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x14, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x14) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x15, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x15) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x16, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x16) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x17, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x17) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x18, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x18) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x19, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x19) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x1A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x1A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x1B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x1B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x1C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x1C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x1D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x1D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x1E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x1E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x1F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x1F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x20, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x20) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x21, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x21) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x22, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x22) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x23, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x23) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x24, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x24) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x25, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x25) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x26, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x26) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x27, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x27) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x28, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x28) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x29, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x29) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x2A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x2A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x2B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x2B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x2C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x2C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x2D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x2D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x2E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x2E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x2F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x2F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x30, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x30) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x31, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x31) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x32, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x32) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x33, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x33) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x34, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x34) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x35, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x35) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x36, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x36) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x37, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x37) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x38, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x38) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x39, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x39) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x3A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x3A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x3B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x3B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x3C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x3C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x3D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x3D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x3E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x3E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x3F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x3F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x40, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x40) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x41, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x41) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x42, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x42) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x43, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x43) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x44, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x44) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x45, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x45) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x46, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x46) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x47, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x47) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x48, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x48) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x49, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x49) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x4A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x4A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x4B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x4B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x4C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x4C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x4D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x4D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x4E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x4E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x4F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x4F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x50, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x50) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x51, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x51) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x52, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x52) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x53, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x53) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x54, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x54) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x55, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x55) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x56, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x56) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x57, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x57) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x58, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x58) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x59, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x59) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x5A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x5A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x5B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x5B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x5C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x5C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x5D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x5D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x5E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x5E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x5F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x5F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x60, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x60) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x61, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x61) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x62, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x62) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x63, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x63) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x64, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x64) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x65, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x65) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x66, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x66) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x67, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x67) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x68, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x68) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x69, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x69) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x6A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x6A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x6B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x6B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x6C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x6C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x6D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x6D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x6E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x6E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x6F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x6F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x70, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x70) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x71, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x71) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x72, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x72) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x73, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x73) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x74, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x74) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x75, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x75) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x76, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x76) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x77, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x77) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x78, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x78) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x79, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x79) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x7A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x7A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x7B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x7B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x7C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x7C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x7D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x7D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x7E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x7E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x7F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x7F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x80, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x80) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x81, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x81) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x82, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x82) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x83, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x83) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x84, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x84) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x85, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x85) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x86, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x86) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x87, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x87) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x88, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x88) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x89, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x89) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x8A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x8A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x8B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x8B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x8C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x8C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x8D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x8D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x8E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x8E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x8F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x8F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x90, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x90) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x91, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x91) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x92, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x92) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x93, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x93) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x94, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x94) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x95, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x95) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x96, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x96) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x97, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x97) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x98, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x98) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x99, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x99) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x9A, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x9A) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x9B, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x9B) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x9C, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x9C) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x9D, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x9D) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x9E, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x9E) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0x9F, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0x9F) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA0, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA0) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA1, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA1) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA2, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA2) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA3, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA3) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA4, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA4) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA5, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA5) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA6, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA6) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA7, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA7) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA8, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA8) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xA9, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xA9) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xAA, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xAA) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xAB, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xAB) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xAC, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xAC) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xAD, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xAD) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xAE, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xAE) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xAF, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xAF) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB0, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB0) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB1, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB1) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB2, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB2) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB3, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB3) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB4, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB4) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB5, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB5) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB6, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB6) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB7, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB7) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB8, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB8) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xB9, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xB9) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xBA, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xBA) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xBB, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xBB) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xBC, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xBC) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xBD, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xBD) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xBE, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xBE) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xBF, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xBF) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC0, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC0) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC1, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC1) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC2, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC2) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC3, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC3) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC4, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC4) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC5, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC5) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC6, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC6) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC7, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC7) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC8, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC8) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xC9, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xC9) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xCA, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xCA) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xCB, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xCB) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xCC, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xCC) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xCD, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xCD) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xCE, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xCE) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xCF, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xCF) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD0, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD0) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD1, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD1) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD2, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD2) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD3, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD3) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD4, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD4) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD5, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD5) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD6, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD6) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD7, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD7) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD8, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD8) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xD9, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xD9) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xDA, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xDA) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xDB, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xDB) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xDC, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xDC) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xDD, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xDD) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xDE, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xDE) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xDF, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xDF) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE0, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE0) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE1, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE1) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE2, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE2) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE3, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE3) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE4, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE4) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE5, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE5) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE6, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE6) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE7, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE7) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE8, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE8) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xE9, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xE9) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xEA, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xEA) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xEB, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xEB) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xEC, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xEC) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xED, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xED) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xEE, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xEE) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xEF, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xEF) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF0, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF0) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF1, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF1) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF2, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF2) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF3, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF3) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF4, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF4) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF5, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF5) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF6, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF6) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF7, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF7) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF8, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF8) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xF9, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xF9) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xFA, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xFA) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xFB, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xFB) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xFC, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xFC) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xFD, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xFD) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xFE, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xFE) | DIDFT_BUTTON, 0 },
        { &g_GUID_Key, 0xFF, DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(0xFF) | DIDFT_BUTTON, 0 },
    };
    static DIDATAFORMAT g_dfDIKeyboard = {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_RELAXIS,
        256,
        256,
        g_dfDIKeyboardObject,
    };
    
    // common Mouse data format
    static DIOBJECTDATAFORMAT g_dfDIMouseObject[11] = {
        { &g_GUID_XAxis,  0, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , 0 },
        { &g_GUID_YAxis,  4, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , 0 },
        { &g_GUID_ZAxis,  8, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , 0 },
        { NULL         , 12, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 13, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 14, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 15, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 16, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 17, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 18, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
        { NULL         , 19, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON, 0 },
    };
    static DIDATAFORMAT g_dfDIMouse = {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_RELAXIS,
        sizeof(DIMOUSESTATE),
        7,
        g_dfDIMouseObject,
    };
    static DIDATAFORMAT g_dfDIMouse2 = {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_RELAXIS,
        sizeof(DIMOUSESTATE2),
        11,
        g_dfDIMouseObject,
    };
    
    // common Controller data format
    static DIOBJECTDATAFORMAT g_dfDIJoystickObject[44] = {
        { &g_GUID_XAxis ,  0, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_YAxis ,  4, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_ZAxis ,  8, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_RxAxis, 12, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_RyAxis, 16, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_RzAxis, 20, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_Slider, 24, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_Slider, 28, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_AXIS  , DIDOI_ASPECTPOSITION },
        { &g_GUID_POV   , 32, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_POV   ,                    0 },
        { &g_GUID_POV   , 36, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_POV   ,                    0 },
        { &g_GUID_POV   , 40, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_POV   ,                    0 },
        { &g_GUID_POV   , 44, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_POV   ,                    0 },
        { NULL          , 48, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 49, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 50, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 51, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 52, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 53, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 54, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 55, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 56, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 57, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 58, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 59, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 60, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 61, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 62, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 63, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 64, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 65, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 66, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 67, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 68, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 69, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 70, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 71, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 72, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 73, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 74, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 75, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 76, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 77, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 78, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
        { NULL          , 79, DIDFT_OPTIONAL | DIDFT_ANYINSTANCE | DIDFT_BUTTON,                    0 },
    };
    static DIDATAFORMAT g_dfDIJoystick = {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_ABSAXIS,
        sizeof(DIJOYSTATE),
        44,
        g_dfDIJoystickObject,
    };
};

// DirectInput
namespace native
{
    constexpr DWORD g_dwControllerBufferSize = 64;
    
    struct DirectInput::_Data
    {
        HWND window = NULL;
        HMODULE dll = NULL;
        Microsoft::WRL::ComPtr<IDirectInput8W> dinput;
        std::vector<DIDEVICEINSTANCEW> gamepad_device;
        std::vector<Microsoft::WRL::ComPtr<IDirectInputDevice8W>> gamepad;
        std::vector<AxisRange> gamepad_prop;
        std::vector<DIJOYSTATE> gamepad_state;
    };
    
    static bool _excludeXInput = true;
    static BOOL CALLBACK _listGamepads(LPCDIDEVICEINSTANCEW device, LPVOID data)
    {
        _LOGDEBUG(L"InstanceName:%s ProductName:%s UsagePage:%u Usage:%u\n",
            device->tszInstanceName, device->tszProductName,
            device->wUsagePage, device->wUsage);
        #define case_print(x) case x: { _LOGDEBUG(L"    " L#x L"\n"); break; }
        switch (device->dwDevType & 0xFF)
        {
            case_print(DI8DEVTYPE_KEYBOARD);
            case_print(DI8DEVTYPE_MOUSE);
            case_print(DI8DEVTYPE_SCREENPOINTER);
            case_print(DI8DEVTYPE_JOYSTICK);
            case_print(DI8DEVTYPE_GAMEPAD);
            case_print(DI8DEVTYPE_FLIGHT);
            case_print(DI8DEVTYPE_DRIVING);
            case_print(DI8DEVTYPE_SUPPLEMENTAL);
            case_print(DI8DEVTYPE_1STPERSON);
            case_print(DI8DEVTYPE_DEVICECTRL);
            case_print(DI8DEVTYPE_DEVICE);
            case_print(DI8DEVTYPE_REMOTE);
            default: { _LOGDEBUG(L"    DI8DEVTYPE_UNKNOWN\n"); break; }
        };
        #undef case_print
        const bool isXInput = IsXInputDevice(&device->guidProduct);
        if (_excludeXInput && isXInput)
        {
            _LOGDEBUG(L"    XInput support\n");
        }
        else
        {
            auto* vec = (std::vector<DIDEVICEINSTANCEW>*)data;
            vec->push_back(*device);
            if (!_excludeXInput)
            {
                vec->back().dwSize = isXInput ? 1 : 0;
            }
            else
            {
                vec->back().dwSize = 0;
            }
        }
        return DIENUM_CONTINUE; // DIENUM_STOP
    };
    
    template<typename T>
    inline T clamp(T v, T a, T b)
    {
        if (a > b)
        {
            const T c = a;
            a = b;
            b = c;
        }
        v = (v > a) ? v : a;
        v = (v < b) ? v : b;
        return v;
    }
    inline bool _initGamepad(HWND window, IDirectInputDevice8W* device, DirectInput::AxisRange& range)
    {
        HRESULT hr = 0;
        
        hr = device->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        if (hr != DI_OK)
        {
            _LOGDEBUG(L"game controller SetCooperativeLevel failed\n");
        }
        hr = device->SetDataFormat(&g_dfDIJoystick);
        if (hr != DI_OK)
        {
            _LOGDEBUG(L"game controller SetDataFormat failed\n");
        }
        DIPROPDWORD bufferProperty;
        bufferProperty.diph.dwSize = sizeof(DIPROPDWORD);
        bufferProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        bufferProperty.diph.dwObj = 0;
        bufferProperty.diph.dwHow = DIPH_DEVICE;
        bufferProperty.dwData = g_dwControllerBufferSize;
        hr = device->SetProperty(DIPROP_BUFFERSIZE, &bufferProperty.diph);
        if (hr != DI_OK)
        {
            _LOGDEBUG(L"game controller SetProperty failed\n");
        }
        
        DIPROPRANGE axisRange;
        axisRange.diph.dwSize= sizeof(DIPROPRANGE);
        axisRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        axisRange.diph.dwHow = DIPH_BYOFFSET;
        
        // DO NOT USING SetProperty(DIPROP_RANGE) because it will fxxk XInput
        /*
        // set
        axisRange.lMin = -32768;
        axisRange.lMax = 32767;
        
        // X
        axisRange.diph.dwObj = DIJOFS_X;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // Y
        axisRange.diph.dwObj = DIJOFS_Y;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // Z
        axisRange.diph.dwObj = DIJOFS_Z;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);

        // RZ
        axisRange.diph.dwObj = DIJOFS_RX;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // RY
        axisRange.diph.dwObj = DIJOFS_RY;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // RZ
        axisRange.diph.dwObj = DIJOFS_RZ;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        //*/
        
        // get
        axisRange.lMin = 0;
        axisRange.lMax = 0;
        
        // X
        axisRange.diph.dwObj = DIJOFS_X;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.XMin = axisRange.lMin;
            range.XMax = axisRange.lMax;
        }
        // Y
        axisRange.diph.dwObj = DIJOFS_Y;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.YMin = axisRange.lMin;
            range.YMax = axisRange.lMax;
        }
        // Z
        axisRange.diph.dwObj = DIJOFS_Z;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.ZMin = axisRange.lMin;
            range.ZMax = axisRange.lMax;
        }
        
        // RX
        axisRange.diph.dwObj = DIJOFS_RX;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.RxMin = axisRange.lMin;
            range.RxMax = axisRange.lMax;
        }
        // RY
        axisRange.diph.dwObj = DIJOFS_RY;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.RyMin = axisRange.lMin;
            range.RyMax = axisRange.lMax;
        }
        // Z
        axisRange.diph.dwObj = DIJOFS_RZ;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.RzMin = axisRange.lMin;
            range.RzMax = axisRange.lMax;
        }
        
        // Slider0
        axisRange.diph.dwObj = DIJOFS_SLIDER(0);
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.Slider0Min = axisRange.lMin;
            range.Slider0Max = axisRange.lMax;
        }
        // Slider1
        axisRange.diph.dwObj = DIJOFS_SLIDER(1);
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.Slider1Min = axisRange.lMin;
            range.Slider1Max = axisRange.lMax;
        }
        
        hr = device->Acquire();
        if (!(hr == DI_OK || hr == S_FALSE))
        {
            _LOGDEBUG(L"game controller first Acquire failed\n");
        }
        
        return true;
    }
    inline void _updateGamepad(IDirectInputDevice8W* device, DIJOYSTATE& state, size_t idx)
    {
        DIDEVICEOBJECTDATA data[g_dwControllerBufferSize];
        DWORD data_n = g_dwControllerBufferSize;
        
        HRESULT hr = 0;
        hr = device->Acquire(); // get device access
        if (hr == DI_OK || hr == S_FALSE)
        {
            hr = device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &data_n, 0);
            if (hr == DI_OK || hr == DI_BUFFEROVERFLOW)
            {
                if (data_n > 0)
                {
                    _LOGDEBUG("Gamepad[%u] recive %u data\n", idx, data_n);
                    // process data
                    for (size_t i = 0; i < data_n; i += 1)
                    {
                        // I know, I know...
                        #pragma warning(disable : 4644)
                        switch (data[i].dwOfs)
                        {
                        case DIJOFS_X:
                            state.lX = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_Y:
                            state.lY = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_Z:
                            state.lZ = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_RX:
                            state.lRx = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_RY:
                            state.lRy = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_RZ:
                            state.lRz = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_SLIDER(0):
                            state.rglSlider[0] = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_SLIDER(1):
                            state.rglSlider[1] = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_POV(0):
                            state.rgdwPOV[0] = data[i].dwData;
                            break;
                        case DIJOFS_POV(1):
                            state.rgdwPOV[1] = data[i].dwData;
                            break;
                        case DIJOFS_POV(2):
                            state.rgdwPOV[2] = data[i].dwData;
                            break;
                        case DIJOFS_POV(3):
                            state.rgdwPOV[3] = data[i].dwData;
                            break;
                        default:
                            if (data[i].dwOfs >= DIJOFS_BUTTON(0) && data[i].dwOfs <= DIJOFS_BUTTON(31))
                            {
                                state.rgbButtons[data[i].dwOfs - DIJOFS_BUTTON(0)] = ((data[i].dwData & 0x80) != 0);
                            }
                            break;
                        }
                        #pragma warning(default : 4644)
                    }
                }
            }
            else
            {
                _LOGDEBUG(L"Gamepad[%u] GetDeviceData failed\n", idx);
            }
        }
        else
        {
            _LOGDEBUG(L"Gamepad[%u] Acquire failed\n", idx);
        }
    }
    
    #define getself() _Data& self = *_data;
    
    uint32_t DirectInput::count()
    {
        getself();
        return (uint32_t)self.gamepad.size();
    }
    uint32_t DirectInput::refresh()
    {
        getself();
        clear(); // clear first
        if (self.dinput)
        {
            HRESULT hr = 0;
            std::vector<DIDEVICEINSTANCEW> raw_device;
            // list all Gamepads
            // *
            // DI8DEVCLASS_ALL
            // DI8DEVCLASS_GAMECTRL
            // DI8DEVCLASS_KEYBOARD
            // DI8DEVCLASS_POINTER
            // *
            // DIEDFL_ALLDEVICES
            // DIEDFL_ATTACHEDONLY
            _excludeXInput = false;
            hr = self.dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, &_listGamepads, &raw_device, DI8DEVCLASS_ALL);
            if (hr != DI_OK)
            {
                _LOGDEBUG(L"EnumDevices failed\n");
            }
            // create Gamepad devices
            for (auto& v : raw_device)
            {
                Microsoft::WRL::ComPtr<IDirectInputDevice8W> device;
                hr = self.dinput->CreateDevice(v.guidInstance, device.GetAddressOf(), NULL);
                if (hr == DI_OK)
                {
                    AxisRange range;
                    ZeroMemory(&range, sizeof(range));
                    _initGamepad(self.window, device.Get(), range);
                    self.gamepad_device.push_back(v);
                    self.gamepad.push_back(device);
                    self.gamepad_prop.push_back(range);
                    self.gamepad_state.push_back({});
                }
                else
                {
                    _LOGDEBUG(L"CreateDevice game controller failed\n");
                }
            }
        }
        else
        {
            _LOGDEBUG(L"dinput8 NULL exception\n");
        }
        reset();
        return (uint32_t)self.gamepad.size();
    }
    void DirectInput::update()
    {
        getself();
        for (size_t idx = 0; idx < self.gamepad.size(); idx += 1)
        {
            _updateGamepad(self.gamepad[idx].Get(), self.gamepad_state[idx], idx);
        }
    }
    void DirectInput::reset()
    {
        getself();
        for (size_t idx = 0; idx < self.gamepad.size(); idx += 1)
        {
            auto& range = self.gamepad_prop[idx];
            auto& state = self.gamepad_state[idx];
            
            #define centered(_X, _A, _B) state._X = (range._B - range._A) / 2;
            centered(lX, XMin, XMax);
            centered(lY, YMin, YMax);
            centered(lZ, ZMin, ZMax);
            centered(lRx, RxMin, RxMax);
            centered(lRy, RyMin, RyMax);
            centered(lRz, RzMin, RzMax);
            #undef centered
            
            ZeroMemory(&state.rglSlider, sizeof(state.rglSlider));
            
            state.rgdwPOV[0] = 0xFFFFFFFF;
            state.rgdwPOV[1] = 0xFFFFFFFF;
            state.rgdwPOV[2] = 0xFFFFFFFF;
            state.rgdwPOV[3] = 0xFFFFFFFF;
            
            ZeroMemory(&state.rgbButtons, sizeof(state.rgbButtons));
        }
    }
    void DirectInput::clear()
    {
        getself();
        self.gamepad_device.clear();
        self.gamepad.clear();
        self.gamepad_prop.clear();
        self.gamepad_state.clear();
    }
    bool DirectInput::getAxisRange(uint32_t index, DirectInput::AxisRange* range)
    {
        getself();
        if (index < self.gamepad_prop.size())
        {
            CopyMemory(range, &self.gamepad_prop[index], sizeof(DirectInput::AxisRange));
            return true;
        }
        return false;
    }
    bool DirectInput::getRawState(uint32_t index, DirectInput::RawState* state)
    {
        getself();
        if (index < self.gamepad_state.size())
        {
            CopyMemory(state, &self.gamepad_state[index], sizeof(DirectInput::RawState));
            return true;
        }
        return false;
    }
    bool DirectInput::getState(uint32_t index, DirectInput::State* state)
    {
        getself();
        if (index < self.gamepad_state.size())
        {
            auto& device = self.gamepad_device[index];
            auto& raw = self.gamepad_state[index];
            auto& range = self.gamepad_prop[index];
            auto& _state = *state;
            
            _state.wButtons = 0;
            
            if (device.dwSize == 0)
            {
                if (raw.rgbButtons[0]) { _state.wButtons |= XINPUT_GAMEPAD_X; }
                if (raw.rgbButtons[1]) { _state.wButtons |= XINPUT_GAMEPAD_A; }
                if (raw.rgbButtons[2]) { _state.wButtons |= XINPUT_GAMEPAD_B; }
                if (raw.rgbButtons[3]) { _state.wButtons |= XINPUT_GAMEPAD_Y; }
                
                if (raw.rgbButtons[4]) { _state.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER; }
                if (raw.rgbButtons[5]) { _state.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER; }
                
                if (raw.rgbButtons[6]) { _state.bLeftTrigger  = 255; } else { _state.bLeftTrigger  = 0; }
                if (raw.rgbButtons[7]) { _state.bRightTrigger = 255; } else { _state.bRightTrigger = 0; }
                
                if (raw.rgbButtons[8]) { _state.wButtons |= XINPUT_GAMEPAD_BACK; }
                if (raw.rgbButtons[9]) { _state.wButtons |= XINPUT_GAMEPAD_START; }
                
                if (raw.rgbButtons[10]) { _state.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB; }
                if (raw.rgbButtons[11]) { _state.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB; }
            }
            else
            {
                if (raw.rgbButtons[0]) { _state.wButtons |= XINPUT_GAMEPAD_A; }
                if (raw.rgbButtons[1]) { _state.wButtons |= XINPUT_GAMEPAD_B; }
                if (raw.rgbButtons[2]) { _state.wButtons |= XINPUT_GAMEPAD_X; }
                if (raw.rgbButtons[3]) { _state.wButtons |= XINPUT_GAMEPAD_Y; }
                
                if (raw.rgbButtons[4]) { _state.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER; }
                if (raw.rgbButtons[5]) { _state.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER; }
                
                if (raw.rgbButtons[6]) { _state.wButtons |= XINPUT_GAMEPAD_BACK; }
                if (raw.rgbButtons[7]) { _state.wButtons |= XINPUT_GAMEPAD_START; }
                
                if (raw.rgbButtons[8]) { _state.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB; }
                if (raw.rgbButtons[9]) { _state.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB; }
                
                if (raw.lZ > 32767)
                {
                    _state.bLeftTrigger = (BYTE)clamp(255.0f * ((float)(raw.lZ - 32767) / 32768.0f), 0.0f, 255.0f);
                    _state.bRightTrigger = 0;
                }
                else if (raw.lZ < 32767)
                {
                    _state.bLeftTrigger = 0;
                    _state.bRightTrigger = (BYTE)clamp(255.0f * (1.0f - ((float)raw.lZ / 32767.0f)), 0.0f, 255.0f);
                }
                else
                {
                    _state.bLeftTrigger = 0;
                    _state.bRightTrigger = 0;
                }
            }
            
            const DWORD pov0 = raw.rgdwPOV[0];
            if ( ((pov0 >= 0) && (pov0 < 4500)) || ((pov0 > 31500) && (pov0 < 36000)) ) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_UP; }
            else if ((pov0 >  4500) && (pov0 < 13500)) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; }
            else if ((pov0 > 13500) && (pov0 < 22500)) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN; }
            else if ((pov0 > 22500) && (pov0 < 31500)) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT; }
            else if (pov0 ==  4500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_UP    | XINPUT_GAMEPAD_DPAD_RIGHT); }
            else if (pov0 == 13500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN); }
            else if (pov0 == 22500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_DOWN  | XINPUT_GAMEPAD_DPAD_LEFT); }
            else if (pov0 == 31500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_LEFT  | XINPUT_GAMEPAD_DPAD_UP); }
            
            {
                const float cent = (float)(range.XMin + range.XMax) * 0.5f;
                const float half = (float)(range.XMax - range.XMin) * 0.5f;
                const float numv = ((float)raw.lX - cent) / half;
                _state.sThumbLX = (SHORT)clamp(numv * 32768.0f, -32768.0f, 32767.0f);
            }
            {
                const float cent = (float)(range.YMin + range.YMax) * 0.5f;
                const float half = (float)(range.YMax - range.YMin) * 0.5f;
                const float numv = ((float)raw.lY - cent) / half;
                _state.sThumbLY = (SHORT)clamp(numv * -32768.0f, -32768.0f, 32767.0f);
            }
            
            if (device.dwSize == 0)
            {
                {
                    const float cent = (float)(range.ZMin + range.ZMax) * 0.5f;
                    const float half = (float)(range.ZMax - range.ZMin) * 0.5f;
                    const float numv = ((float)raw.lZ - cent) / half;
                    _state.sThumbRX = (SHORT)clamp(numv * 32768.0f, -32768.0f, 32767.0f);
                }
                {
                    const float cent = (float)(range.RzMin + range.RzMax) * 0.5f;
                    const float half = (float)(range.RzMax - range.RzMin) * 0.5f;
                    const float numv = ((float)raw.lRz - cent) / half;
                    _state.sThumbRY = (SHORT)clamp(numv * -32768.0f, -32768.0f, 32767.0f);
                }
            }
            else
            {
                {
                    const float cent = (float)(range.RxMin + range.RxMax) * 0.5f;
                    const float half = (float)(range.RxMax - range.RxMin) * 0.5f;
                    const float numv = ((float)raw.lRx - cent) / half;
                    _state.sThumbRX = (SHORT)clamp(numv * 32768.0f, -32768.0f, 32767.0f);
                }
                {
                    const float cent = (float)(range.RyMin + range.RyMax) * 0.5f;
                    const float half = (float)(range.RyMax - range.RyMin) * 0.5f;
                    const float numv = ((float)raw.lRy - cent) / half;
                    _state.sThumbRY = (SHORT)clamp(numv * -32768.0f, -32768.0f, 32767.0f);
                }
            }
            
            return true;
        }
        return false;
    }
    const wchar_t* DirectInput::getDeviceName(uint32_t index)
    {
        getself();
        if (index < self.gamepad_device.size())
        {
            return self.gamepad_device[index].tszInstanceName;
        }
        return nullptr;
    }
    const wchar_t* DirectInput::getProductName(uint32_t index)
    {
        getself();
        if (index < self.gamepad_device.size())
        {
            return self.gamepad_device[index].tszProductName;
        }
        return nullptr;
    }
    bool DirectInput::isXInputDevice(uint32_t index)
    {
        getself();
        if (index < self.gamepad_device.size())
        {
            return self.gamepad_device[index].dwSize != 0;
        }
        return false;
    }
    
    bool DirectInput::updateTargetWindow(ptrdiff_t window)
    {
        getself();
        self.window = (HWND)window;
        if (self.window == NULL)
        {
            _LOGDEBUG(L"NULL window exception\n");
            return false;
        }
        refresh();
        return true;
    }
    
    DirectInput::DirectInput(ptrdiff_t window)
    {
        _data = new _Data;
        if (_data == nullptr)
        {
            throw;
        }
        getself();
        
        self.window = (HWND)window;
        if (self.window == NULL)
        {
            _LOGDEBUG(L"window is NULL\n");
        }
        self.dll = LoadLibraryW(L"Dinput8.dll");
        if (self.dll == NULL)
        {
            _LOGDEBUG(L"load Dinput8.dll failed\n");
            return;
        }
        typedef HRESULT (CALLBACK *f_DirectInput8Create)(HINSTANCE, DWORD, const IID&, LPVOID*, LPUNKNOWN);
        f_DirectInput8Create f = (f_DirectInput8Create)GetProcAddress(self.dll, "DirectInput8Create");
        if (f == NULL)
        {
            _LOGDEBUG(L"GetProcAddress failed, can not find DirectInput8Create\n");
            return;
        }
        HRESULT hr = 0;
        hr = f(GetModuleHandleW(NULL), DIRECTINPUT_VERSION, g_IID_IDirectInput8W, (LPVOID*)(self.dinput.GetAddressOf()), NULL);
        if (hr != DI_OK)
        {
            _LOGDEBUG(L"DirectInput8Create failed\n");
            return;
        }
    }
    DirectInput::~DirectInput()
    {
        getself();
        
        clear();
        self.dinput.Reset();
        if (self.dll != NULL)
        {
            FreeLibrary(self.dll);
            self.dll = NULL;
        }
        if (self.window != NULL)
        {
            self.window = NULL;
        }
        
        delete _data;
        _data = nullptr;
    }
};

//********** MAGIC CODE **********//

#include <wbemidl.h>
#include <oleauto.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x) { (x)->Release(); (x) = NULL; }
#endif

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains 
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput 
//-----------------------------------------------------------------------------
BOOL IsXInputDevice( const GUID* pGuidProductFromDirectInput )
{
    IWbemLocator*           pIWbemLocator  = NULL;
    IEnumWbemClassObject*   pEnumDevices   = NULL;
    IWbemClassObject*       pDevices[20]   = {0};
    IWbemServices*          pIWbemServices = NULL;
    BSTR                    bstrNamespace  = NULL;
    BSTR                    bstrDeviceID   = NULL;
    BSTR                    bstrClassName  = NULL;
    DWORD                   uReturned      = 0;
    bool                    bIsXinputDevice= false;
    UINT                    iDevice        = 0;
    VARIANT                 var;
    HRESULT                 hr;

    // CoInit if needed
    hr = CoInitialize(NULL);
    bool bCleanupCOM = SUCCEEDED(hr);

    // So we can call VariantClear() later, even if we never had a successful IWbemClassObject::Get().
    VariantInit(&var);

    // Create WMI
    hr = CoCreateInstance( __uuidof(WbemLocator),
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IWbemLocator),
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;

    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;        
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;        
    bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &pIWbemServices );
    if( FAILED(hr) || pIWbemServices == NULL )
        goto LCleanup;

    // Switch security level to IMPERSONATE. 
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );                    

    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

    // Loop over all devices
    for( ;; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED(hr) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice=0; iDevice<uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it's an XInput device
                    // This information can not be found from DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                    if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;

                    // Compare the VID/PID to the DInput device
                    DWORD dwVidPid = MAKELONG( dwVid, dwPid );
                    if( dwVidPid == pGuidProductFromDirectInput->Data1 )
                    {
                        bIsXinputDevice = true;
                        goto LCleanup;
                    }
                }
            }
            VariantClear(&var);
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }

LCleanup:
    VariantClear(&var);
    if(bstrNamespace)
        SysFreeString(bstrNamespace);
    if(bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if(bstrClassName)
        SysFreeString(bstrClassName);
    for( iDevice=0; iDevice<20; iDevice++ )
        SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    if( bCleanupCOM )
        CoUninitialize();

    return bIsXinputDevice;
}
