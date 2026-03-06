--------------------------------------------------------------------------------
--- LuaSTG Sub 键盘输入
--- LuaSTG Sub keyboard input
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

--- [LuaSTG Sub v0.15.6 新增]  
---@class lstg.Input.Keyboard
local M = {}

--------------------------------------------------------------------------------
--- 按键码常量
--- 注意：这些常量的值和内部实现有关，请勿直接使用数字值，应该通过常量访问
--- Key code constants
--- Note: The values of these constants are related to the internal implementation,
--- DO NOT use numeric values directly, they should be accessed through constants

M.None = 0

M.Back = 8
M.Tab = 9
M.Clear = 12
M.Enter = 13
M.Shift = 16
M.Control = 17
M.Alt = 18
M.Pause = 19
M.CapsLock = 20

--M.ImeHangul = 21
--M.ImeKana = 21
--M.ImeOn = 22
--M.ImeJunja = 23
--M.ImeFinal = 24
--M.ImeKanji = 25
--M.ImeHanja = 25
--M.ImeOff = 26

M.Escape = 27

--M.ImeConvert = 28
--M.ImeNoConvert = 29
--M.ImeAccept = 30
--M.ImeModeChangeRequest = 31

M.Space = 32
M.PageUp = 33
M.PageDown = 34
M.End = 35
M.Home = 36
M.Left = 37
M.Up = 38
M.Right = 39
M.Down = 40
M.Select = 41
M.Print = 42
M.Execute = 43
M.PrintScreen = 44
M.Insert = 45
M.Delete = 46
M.Help = 47

M.D0 = 48
M.D1 = 49
M.D2 = 50
M.D3 = 51
M.D4 = 52
M.D5 = 53
M.D6 = 54
M.D7 = 55
M.D8 = 56
M.D9 = 57

M.A = 65
M.B = 66
M.C = 67
M.D = 68
M.E = 69
M.F = 70
M.G = 71
M.H = 72
M.I = 73
M.J = 74
M.K = 75
M.L = 76
M.M = 77
M.N = 78
M.O = 79
M.P = 80
M.Q = 81
M.R = 82
M.S = 83
M.T = 84
M.U = 85
M.V = 86
M.W = 87
M.X = 88
M.Y = 89
M.Z = 90

M.LeftWindows = 91
M.RightWindows = 92
M.Apps = 93

--M.Sleep = 95

M.NumPad0 = 96
M.NumPad1 = 97
M.NumPad2 = 98
M.NumPad3 = 99
M.NumPad4 = 100
M.NumPad5 = 101
M.NumPad6 = 102
M.NumPad7 = 103
M.NumPad8 = 104
M.NumPad9 = 105

M.Multiply = 106
M.Add = 107
M.Separator = 108
M.Subtract = 109
M.Decimal = 110
M.Divide = 111

M.F1 = 112
M.F2 = 113
M.F3 = 114
M.F4 = 115
M.F5 = 116
M.F6 = 117
M.F7 = 118
M.F8 = 119
M.F9 = 120
M.F10 = 121
M.F11 = 122
M.F12 = 123
M.F13 = 124
M.F14 = 125
M.F15 = 126
M.F16 = 127
M.F17 = 128
M.F18 = 129
M.F19 = 130
M.F20 = 131
M.F21 = 132
M.F22 = 133
M.F23 = 134
M.F24 = 135

M.NumLock = 144
M.Scroll = 145
M.LeftShift = 160
M.RightShift = 161
M.LeftControl = 162
M.RightControl = 163
M.LeftAlt = 164
M.RightAlt = 165

--M.BrowserBack = 166
--M.BrowserForward = 167
--M.BrowserRefresh = 168
--M.BrowserStop = 169
--M.BrowserSearch = 170
--M.BrowserFavorites = 171
--M.BrowserHome = 172
--M.VolumeMute = 173
--M.VolumeDown = 174
--M.VolumeUp = 175
--M.MediaNextTrack = 176
--M.MediaPreviousTrack = 177
--M.MediaStop = 178
--M.MediaPlayPause = 179
--M.LaunchMail = 180
--M.SelectMedia = 181
--M.LaunchApplication1 = 182
--M.LaunchApplication2 = 183

--- ;:
M.Semicolon = 186
--- =+
M.Plus = 187
--- ,<
M.Comma = 188
--- -_
M.Minus = 189
--- .>
M.Period = 190
--- /?
M.Question = 191
--- `~
M.Tilde = 192
--- [{
M.OpenBrackets = 219
--- \|
M.Pipe = 220
--- ]}
M.CloseBrackets = 221
--- '"
M.Quotes = 222

--M.Oem8 = 223
--M.Oem102 = 226
--M.ProcessKey = 229

--- [LuaSTG Sub v0.17.0 新增]  
M.NumPadEnter = 232

--M.OemCopy = 242
--M.OemAuto = 243
--M.OemEnlW = 244
--M.Attn = 246
--M.Crsel = 247
--M.Exsel = 248
--M.EraseEof = 249
--M.Play = 250
--M.Zoom = 251
--M.Pa1 = 253
--M.OemClear = 254

--------------------------------------------------------------------------------
--- 方法
--- Methods

--- [LuaSTG Sub v0.15.6 新增]  
--- 获取键盘按键状态  
--- [LuaSTG Sub v0.15.6 Add]  
--- Get keyboard key state  
---@param code number
---@return boolean
function M.GetKeyState(code)
end

return M
