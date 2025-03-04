#pragma once

template<class T>
class LuaCallback : public CCallback<LuaCallback<T>, T>
{
private:
	lua_State* m_L;
	int m_luaCallbackRef;
public:
	LuaCallback<T>(lua_State* L, int luaCallbackRef) : m_L(L), m_luaCallbackRef(luaCallbackRef), 
		CCallback<LuaCallback<T>, T>(this, &LuaCallback<T>::OnComplete){}
	void OnComplete(T* pResult)
	{
		lua_pushstring(m_L, "callback_registry");
		lua_gettable(m_L, LUA_REGISTRYINDEX);
		lua_rawgeti(m_L, -1, m_luaCallbackRef);
		lua_remove(m_L, -2);

		if (lua_isfunction(m_L, -1)) {
			lua_pushinteger(m_L, pResult->m_eResult);
			if (lua_pcall(m_L, 2, 0, 0) != LUA_OK) {
				lua_error(m_L);
			}
		}
		lua_pushstring(m_L, "callback_registry");
		lua_gettable(m_L, LUA_REGISTRYINDEX);
		luaL_unref(m_L, -1, m_luaCallbackRef);
		lua_pop(m_L, 1);
		delete this;
	}
};


struct xSteamUGC {

	static int AddAppDependency(lua_State* L) {
		auto const nPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		auto const nAppID = static_cast<AppId_t>(lua_to_uint32(L, 2));

		auto const result = SteamUGC()->AddAppDependency(nPublishedFileID, nAppID);
		lua_push_SteamAPICall_t(L, result);

		return 1;
	}
	static int AddDependency(lua_State* L) {
		auto const nParentPublishedFileID = lua_to_PublishedFileId_t(L, 1);
		auto const nChildPublishedFileID = lua_to_PublishedFileId_t(L, 2);

		auto const result = SteamUGC()->AddDependency(nParentPublishedFileID, nChildPublishedFileID);
		lua_push_SteamAPICall_t(L, result);

		return 1;
	}
	static int AddExcludedTag(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pTagName = luaL_checkstring(L, 2);

		const bool bSuccess = SteamUGC()->AddExcludedTag(handle, pTagName);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemKeyValueTag(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const char* pchValue = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->AddItemKeyValueTag(handle, pchKey, pchValue);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemPreviewFile(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pszPreviewFile = luaL_checkstring(L, 2);
		const int pPreviewType = luaL_checkinteger(L, 3);
		const bool bSuccess = SteamUGC()->AddItemPreviewFile(handle, pszPreviewFile, EItemPreviewType(pPreviewType));

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemPreviewVideo(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pszVideoID = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->AddItemPreviewVideo(handle, pszVideoID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddItemToFavorites(lua_State* L) {
		const int nAppID = luaL_checkinteger(L, 1);
		const int nPublishedFileID = luaL_checkinteger(L, 2);
		const int SteamAPICall = SteamUGC()->AddItemToFavorites(nAppID, nPublishedFileID);

		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int AddRequiredKeyValueTag(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const char* pchValue = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->AddRequiredKeyValueTag(handle, pchKey, pchValue);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddRequiredTag(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pTagName = luaL_checkstring(L, 2);

		const bool bSuccess = SteamUGC()->AddRequiredTag(handle, pTagName);

		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int BInitWorkshopForGameServer(lua_State* L) {
		const int unWorkshopDepotID = luaL_checkinteger(L, 1);
		const char* pszFolder = luaL_checkstring(L, 2);
		const int b = SteamUGC()->BInitWorkshopForGameServer(unWorkshopDepotID, pszFolder);

		lua_pushboolean(L, b);
		return 1;
	}
	static int CreateItem(lua_State* L) {
		const int nConsumerAppID = luaL_checkinteger(L, 1);
		const int eFileType = luaL_checkinteger(L, 2);
		const int SteamAPICall = SteamUGC()->CreateItem(nConsumerAppID, EWorkshopFileType(eFileType));
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int DeleteItem(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const int SteamAPICall = SteamUGC()->DeleteItem(nPublishedFileID);
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int DownloadItem(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const bool bHighPriority = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->DownloadItem(nPublishedFileID, bHighPriority);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int GetAppDependencies(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const int SteamAPICall = SteamUGC()->GetAppDependencies(nPublishedFileID);
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int GetItemDownloadInfo(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		uint64 BytesDownloaded = 0;
		uint64 BytesTotal = 0;
		if (SteamUGC()->GetItemDownloadInfo(nPublishedFileID, &BytesDownloaded, &BytesTotal)) {
			lua_pushnumber(L, BytesDownloaded);
			lua_pushnumber(L, BytesTotal);
			return 2;
		}
		return 0;
	}
	static int GetItemState(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		int eState = SteamUGC()->GetItemState(nPublishedFileID);
		lua_pushinteger(L, eState);
		return 1;
	}
	static int GetItemUpdateProgress(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		uint64 BytesDownloaded = 0;
		uint64 BytesTotal = 0;
		if (SteamUGC()->GetItemUpdateProgress(nPublishedFileID, &BytesDownloaded, &BytesTotal)) {
			lua_pushnumber(L, BytesDownloaded);
			lua_pushnumber(L, BytesTotal);
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
		const int cMaxEntries = luaL_checkinteger(L, 1);
		uint64* pvecPublishedFileID = new uint64[cMaxEntries];
		uint32 unNumItems = SteamUGC()->GetSubscribedItems(pvecPublishedFileID, cMaxEntries);
		lua_createtable(L, unNumItems, 0);
		for (uint32 i = 0; i < unNumItems; i++) {
			lua_pushinteger(L, (lua_Number)pvecPublishedFileID[i]);
			lua_rawseti(L, 2, i + 1);
		}
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int GetUserItemVote(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const int SteamAPICall = SteamUGC()->GetUserItemVote(nPublishedFileID);
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int ReleaseQueryUGCRequest(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const bool bResult = SteamUGC()->ReleaseQueryUGCRequest(handle);
		lua_pushboolean(L, bResult);
		return 1;
	}
	static int RemoveAppDependency(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const int nAppID = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveAppDependency(nPublishedFileID, nAppID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveDependency(lua_State* L) {
		const int nParentPublishedFileID = luaL_checkinteger(L, 1);
		const int nChildPublishedFileID = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveDependency(nParentPublishedFileID, nChildPublishedFileID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveItemFromFavorites(lua_State* L) {
		const int nAppID = luaL_checkinteger(L, 1);
		const int nPublishedFileID = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveItemFromFavorites(nAppID, nPublishedFileID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveItemKeyValueTags(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->RemoveItemKeyValueTags(handle, pchKey);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveItemPreview(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const int iPreviewIndex = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveItemPreview(handle, iPreviewIndex);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int AddContentDescriptor(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const int descid = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->AddContentDescriptor(handle, EUGCContentDescriptorID(descid));
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int RemoveContentDescriptor(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const int descid = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->RemoveContentDescriptor(handle, EUGCContentDescriptorID(descid));
		lua_pushboolean(L, bSuccess);
		return 1;
	}

	static int SetItemContent(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pszContentFolder = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemContent(handle, pszContentFolder);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemDescription(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchDescription = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemDescription(handle, pchDescription);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemMetadata(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchMetadata = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemMetadata(handle, pchMetadata);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemPreview(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pszPreviewFile = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemPreview(handle, pszPreviewFile);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemTags(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char** ppchTags = new const char* [10];
		for (int i = 0; i < 10; i++) {
			lua_rawgeti(L, 2, i + 1);
			ppchTags[i] = luaL_checkstring(L, -1);
			lua_pop(L, 1);
		}
		SteamParamStringArray_t tags;
		tags.m_ppStrings = ppchTags;
		tags.m_nNumStrings = 10;
		const bool bSuccess = SteamUGC()->SetItemTags(handle, &tags, 10);
		lua_pushboolean(L, bSuccess);
		delete[] ppchTags;
		return 1;
	}
	static int SetItemTitle(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchTitle = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemTitle(handle, pchTitle);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemUpdateLanguage(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchLanguage = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetItemUpdateLanguage(handle, pchLanguage);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetItemVisibility(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const int eVisibility = luaL_checkinteger(L, 2);
		const bool bSuccess = SteamUGC()->SetItemVisibility(handle, ERemoteStoragePublishedFileVisibility(eVisibility));
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SetLanguage(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchLanguage = luaL_checkstring(L, 2);
		const bool bSuccess = SteamUGC()->SetLanguage(handle, pchLanguage);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int CreateQueryAllUGCRequest(lua_State* L) {
		const int eQueryType = luaL_checkinteger(L, 1);
		const int eFileType = luaL_checkinteger(L, 2);
		const int eCreatorAppID = luaL_checkinteger(L, 3);
		const int eConsumerAppID = luaL_checkinteger(L, 4);
		const int unPage = luaL_checkinteger(L, 5);
		const int UGCQueryHandle = SteamUGC()->CreateQueryAllUGCRequest(EUGCQuery(eQueryType),
			EUGCMatchingUGCType(eFileType), eCreatorAppID, eConsumerAppID, unPage);
		lua_pushinteger(L, UGCQueryHandle);
		return 1;
	}
	static int CreateQueryUserUGCRequest(lua_State* L) {
		const int accountID = luaL_checkinteger(L, 1);
		const int eListType = luaL_checkinteger(L, 2);
		const int eMatchingUGCType = luaL_checkinteger(L, 3);
		const int eSortOrder = luaL_checkinteger(L, 4);
		const int nCreatorAppID = luaL_checkinteger(L, 5);
		const int nConsumerAppID = luaL_checkinteger(L, 6);
		const int unPage = luaL_checkinteger(L, 7);
		const int UGCQueryHandle = SteamUGC()->CreateQueryUserUGCRequest(accountID, EUserUGCList(eListType),
			EUGCMatchingUGCType(eMatchingUGCType), EUserUGCListSortOrder(eSortOrder), nCreatorAppID, nConsumerAppID, unPage);
		lua_pushinteger(L, UGCQueryHandle);
		return 1;
	}

	static int SendQueryUGCRequest(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const int SteamAPICall = SteamUGC()->SendQueryUGCRequest(handle);
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int SetUserItemVote(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const bool bVoteUp = luaL_checkinteger(L, 2);
		const int SteamAPICall = SteamUGC()->SetUserItemVote(nPublishedFileID, bVoteUp);
		lua_pushboolean(L, SteamAPICall);
		return 1;
	}
	static int StartItemUpdate(lua_State* L) {
		const int nConsumerAppID = luaL_checkinteger(L, 1);
		const int nPublishedFileID = luaL_checkinteger(L, 2);
		const int handle = SteamUGC()->StartItemUpdate(nConsumerAppID, nPublishedFileID);
		lua_pushinteger(L, handle);
		return 1;
	}
	static int StartPlaytimeTracking(lua_State* L) {
		const int unNumPublishedFileIDs = luaL_checkinteger(L, 2);
		uint64* pvecPublishedFileID = new uint64[unNumPublishedFileIDs];
		for (int i = 0; i < unNumPublishedFileIDs; i++) {
			lua_rawgeti(L, 2, i + 1);
			pvecPublishedFileID[i] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		const int SteamAPICall = SteamUGC()->StartPlaytimeTracking(pvecPublishedFileID, unNumPublishedFileIDs);
		lua_pushinteger(L, SteamAPICall);
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int StopPlaytimeTracking(lua_State* L) {
		const int unNumPublishedFileIDs = luaL_checkinteger(L, 2);
		uint64* pvecPublishedFileID = new uint64[unNumPublishedFileIDs];
		for (int i = 0; i < unNumPublishedFileIDs; i++) {
			lua_rawgeti(L, 2, i + 1);
			pvecPublishedFileID[i] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		const int SteamAPICall = SteamUGC()->StopPlaytimeTracking(pvecPublishedFileID, unNumPublishedFileIDs);
		lua_pushinteger(L, SteamAPICall);
		delete[] pvecPublishedFileID;
		return 1;
	}
	static int StopPlaytimeTrackingForAllItems(lua_State* L) {
		const int SteamAPICall = SteamUGC()->StopPlaytimeTrackingForAllItems();
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int SubscribeItem(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const bool bSuccess = SteamUGC()->SubscribeItem(nPublishedFileID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int UnsubscribeItem(lua_State* L) {
		const int nPublishedFileID = luaL_checkinteger(L, 1);
		const bool bSuccess = SteamUGC()->UnsubscribeItem(nPublishedFileID);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int SubmitItemUpdate(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const char* pchChangeNote = luaL_checkstring(L, 2);
		const int SteamAPICall = SteamUGC()->SubmitItemUpdate(handle, pchChangeNote);
		lua_pushinteger(L, SteamAPICall);
		return 1;
	}
	static int SuspendDownloads(lua_State* L) {
		const bool bSuspend = luaL_checkinteger(L, 1);
		SteamUGC()->SuspendDownloads(bSuspend);
		return 1;
	}
	static int UpdateItemPreviewFile(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
		const int index = luaL_checkinteger(L, 2);
		const char* pszPreviewFile = luaL_checkstring(L, 3);
		const bool bSuccess = SteamUGC()->UpdateItemPreviewFile(handle, index, pszPreviewFile);
		lua_pushboolean(L, bSuccess);
		return 1;
	}
	static int UpdateItemPreviewVideo(lua_State* L) {
		const int handle = luaL_checkinteger(L, 1);
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

			{NULL, NULL},
		};
		lua_pushstring(L, "SteamUGC");
		lua_createtable(L, 0, 52);
		luaL_register(L, NULL, lib);
		lua_settable(L, -3);

		lua_newtable(L);  // 创建一个新的表
		lua_pushstring(L, "callback_registry");
		lua_pushvalue(L, -2);  // 复制表
		lua_settable(L, LUA_REGISTRYINDEX);
		return 0;
	};
};