#pragma once

// ---------- ---------- framework ---------- ---------- //

#define LUASTG_CONFIGURATION_FILE "config.json"

#define LUASTG_LOG_FILE "engine.log"

#define LUASTG_LAUNCH_SCRIPT "launch"

#define USING_LAUNCH_FILE

//#define USING_ENCRYPTION

#define USING_DEAR_IMGUI

#define USING_CONSOLE_OUTPUT

// ---------- ---------- game play ---------- ---------- //

// Sakuya: THE WORLD!
#define LUASTG_ENABLE_THE_WORLD

// !!!deprecated
// lstg.GameObject.pause
//#define LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE

// add gravity (self.ag), speed limit (self.maxv, self.maxvx, self.maxvy)
#define USER_SYSTEM_OPERATION

// !!!deprecated
// BAKACHU
#define GLOBAL_SCALE_COLLI_SHAPE

// BAKAESC
#define USING_MULTI_GAME_WORLD

// allow render a object directly (no render callback)
#define USING_ADVANCE_GAMEOBJECT_CLASS

// ---------- ---------- steam api ---------- ---------- //

#define STEAM_APP_ID 0

//#define USING_STEAM_API

//#define KEEP_LAUNCH_BY_STEAM
