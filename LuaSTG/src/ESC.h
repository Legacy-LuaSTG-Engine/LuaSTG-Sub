#pragma once
//======================================
//ex+网络传输

#define MAX_INPUT_CLIENTS 32 
#define MAX_INPUT 16
typedef unsigned long EX_KEY;

#define EX_MASK_SYSTEM 1
#define EX_MASK_PLAYER1 2
#define EX_MASK_PLAYER2 4
#define EX_MASK_PLAYER3 8
#define EX_MASK_PLAYER4 16

#define EX_SLOT_SYSTEM 0
#define EX_SLOT_PLAYER1 1
#define EX_SLOT_PLAYER2 2
#define EX_SLOT_PLAYER3 3
#define EX_SLOT_PLAYER4 4

struct EX_REMOTE_DEST{
	unsigned long addr;
	unsigned long port;
};

class IExReplayInput;

class IExLocalInput;

class IExCommonInput;

class EXTCPIPSERVERCLIENTINFO;

class EXSTRINGBUFFER;

class IExInputControl{
public:
	virtual bool GetVKeyState(int slot, int vkcode) = 0;
	virtual IExLocalInput *CreateLocalInput() = 0;
	virtual IExReplayInput *CreateReplayInput() = 0;
	virtual bool AssignInput(int deviceslot, IExCommonInput *input) = 0;
	virtual void SetIgnoreInput(int frames) = 0;
	virtual int GetIgnoreInput() = 0;
	virtual bool ProceedInput(bool *localInput) = 0;

	virtual bool SPGetKeyState(int keycode) = 0;
	virtual int SPAddKeyAlias(int vkey, int key, int slot) = 0;
	virtual int SPRemoveKeyAlias(int vkey, int key, int slot) = 0;
	virtual int SPResetKeyAlias(int key) = 0;
	virtual int SPResetVKeyAlias(int vkey) = 0;
	virtual int SPResetAllAlias() = 0;
	virtual EXTCPIPSERVERCLIENTINFO *GetNetwork() = 0;
	virtual bool ConnectTo(const char *dest,int port) = 0;
	virtual EXSTRINGBUFFER *GetNetworkBuffer() = 0;
	virtual void Reset() = 0;
	
};

class ExInputInterface;

class IExCommonInput{
public:
	virtual void SetMask(unsigned long mask) = 0;
	virtual void AddMask(unsigned long mask)=0;
	virtual void RemoveMask(unsigned long mask)=0;
	virtual unsigned long GetMask() = 0;
	virtual void SetRead(bool isread)=0;
};

class IExLocalInput{
public:
	virtual IExCommonInput *ToCommonInput()=0;
	virtual int GetDelay()=0;
	virtual void SetDelay(int delay)=0;
	virtual int AddKeyAlias(int vkey,int key) = 0;
	virtual int RemoveKeyAlias(int vkey, int key) = 0;
	virtual int ResetKeyAlias(int key) = 0;
	virtual int ResetVKeyAlias(int vkey) = 0;
	virtual int ResetAllAlias() = 0;
	virtual EXTCPIPSERVERCLIENTINFO *SetRemote(bool remote) = 0;
	virtual void Release() =0;
	virtual void SetNetwork(EXTCPIPSERVERCLIENTINFO *p) = 0;
};

class IExRemoteInput{
public:
	virtual IExCommonInput *ToCommonInput() = 0;
	virtual int GetBufferLength() = 0;
	virtual bool ConnectTo(EX_REMOTE_DEST *dest, int deviceslot);
};

class IExReplayInput{
public:
	virtual IExCommonInput *ToCommonInput() = 0;
	virtual int GetBufferLength() = 0;
	virtual int GetCurrentPos() = 0;
	virtual int GetBuffer(EX_KEY *keys,int maxlength) = 0;
	virtual int SetBuffer(EX_KEY *keys,int length) = 0;
	virtual bool ConnectTo(EX_REMOTE_DEST *dest, int deviceslot);
};

IExInputControl *CreateInputEx();

IExLocalInput *CreateLocalInput();

//======================================
//ex+模型读取

#include <stdio.h>
#include <string>
#include <list>
#include <vector>
//#include <atldbcli.h>
#include <io.h>

#define CP_UNICODE -1
#define CP_SHIFT_JS 932
#define CP_GBK 936

typedef std::vector<char> sbuffer;
/*

struct CODEPAGECOUNTER{
	unsigned int ansi;
	unsigned int unicode;
	unsigned int utf8;
	unsigned int gbk;
	unsigned int shiftjs;
	unsigned int total;
	CODEPAGECOUNTER();
	int CheckAll(const char *buffer);
	int getLikelyCode(int def);
};

struct CODEDSTR
{
	int codepage;
	sbuffer buffer;
	CODEDSTR();
	CODEDSTR(int page);
	CODEDSTR &operator=(const CODEDSTR&t);
	CODEDSTR &operator=(int page);
	char *getstring();
	int getstring(char *str, int maxcnt);
	wchar_t *getwstring();
	void getwstring(std::wstring &ws);
	bool LoadBuffer(const void *tbuffer, int tcodepage);
};
//*/
//======================================
//ex+math

#include "lua.hpp"

int ExSampleBezierA1(lua_State* L, int count, int sampleBy, float length, float rate);
