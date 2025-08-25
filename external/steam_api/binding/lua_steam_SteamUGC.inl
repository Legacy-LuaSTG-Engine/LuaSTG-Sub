#pragma once


class DownloadItemCallback {
public:

	int m_luaCallbackRef;
	lua_State* m_L;
	DownloadItemCallback(lua_State* L, int luaCallbackRef) : m_luaCallbackRef(luaCallbackRef), m_L(L) {}
	STEAM_CALLBACK(DownloadItemCallback, OnDownloadItemResult, DownloadItemResult_t);


};
void DownloadItemCallback::OnDownloadItemResult(DownloadItemResult_t* pCallback) {
	lua_pushstring(m_L, "SteamUGC_callback_registry");
	lua_gettable(m_L, LUA_REGISTRYINDEX);
	lua_rawgeti(m_L, -1, m_luaCallbackRef);
	lua_remove(m_L, -2);
	if (lua_isfunction(m_L, -1)) {

		lua_createtable(m_L, 0, 3);
		lua_pushinteger(m_L, pCallback->m_eResult);
		lua_setfield(m_L, -2, "m_eResult");
		lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
		lua_setfield(m_L, -2, "m_nPublishedFileId");
		lua_push_uint32(m_L, pCallback->m_unAppID);
		lua_setfield(m_L, -2, "m_unAppID");
		if (lua_pcall(m_L, 1, 0, 0) != LUA_OK) {

			lua_error(m_L);
			lua_pop(m_L, 1);
		}
	}
	else
	{
		luaL_error(m_L, "Callback function not found");
	}
	lua_pushstring(m_L, "SteamUGC_callback_registry");
	lua_gettable(m_L, LUA_REGISTRYINDEX);
	luaL_unref(m_L, -1, m_luaCallbackRef);
	lua_pop(m_L, 1);
	delete this;
}

struct xSteamUGC {
	template<class T>
	class LuaCallResult {
	private:
		lua_State* m_L;
		SteamAPICall_t m_Callback;
		CCallResult<LuaCallResult<T>, T> m_CallResult;
		int m_luaCallbackRef;
	public:
		LuaCallResult(lua_State* L, SteamAPICall_t call, int luaCallbackRef, void (LuaCallResult<T>::* func)(T*, bool))
			: m_L(L), m_Callback(call), m_luaCallbackRef(luaCallbackRef)
		{
			m_CallResult.Set(m_Callback, this, func);
		}
		LuaCallResult() {
			m_CallResult.Set(k_uAPICallInvalid, this, &LuaCallResult<T>::NullCallBack);
		}
		~LuaCallResult() {
			if (m_luaCallbackRef != LUA_NOREF && m_L) {
				luaL_unref(m_L, LUA_REGISTRYINDEX, m_luaCallbackRef);
				m_luaCallbackRef = LUA_NOREF;
			}
			m_L = nullptr;
			m_Callback = k_uAPICallInvalid;
		}
		void RegisterCallback(lua_State* L, SteamAPICall_t call, int luaCallbackRef, void (LuaCallResult<T>::* func)(T*, bool)) {
			m_L = L;
			m_Callback = call;
			m_luaCallbackRef = luaCallbackRef;
			m_CallResult.Set(m_Callback, this, func);
		}
		void NullCallBack(T* pCallback, bool bIOFailure) {
			//delete this;
		}


#define CALLBACKFUNC(NAME, TOLUA) \
    void On##NAME(T* pCallback, bool bIOFailure) { \
        if (bIOFailure) { \
            luaL_error(m_L, "IO failure"); \
            return; \
        } \
        lua_pushstring(m_L, "SteamUGC_callback_registry"); \
        lua_gettable(m_L, LUA_REGISTRYINDEX); \
        lua_rawgeti(m_L, -1, m_luaCallbackRef); \
        lua_remove(m_L, -2); \
        if (lua_isfunction(m_L, -1)) { \
            do { TOLUA; } while (0); \
            if (lua_pcall(m_L, 1, 0, 0) != LUA_OK) { \
                const char* errmsg = lua_tostring(m_L, -1); \
                luaL_error(m_L, "Lua callback error: %s", errmsg); \
				return; \
            } \
        } \
        else { \
            luaL_error(m_L, "Callback function not found"); \
        } \
        delete this; \
    }
		CALLBACKFUNC(AddAppDependencyResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");

			lua_push_uint32(m_L, pCallback->m_nAppID);
			lua_setfield(m_L, -2, "m_nAppID");
			});
		CALLBACKFUNC(AddUGCDependencyResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nChildPublishedFileId);
			lua_setfield(m_L, -2, "m_nChildPublishedFileId");
			});
		CALLBACKFUNC(CreateItemResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");

			lua_pushboolean(m_L, pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
			lua_setfield(m_L, -2, "m_bUserNeedsToAcceptWorkshopLegalAgreement");
			});
		CALLBACKFUNC(DeleteItemResult, {
			lua_createtable(m_L, 0, 2);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			});
		CALLBACKFUNC(DownloadItemResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			lua_push_uint32(m_L, pCallback->m_unAppID);
			lua_setfield(m_L, -2, "m_unAppID");
			});
		CALLBACKFUNC(GetAppDependenciesResult, {
			lua_createtable(m_L, 0, 5);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			lua_pushinteger(m_L, pCallback->m_nNumAppDependencies);
			lua_setfield(m_L, -2, "m_nNumAppDependencies");
			lua_pushinteger(m_L, pCallback->m_nTotalNumAppDependencies);
			lua_setfield(m_L, -2, "m_nTotalNumAppDependencies");
			lua_createtable(m_L, pCallback->m_nNumAppDependencies, 0);
			for (uint32 i = 0; i < pCallback->m_nNumAppDependencies; i++) {
				lua_pushinteger(m_L, pCallback->m_rgAppIDs[i]);
				lua_rawseti(m_L, -2, i + 1);
			}
			lua_setfield(m_L, -2, "m_rgAppIDs");
			});
		CALLBACKFUNC(GetUserItemVoteResult, {
			lua_createtable(m_L, 0, 5);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			lua_pushboolean(m_L, pCallback->m_bVotedUp);
			lua_setfield(m_L, -2, "m_bVotedUp");
			lua_pushboolean(m_L, pCallback->m_bVotedDown);
			lua_setfield(m_L, -2, "m_bVotedDown");
			lua_pushboolean(m_L, pCallback->m_bVoteSkipped);
			lua_setfield(m_L, -2, "m_bVoteSkipped");
			});
		CALLBACKFUNC(ItemInstalled, {
			lua_createtable(m_L, 0, 2);
			lua_push_uint32(m_L, pCallback->m_unAppID);
			lua_setfield(m_L, -2, "m_unAppID");
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			});
		CALLBACKFUNC(RemoveAppDependencyResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");

			lua_push_uint32(m_L, pCallback->m_nAppID);
			lua_setfield(m_L, -2, "m_nAppID");
			});
		CALLBACKFUNC(RemoveUGCDependencyResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nChildPublishedFileId);
			lua_setfield(m_L, -2, "m_nChildPublishedFileId");
			});
		CALLBACKFUNC(SetUserItemVoteResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");

			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");

			lua_pushboolean(m_L, pCallback->m_bVoteUp);
			lua_setfield(m_L, -2, "m_bVoteUp");
			});
		CALLBACKFUNC(StartPlaytimeTrackingResult, {
			lua_createtable(m_L, 0, 1);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			});
		CALLBACKFUNC(StopPlaytimeTrackingResult, {
			lua_createtable(m_L, 0, 1);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			});
		CALLBACKFUNC(SteamUGCQueryCompleted, {
			lua_createtable(m_L, 0, 5);
			lua_push_UGCQueryHandle_t(m_L, pCallback->m_handle);
			lua_setfield(m_L, -2, "m_handle");
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_pushinteger(m_L, pCallback->m_unNumResultsReturned);
			lua_setfield(m_L, -2, "m_unNumResultsReturned");
			lua_pushinteger(m_L, pCallback->m_unTotalMatchingResults);
			lua_setfield(m_L, -2, "m_unTotalMatchingResults");
			lua_pushboolean(m_L, pCallback->m_bCachedData);
			lua_setfield(m_L, -2, "m_bCachedData");
			});
		CALLBACKFUNC(SubmitItemUpdateResult, {
			lua_createtable(m_L, 0, 2);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_pushboolean(m_L, pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
			lua_setfield(m_L, -2, "m_bUserNeedsToAcceptWorkshopLegalAgreement");
			});
		CALLBACKFUNC(UserFavoriteItemsListChanged, {
			lua_createtable(m_L, 0, 3);
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_pushboolean(m_L, pCallback->m_bWasAddRequest);
			lua_setfield(m_L, -2, "m_bWasAddRequest");
			});
		CALLBACKFUNC(UserSubscribedItemsListChanged, {
			lua_createtable(m_L, 0, 1);
			lua_push_uint32(m_L, pCallback->m_nAppID);
			lua_setfield(m_L, -2, "m_nAppID");
			});
		CALLBACKFUNC(RemoteStorageUnsubscribePublishedFileResult, {
			lua_createtable(m_L, 0, 2);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			});
		CALLBACKFUNC(RemoteStorageUpdatePublishedFileResult, {
			lua_createtable(m_L, 0, 3);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_push_PublishedFileId_t(m_L, pCallback->m_nPublishedFileId);
			lua_setfield(m_L, -2, "m_nPublishedFileId");
			lua_pushboolean(m_L, pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
			lua_setfield(m_L, -2, "m_bUserNeedsToAcceptWorkshopLegalAgreement");
			});
		CALLBACKFUNC(WorkshopEULAStatus, {
			lua_createtable(m_L, 0, 6);
			lua_pushinteger(m_L, pCallback->m_eResult);
			lua_setfield(m_L, -2, "m_eResult");
			lua_push_uint32(m_L, pCallback->m_nAppID);
			lua_setfield(m_L, -2, "m_nAppID");
			lua_push_uint32(m_L, pCallback->m_unVersion);
			lua_setfield(m_L, -2, "m_unVersion");
			lua_push_uint32(m_L, pCallback->m_rtAction);
			lua_setfield(m_L, -2, "m_rtAction");
			lua_pushboolean(m_L, pCallback->m_bAccepted);
			lua_setfield(m_L, -2, "m_bAccepted");
			lua_pushboolean(m_L, pCallback->m_bNeedsAction);
			lua_setfield(m_L, -2, "m_bNeedsAction");

			});

	};

	static int getLuaFuncRef(lua_State* L, int idx) {
		if (idx == -1) {
			idx = lua_gettop(L);
		}
		luaL_checktype(L, idx, LUA_TFUNCTION);

		lua_pushstring(L, "SteamUGC_callback_registry");
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushvalue(L, idx);
		int callbackRef = luaL_ref(L, -2);
		lua_pop(L, 1);
		return callbackRef;
	}
#define REGISTERCALLBACK(TYPE) \
    do { \
        if (result != k_uAPICallInvalid) { \
            new LuaCallResult<TYPE##_t>(L, result, callbackRef, &LuaCallResult<TYPE##_t>::On##TYPE); \
        } \
    } while(0)
	static int AddAppDependency(lua_State* L) {
		PublishedFileId_t const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		AppId_t const nAppID = lua_to_uint32(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->AddAppDependency(nPublishedFileID, nAppID);
		REGISTERCALLBACK(AddAppDependencyResult);
		return 1;
	}
	static int AddDependency(lua_State* L) {
		auto const nParentPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		auto const nChildPublishedFileID = lua_to_PublishedFileId_t(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->AddDependency(nParentPublishedFileID, nChildPublishedFileID);
		REGISTERCALLBACK(AddUGCDependencyResult);

		return 1;
	}
	static int AddExcludedTag(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pTagName = luaL_checkstring(L, 2);

		const bool bSuccess = SteamUGC()->AddExcludedTag(handle, pTagName);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemKeyValueTag(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const char* pchValue = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->AddItemKeyValueTag(handle, pchKey, pchValue);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemPreviewFile(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pszPreviewFile = luaL_checkstring(L, 2);
		const int pPreviewType = luaL_checkinteger(L, 3);
		const bool bSuccess = SteamUGC()->AddItemPreviewFile(handle, pszPreviewFile, EItemPreviewType(pPreviewType));

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemPreviewVideo(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pszVideoID = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->AddItemPreviewVideo(handle, pszVideoID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemToFavorites(lua_State* L) {
		auto const nAppID = lua_to_uint32(L, 1);
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->AddItemToFavorites(nAppID, nPublishedFileID);
		REGISTERCALLBACK(UserFavoriteItemsListChanged);
		return 1;
	}
	static int AddRequiredKeyValueTag(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const char* pchValue = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->AddRequiredKeyValueTag(handle, pchKey, pchValue);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddRequiredTag(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pTagName = luaL_checkstring(L, 2);

		const bool bSuccess = SteamUGC()->AddRequiredTag(handle, pTagName);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int BInitWorkshopForGameServer(lua_State* L) {
		auto const unWorkshopDepotID = lua_to_uint32(L, 1);
		const char* pszFolder = luaL_checkstring(L, 2);
		const int b = SteamUGC()->BInitWorkshopForGameServer(unWorkshopDepotID, pszFolder);

		lua_pushboolean(L, b);
		return 1;
	}
	static int CreateItem(lua_State* L) {
		auto const nConsumerAppID = lua_to_uint32(L, 1);
		const int eFileType = luaL_checkinteger(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->CreateItem(nConsumerAppID, EWorkshopFileType(eFileType));
		REGISTERCALLBACK(CreateItemResult);
		return 1;
	}
	static int DeleteItem(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->DeleteItem(nPublishedFileID);
		REGISTERCALLBACK(DeleteItemResult);
		return 1;
	}
	static int DownloadItem(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		const bool bHighPriority = lua_toboolean(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		const bool bSuccess = SteamUGC()->DownloadItem(nPublishedFileID, bHighPriority);
		if (bSuccess) {
			new DownloadItemCallback(L, callbackRef);
		}
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int GetAppDependencies(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->GetAppDependencies(nPublishedFileID);
		REGISTERCALLBACK(GetAppDependenciesResult);
		return 1;
	}
	static int GetItemDownloadInfo(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		uint64 BytesDownloaded = 0;
		uint64 BytesTotal = 0;
		if (SteamUGC()->GetItemDownloadInfo(nPublishedFileID, &BytesDownloaded, &BytesTotal)) {
			lua_push_uint64(L, BytesDownloaded);
			lua_push_uint64(L, BytesTotal);
			return 2;
		}
		return 0;
	}
	static int GetItemInstallInfo(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		uint64 SizeOnDisk = 0;
		int cchFolderSize = 1024;
		char* pchFolder = new char[cchFolderSize];
		uint32 nTimeStamp = 0;
		if (SteamUGC()->GetItemInstallInfo(nPublishedFileID, &SizeOnDisk, pchFolder, cchFolderSize, &nTimeStamp)) {
			lua_pushboolean(L, true);
			lua_push_uint64(L, SizeOnDisk);
			lua_pushstring(L, pchFolder);
			lua_pushinteger(L, nTimeStamp);
			delete[] pchFolder;
			return 4;
		}
		else {
			lua_pushboolean(L, false);
			delete[] pchFolder;
			return 1;
		}
	}
	static int GetItemState(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		int eState = SteamUGC()->GetItemState(nPublishedFileID);
		lua_pushinteger(L, eState);
		return 1;
	}
	static int GetItemUpdateProgress(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		uint64 BytesDownloaded = 0;
		uint64 BytesTotal = 0;
		if (SteamUGC()->GetItemUpdateProgress(nPublishedFileID, &BytesDownloaded, &BytesTotal)) {
			lua_push_uint64(L, BytesDownloaded);
			lua_push_uint64(L, BytesTotal);
			return 2;
		}
		return 0;
	}
	static int GetNumSubscribedItems(lua_State* L) {
		const int nNumItems = SteamUGC()->GetNumSubscribedItems();
		lua_pushinteger(L, nNumItems);
		return 1;
	}
	static int GetSubscribedItems(lua_State* L) {
		const int cMaxEntries = SteamUGC()->GetNumSubscribedItems();
		PublishedFileId_t* pvecPublishedFileID = new PublishedFileId_t[cMaxEntries];
		uint32 unNumItems = SteamUGC()->GetSubscribedItems(pvecPublishedFileID, cMaxEntries);
		lua_createtable(L, unNumItems, 0);
		for (uint32 i = 0; i < unNumItems; i++) {
			lua_push_PublishedFileId_t(L, pvecPublishedFileID[i]);
			lua_rawseti(L, 1, i + 1);
		}
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int GetUserItemVote(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->GetUserItemVote(nPublishedFileID);
		REGISTERCALLBACK(GetUserItemVoteResult);
		return 1;
	}
	static int ReleaseQueryUGCRequest(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const bool bResult = SteamUGC()->ReleaseQueryUGCRequest(handle);
		lua_pushboolean(L, bResult);
		return 1;
	}
	static int RemoveAppDependency(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		auto const nAppID = lua_to_uint32(L, 2);
		const bool bSuccess = SteamUGC()->RemoveAppDependency(nPublishedFileID, nAppID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveDependency(lua_State* L) {
		auto const nParentPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		auto const nChildPublishedFileID = lua_to_PublishedFileId_t(L, 2);
		const bool bSuccess = SteamUGC()->RemoveDependency(nParentPublishedFileID, nChildPublishedFileID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveItemFromFavorites(lua_State* L) {
		auto const nAppID = lua_to_uint32(L, 1);
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 2);
		const bool bSuccess = SteamUGC()->RemoveItemFromFavorites(nAppID, nPublishedFileID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveItemKeyValueTags(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->RemoveItemKeyValueTags(handle, pchKey);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveItemPreview(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const int iPreviewIndex = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveItemPreview(handle, iPreviewIndex);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddContentDescriptor(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const int descid = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->AddContentDescriptor(handle, EUGCContentDescriptorID(descid));
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveContentDescriptor(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const int descid = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveContentDescriptor(handle, EUGCContentDescriptorID(descid));
		lua_pushboolean(L, bSuccess);
		return 1;
	}

	static int SetItemContent(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pszContentFolder = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemContent(handle, pszContentFolder);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemDescription(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pchDescription = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemDescription(handle, pchDescription);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemMetadata(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pchMetadata = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemMetadata(handle, pchMetadata);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemPreview(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pszPreviewFile = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemPreview(handle, pszPreviewFile);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemTags(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		size_t numTags = lua_objlen(L, 2);
		const char** ppchTags = new const char* [numTags];
		for (size_t i = 0; i < numTags; i++) {
			lua_rawgeti(L, 2, i + 1);
			ppchTags[i] = luaL_checkstring(L, -1);
			lua_pop(L, 1);
		}
		SteamParamStringArray_t tags;
		tags.m_ppStrings = ppchTags;
		tags.m_nNumStrings = numTags;
		const bool bSuccess = SteamUGC()->SetItemTags(handle, &tags);
		lua_pushboolean(L, bSuccess);
		delete[] ppchTags;
		return 1;
	}
	static int SetItemTitle(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pchTitle = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemTitle(handle, pchTitle);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemUpdateLanguage(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pchLanguage = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemUpdateLanguage(handle, pchLanguage);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemVisibility(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const int eVisibility = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->SetItemVisibility(handle, ERemoteStoragePublishedFileVisibility(eVisibility));
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetLanguage(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const char* pchLanguage = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetLanguage(handle, pchLanguage);
		lua_pushboolean(L, bSuccess);
		return 1;
	}

	static int CreateQueryAllUGCRequest(lua_State* L) {
		const int eQueryType = luaL_checkinteger(L, 1);
		const int eFileType = luaL_checkinteger(L, 2);
		const int eCreatorAppID = lua_to_uint32(L, 3);
		const int eConsumerAppID = lua_to_uint32(L, 4);
		const int unPage = luaL_checkinteger(L, 5);
		auto const UGCQueryHandle = SteamUGC()->CreateQueryAllUGCRequest(EUGCQuery(eQueryType),
			EUGCMatchingUGCType(eFileType), eCreatorAppID, eConsumerAppID, unPage);
		lua_push_UGCQueryHandle_t(L, UGCQueryHandle);
		return 1;
	}
	static int CreateQueryUserUGCRequest(lua_State* L) {
		const int accountID = lua_to_uint32(L, 1);
		const int eListType = luaL_checkinteger(L, 2);
		const int eMatchingUGCType = luaL_checkinteger(L, 3);
		const int eSortOrder = luaL_checkinteger(L, 4);
		auto const nCreatorAppID = lua_to_uint32(L, 5);
		auto const nConsumerAppID = lua_to_uint32(L, 6);
		const int unPage = luaL_checkinteger(L, 7);
		auto const UGCQueryHandle = SteamUGC()->CreateQueryUserUGCRequest(accountID, EUserUGCList(eListType),
			EUGCMatchingUGCType(eMatchingUGCType), EUserUGCListSortOrder(eSortOrder), nCreatorAppID, nConsumerAppID, unPage);
		lua_push_UGCQueryHandle_t(L, UGCQueryHandle);
		return 1;
	}
	static int CreateQueryUGCDetailsRequest(lua_State* L) {
		const int unNumPublishedFileIDs = lua_objlen(L, 1);
		if (unNumPublishedFileIDs < 1) {
			lua_pushnil(L);
			return 0;  // ���� nil ��ʾʧ��
		}
		PublishedFileId_t* pvecPublishedFileID = new PublishedFileId_t[unNumPublishedFileIDs];
		for (int i = 0; i < unNumPublishedFileIDs; i++) {
			lua_rawgeti(L, 1, i + 1);
			pvecPublishedFileID[i] = lua_to_PublishedFileId_t(L, -1);
			lua_pop(L, 1);
		}
		auto const UGCQueryHandle = SteamUGC()->CreateQueryUGCDetailsRequest(pvecPublishedFileID, unNumPublishedFileIDs);
		lua_push_UGCQueryHandle_t(L, UGCQueryHandle);
		delete[] pvecPublishedFileID;
		return 1;
	}

	static int SendQueryUGCRequest(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->SendQueryUGCRequest(handle);
		if (result == k_uAPICallInvalid) {
			lua_pushboolean(L, false);
			return 0;
		}
		else
		{
			REGISTERCALLBACK(SteamUGCQueryCompleted);
			lua_pushboolean(L, true);
			return 1;

		}
	}
	static int SetUserItemVote(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		const bool bVoteUp = lua_toboolean(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->SetUserItemVote(nPublishedFileID, bVoteUp);
		REGISTERCALLBACK(SetUserItemVoteResult);
		return 1;
	}
	static int StartItemUpdate(lua_State* L) {
		auto const nConsumerAppID = lua_to_uint32(L, 1);
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 2);
		auto const handle = SteamUGC()->StartItemUpdate(nConsumerAppID, nPublishedFileID);
		lua_push_UGCUpdateHandle_t(L, handle);
		return 1;
	}
	static int StartPlaytimeTracking(lua_State* L) {
		size_t unNumPublishedFileIDs = lua_objlen(L, 1);
		if (unNumPublishedFileIDs == 0) {
			return 0;
		}
		PublishedFileId_t* pvecPublishedFileID = new PublishedFileId_t[unNumPublishedFileIDs];
		for (size_t i = 0; i < unNumPublishedFileIDs; i++) {
			lua_rawgeti(L, 1, i + 1);
			pvecPublishedFileID[i] = lua_to_PublishedFileId_t(L, -1);
			lua_pop(L, 1);
		}
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->StartPlaytimeTracking(pvecPublishedFileID, unNumPublishedFileIDs);
		REGISTERCALLBACK(StartPlaytimeTrackingResult);
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int StopPlaytimeTracking(lua_State* L) {
		size_t unNumPublishedFileIDs = lua_objlen(L, 1);
		if (unNumPublishedFileIDs == 0) {
			return 0;
		}
		PublishedFileId_t* pvecPublishedFileID = new PublishedFileId_t[unNumPublishedFileIDs];
		for (size_t i = 0; i < unNumPublishedFileIDs; i++) {
			lua_rawgeti(L, 1, i + 1);
			pvecPublishedFileID[i] = lua_to_PublishedFileId_t(L, -1);
			lua_pop(L, 1);
		}
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->StopPlaytimeTracking(pvecPublishedFileID, unNumPublishedFileIDs);
		REGISTERCALLBACK(StopPlaytimeTrackingResult);
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int StopPlaytimeTrackingForAllItems(lua_State* L) {
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->StopPlaytimeTrackingForAllItems();
		REGISTERCALLBACK(StopPlaytimeTrackingResult);
		return 1;
	}
	static int SubscribeItem(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->SubscribeItem(nPublishedFileID);
		REGISTERCALLBACK(RemoteStorageUpdatePublishedFileResult);
		return 1;
	}
	static int UnsubscribeItem(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->UnsubscribeItem(nPublishedFileID);
		REGISTERCALLBACK(RemoteStorageUnsubscribePublishedFileResult);
		return 1;
	}
	static int SubmitItemUpdate(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const char* pchChangeNote = luaL_checkstring(L, 2);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUGC()->SubmitItemUpdate(handle, pchChangeNote);
		REGISTERCALLBACK(SubmitItemUpdateResult);
		return 1;
	}
	static int SuspendDownloads(lua_State* L) {
		const bool bSuspend = lua_toboolean(L, 1);
		SteamUGC()->SuspendDownloads(bSuspend);
		return 1;
	}
	static int UpdateItemPreviewFile(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const int index = luaL_checkinteger(L, 2);
		const char* pszPreviewFile = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->UpdateItemPreviewFile(handle, index, pszPreviewFile);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int UpdateItemPreviewVideo(lua_State* L) {
		auto const handle = lua_to_UGCUpdateHandle_t(L, 1);
		const int index = luaL_checkinteger(L, 2);
		const char* pszVideoID = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->UpdateItemPreviewVideo(handle, index, pszVideoID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int ShowWorkshopEULA(lua_State* L) {
		const bool bSuccess = SteamUGC()->ShowWorkshopEULA();
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int GetQueryUGCResult(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		SteamUGCDetails_t* details = new SteamUGCDetails_t;
		if (SteamUGC()->GetQueryUGCResult(handle, index, details)) {
			lua_createtable(L, 0, 26);
			lua_push_PublishedFileId_t(L, details->m_nPublishedFileId);
			lua_setfield(L, -2, "m_nPublishedFileId");

			lua_pushinteger(L, details->m_eResult);
			lua_setfield(L, -2, "m_eResult");

			lua_pushinteger(L, details->m_eFileType);
			lua_setfield(L, -2, "m_eFileType");

			lua_push_uint32(L, details->m_nCreatorAppID);
			lua_setfield(L, -2, "m_nCreatorAppID");

			lua_push_uint32(L, details->m_nConsumerAppID);
			lua_setfield(L, -2, "m_nConsumerAppID");

			lua_pushstring(L, details->m_rgchTitle);
			lua_setfield(L, -2, "m_rgchTitle");

			lua_pushstring(L, details->m_rgchDescription);
			lua_setfield(L, -2, "m_rgchDescription");

			lua_push_uint64(L, details->m_ulSteamIDOwner);
			lua_setfield(L, -2, "m_ulSteamIDOwner");

			lua_pushinteger(L, details->m_rtimeCreated);
			lua_setfield(L, -2, "m_rtimeCreated");

			lua_pushinteger(L, details->m_rtimeUpdated);
			lua_setfield(L, -2, "m_rtimeUpdated");

			lua_pushinteger(L, details->m_rtimeAddedToUserList);
			lua_setfield(L, -2, "m_rtimeAddedToUserList");

			lua_pushinteger(L, details->m_eVisibility);
			lua_setfield(L, -2, "m_eVisibility");

			lua_pushboolean(L, details->m_bBanned);
			lua_setfield(L, -2, "m_bBanned");

			lua_pushboolean(L, details->m_bAcceptedForUse);
			lua_setfield(L, -2, "m_bAcceptedForUse");

			lua_pushboolean(L, details->m_bTagsTruncated);
			lua_setfield(L, -2, "m_bTagsTruncated");

			lua_pushstring(L, details->m_rgchTags);
			lua_setfield(L, -2, "m_rgchTags");

			lua_push_UGCHandle_t(L, details->m_hFile);
			lua_setfield(L, -2, "m_hFile");

			lua_push_UGCHandle_t(L, details->m_hPreviewFile);
			lua_setfield(L, -2, "m_hPreviewFile");

			lua_pushstring(L, details->m_pchFileName);
			lua_setfield(L, -2, "m_pchFileName");

			lua_pushinteger(L, details->m_nFileSize);
			lua_setfield(L, -2, "m_nFileSize");

			lua_pushinteger(L, details->m_nPreviewFileSize);
			lua_setfield(L, -2, "m_nPreviewFileSize");

			lua_pushstring(L, details->m_rgchURL);
			lua_setfield(L, -2, "m_rgchURL");

			lua_pushinteger(L, details->m_unVotesUp);
			lua_setfield(L, -2, "m_unVotesUp");

			lua_pushinteger(L, details->m_unVotesDown);
			lua_setfield(L, -2, "m_unVotesDown");

			lua_pushnumber(L, details->m_flScore);
			lua_setfield(L, -2, "m_flScore");

			lua_pushinteger(L, details->m_unNumChildren);
			lua_setfield(L, -2, "m_unNumChildren");
			delete details;
			return 1;

		}
		else {
			lua_pushnil(L);
			delete details;
			return 0;
		}

	}
	static int GetQueryUGCAdditionalPreview(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 previewIndex = luaL_checkinteger(L, 3);
		uint32 cchURLSize = k_cchPublishedFileURLMax;
		uint32 cchOriginalFileNameSize = k_cchFilenameMax;
		char* pchURLOrVideoID = new char[k_cchPublishedFileURLMax] { 0 };
		char* pchOriginalFileName = new char[k_cchFilenameMax] { 0 };
		EItemPreviewType ePreviewType;
		if (SteamUGC()->GetQueryUGCAdditionalPreview(handle, index, previewIndex, pchURLOrVideoID,
			cchURLSize, pchOriginalFileName, cchOriginalFileNameSize, &ePreviewType)) {
			lua_pushstring(L, pchURLOrVideoID);
			lua_pushstring(L, pchOriginalFileName);
			lua_pushinteger(L, ePreviewType);
			delete[] pchURLOrVideoID;
			delete[] pchOriginalFileName;
			return 3;
		}
		else {
			lua_pushnil(L);
			delete[] pchURLOrVideoID;
			delete[] pchOriginalFileName;
			return 0;
		}
	}
	static int GetQueryUGCChildren(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 cMaxEntries = luaL_checkinteger(L, 3);
		PublishedFileId_t* pvecPublishedFileID = new PublishedFileId_t[cMaxEntries];
		lua_createtable(L, cMaxEntries, 0);
		if (cMaxEntries == 0) {
			return 1;
		}
		if (SteamUGC()->GetQueryUGCChildren(handle, index, pvecPublishedFileID, cMaxEntries)) {
			for (uint32 i = 0; i < cMaxEntries; i++) {
				lua_push_PublishedFileId_t(L, pvecPublishedFileID[i]);
				lua_rawseti(L, -2, i + 1);
			}
		}
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int GetQueryUGCNumTags(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 count = SteamUGC()->GetQueryUGCNumTags(handle, index);
		lua_pushinteger(L, count);
		return 1;
	}
	static int GetQueryUGCTag(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 tagIndex = luaL_checkinteger(L, 3);
		char pchValue[128]= { 0 };
		if (SteamUGC()->GetQueryUGCTag(handle, index, tagIndex, pchValue, 128)) {
			lua_pushstring(L, pchValue);
			return 1;
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	}
	static int GetQueryUGCTagDisplayName(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 tagIndex = luaL_checkinteger(L, 3);
		char pchValue[512];
		if (SteamUGC()->GetQueryUGCTagDisplayName(handle, index, tagIndex, pchValue, 512)) {
			lua_pushstring(L, pchValue);
			return 1;
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	}
	static int GetQueryUGCKeyValueTag(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 keyValueTagIndex=luaL_checkinteger(L, 3);
		char pchKey[512] = { 0 };
		char pchValue[512] = { 0 };
		if (SteamUGC()->GetQueryUGCKeyValueTag(handle, index, keyValueTagIndex, pchKey, 512, pchValue, 512)) {
			lua_pushstring(L, pchKey);
			lua_pushstring(L, pchValue);
			return 2;
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	}
	static int GetQueryUGCMetadata(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		char pchMetadata[512] = { 0 };
		if (SteamUGC()->GetQueryUGCMetadata(handle, index, pchMetadata, 512)) {
			lua_pushstring(L, pchMetadata);
			return 1;
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	}
	static int GetQueryUGCNumAdditionalPreviews(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 count = SteamUGC()->GetQueryUGCNumAdditionalPreviews(handle, index);
		lua_pushinteger(L, count);
		return 1;
	}
	static int GetQueryUGCNumKeyValueTags(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		const uint32 count = SteamUGC()->GetQueryUGCNumKeyValueTags(handle, index);
		lua_pushinteger(L, count);
		return 1;
	}
	static int GetQueryUGCPreviewURL(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		char pchURL[512] = { 0 };
		if (SteamUGC()->GetQueryUGCMetadata(handle, index, pchURL, 512)) {
			lua_pushstring(L, pchURL);
			return 1;
		}
		else {
			lua_pushnil(L);
			return 0;
		}
	}
	static int GetQueryUGCStatistic(lua_State* L) {
		auto const handle = lua_to_UGCQueryHandle_t(L, 1);
		const uint32 index = luaL_checkinteger(L, 2);
		EItemStatistic eStatType = static_cast<EItemStatistic>(luaL_checkinteger(L, 3));
		uint64* pStatValue = new uint64;
		if (SteamUGC()->GetQueryUGCStatistic(handle, index, eStatType, pStatValue)) {
			lua_push_uint64(L, *pStatValue);
			delete pStatValue;
			return 1;
		}
		else {
			lua_pushnil(L);
			delete pStatValue;
			return 0;
		}
	}

	static int xRegister(lua_State* L)
	{
		static const luaL_Reg lib[] = {
			xfbinding(AddAppDependency),
			xfbinding(AddDependency),
			xfbinding(AddExcludedTag),
			xfbinding(AddItemKeyValueTag),
			xfbinding(AddItemPreviewFile),
			xfbinding(AddItemPreviewVideo),
			xfbinding(AddItemToFavorites),
			xfbinding(AddRequiredKeyValueTag),
			xfbinding(AddRequiredTag),
			xfbinding(BInitWorkshopForGameServer),
			xfbinding(CreateItem),
			xfbinding(DeleteItem),
			xfbinding(DownloadItem),
			xfbinding(GetAppDependencies),
			xfbinding(GetItemDownloadInfo),
			xfbinding(GetItemInstallInfo),
			xfbinding(GetItemState),
			xfbinding(GetItemUpdateProgress),
			xfbinding(GetNumSubscribedItems),
			xfbinding(GetSubscribedItems),
			xfbinding(GetUserItemVote),
			xfbinding(ReleaseQueryUGCRequest),
			xfbinding(RemoveAppDependency),
			xfbinding(RemoveDependency),
			xfbinding(RemoveItemFromFavorites),
			xfbinding(RemoveItemKeyValueTags),
			xfbinding(RemoveItemPreview),
			xfbinding(AddContentDescriptor),
			xfbinding(RemoveContentDescriptor),
			xfbinding(SetItemContent),
			xfbinding(SetItemDescription),
			xfbinding(SetItemMetadata),
			xfbinding(SetItemPreview),
			xfbinding(SetItemTags),
			xfbinding(SetItemTitle),
			xfbinding(SetItemUpdateLanguage),
			xfbinding(SetItemVisibility),
			xfbinding(SetLanguage),
			xfbinding(CreateQueryAllUGCRequest),
			xfbinding(CreateQueryUserUGCRequest),
			xfbinding(CreateQueryUGCDetailsRequest),
			xfbinding(SendQueryUGCRequest),
			xfbinding(SetUserItemVote),
			xfbinding(StartItemUpdate),
			xfbinding(StartPlaytimeTracking),
			xfbinding(StopPlaytimeTracking),
			xfbinding(StopPlaytimeTrackingForAllItems),
			xfbinding(SubscribeItem),
			xfbinding(UnsubscribeItem),
			xfbinding(SubmitItemUpdate),
			xfbinding(SuspendDownloads),
			xfbinding(UpdateItemPreviewFile),
			xfbinding(UpdateItemPreviewVideo),
			xfbinding(ShowWorkshopEULA),
			xfbinding(GetQueryUGCResult),
			xfbinding(GetQueryUGCAdditionalPreview),
			xfbinding(GetQueryUGCChildren),
			xfbinding(GetQueryUGCNumTags),
			xfbinding(GetQueryUGCTag),
			xfbinding(GetQueryUGCTagDisplayName),
			xfbinding(GetQueryUGCKeyValueTag),
			xfbinding(GetQueryUGCMetadata),
			xfbinding(GetQueryUGCNumAdditionalPreviews),
			xfbinding(GetQueryUGCNumKeyValueTags),
			xfbinding(GetQueryUGCPreviewURL),
			xfbinding(GetQueryUGCStatistic),

			{NULL, NULL},
		};
		lua_pushstring(L, "SteamUGC");
		lua_createtable(L, 0, 64);
		luaL_register(L, NULL, lib);
		lua_settable(L, -3);

		lua_newtable(L);
		lua_pushstring(L, "SteamUGC_callback_registry");
		lua_pushvalue(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
		lua_pop(L, 1);
		return 0;
	};
#undef CALLBACKFUNC
#undef REGISTERCALLBACK
};

