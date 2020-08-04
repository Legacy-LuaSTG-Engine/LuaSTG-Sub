#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include "E2DFileManager.hpp"
#include "E2DCodePage.hpp"
#include "E2DLogSystem.hpp"
#include "fcyMisc/fcyStringHelper.h"
#include "zip.h"

#define CUSTOM_ZIP_STAT (ZIP_STAT_NAME | ZIP_STAT_INDEX | ZIP_STAT_SIZE | ZIP_STAT_ENCRYPTION_METHOD)

using namespace std;
using namespace Eyes2D::IO;

//======================================

struct Archive::Impl {
	zip_t* ZipFile = nullptr;
	string path = "";//路径，可能不是绝对路径
	string password = "";//备用
};

Archive::Archive(unsigned int uid) {
	m_Impl = new Archive::Impl;
	sort.priority = 0;
	sort.uid = uid;
}

void Archive::class_init(const char* path, const char* password) {
	//检查路径
	if (path == nullptr) {
		throw E2DException(0, 0, L"Eyes2D::IO::Archive::Archive", L"File path is null.");
		return;
	}
	string frompath = path;
	if (frompath.length() < 1) {
		throw E2DException(0, 0, L"Eyes2D::IO::Archive::Archive", L"File path is blank.");
		return;
	}
	m_Impl->path = frompath;
	//开始加载
	bool load = false;//已经完成加载，失败则为false
	m_Impl->ZipFile = nullptr;
	int err; m_Impl->ZipFile = zip_open(frompath.c_str(), ZIP_RDONLY, &err);//默认视作ANSI
	if (m_Impl->ZipFile == nullptr) {
		string topath = std::move(Eyes2D::String::UTF8ToANSI(frompath));
		m_Impl->ZipFile = zip_open(topath.c_str(), ZIP_RDONLY, &err);//加载错误，可能是UTF8，需要转换为ANSI
		if (m_Impl->ZipFile != nullptr) {
			load = true;
		}
	}
	else {
		load = true;
	}
	//检查结果
	if (load) {
		if (password != nullptr) {
			//密码内一般不会带中文，所以直接设置了
			//内存不足不用管，出错不要管，反正出错也没什么问题.jpg
			zip_set_default_password(m_Impl->ZipFile, password);
			m_Impl->password = password;
		}
	}
	else {
		zip_error_t error;
		zip_error_init_with_code(&error, err);
		string errstr = zip_error_strerror(&error);
		wstring errwstr = std::move(Eyes2D::String::ANSIToUTF16(errstr));
		zip_error_fini(&error);
		throw E2DException(0, 0, L"Eyes2D::IO::Archive::Archive", wstring(L"Failed to open archive file : ") + errwstr);
	}
}

Archive::Archive(const char* path, unsigned int uid) {
	m_Impl = new Archive::Impl;
	sort.priority = 0;
	sort.uid = uid;
	class_init(path, nullptr);
}

Archive::Archive(const char* path, int priority, unsigned int uid) {
	m_Impl = new Archive::Impl;
	sort.priority = priority;
	sort.uid = uid;
	class_init(path, nullptr);
}

Archive::Archive(const char* path, const char* password, unsigned int uid) {
	m_Impl = new Archive::Impl;
	sort.priority = 0;
	sort.uid = uid;
	class_init(path, password);
}

Archive::Archive(const char* path, int priority, const char* password, unsigned int uid) {
	m_Impl = new Archive::Impl;
	sort.priority = priority;
	sort.uid = uid;
	class_init(path, password);
}

void Archive::class_del() {
	if (m_Impl->ZipFile != nullptr) {
		zip_discard(m_Impl->ZipFile);//只读模式打开，所以关闭的时候不作任何更改
	}
}

Archive::~Archive() {
	class_del();//一定要放在前面
	delete m_Impl;
}

long long Archive::file_precheck(const char* filepath) {
	//检查路径
	if (filepath == nullptr) {
		return -1;
	}
	string frompath = filepath;
	if (frompath.length() < 1) {
		return -1;
	}
	//查找文件
	bool find = false;
	zip_int64_t index = zip_name_locate(m_Impl->ZipFile, frompath.c_str(), ZIP_FL_ENC_GUESS);
	if (index < 0) {
		string topath = std::move(Eyes2D::String::UTF8ToANSI(frompath));
		index = zip_name_locate(m_Impl->ZipFile, topath.c_str(), ZIP_FL_ENC_GUESS);
		if (index >= 0) {
			find = true;
		}
	}
	else {
		find = true;
	}
	//检查结果
	if (find) {
		return index;
	}
	else {
		return -1;
	}
}

const char* Archive::GetArchivePath() {
	return m_Impl->path.c_str();
}

int Archive::GetPriority() {
	return sort.priority;
}

void Archive::SetPriority(int priority) {
	sort.priority = priority;
}

unsigned int Archive::GetUID() {
	return sort.uid;
}

bool Archive::FileExist(const char* filepath) {
	return file_precheck(filepath) >= 0;
}

fcyStream* Archive::LoadFile(const char* filepath) {
	//检查路径，查找文件
	zip_int64_t index = file_precheck(filepath);
	if (index < 0) {
		return nullptr;
	}
	//加载
	{
		//获取文件信息
		zip_stat_t zs;
		if (zip_stat_index(m_Impl->ZipFile, index, ZIP_FL_UNCHANGED, &zs) != 0) {
			return nullptr;
		}
		if (CUSTOM_ZIP_STAT != (zs.valid & CUSTOM_ZIP_STAT)) {
			return nullptr;
		}
		//打开压缩包内文件
		zip_file_t* zf = zip_fopen_index(m_Impl->ZipFile, index, ZIP_FL_ENC_GUESS);
		if (zf == nullptr) {
			return nullptr;
		}
		//申请内存
		fcyRefPointer<fcyMemStream> stream;
		try {
			stream.DirectSet(new fcyMemStream(nullptr, zs.size, true, false));//引用计数1
		}
		catch (const bad_alloc&) {
			zip_fclose(zf);//关闭过程中可能会出错，但是关我pi事
			return nullptr;
		}
		//读取内容
		zip_int64_t read = zip_fread(zf, (void*)stream->GetInternalBuffer(), zs.size);
		zip_fclose(zf);//先关掉再说
		if (read != zs.size) {
			return nullptr;
		}
		else {
			stream->AddRef();//引用计数2，这里加一次计数是因为一会返回的时候会自动释放一次……智能指针，智障指针
			return *stream;//返回指针
		}
	}
}

fcyStream* Archive::LoadEncryptedFile(const char* filepath, const char* password) {
	//检查路径，查找文件
	zip_int64_t index = file_precheck(filepath);
	if (index < 0) {
		return nullptr;
	}
	//加载
	{
		//获取文件信息
		zip_stat_t zs;
		if (zip_stat_index(m_Impl->ZipFile, index, ZIP_FL_UNCHANGED, &zs) != 0) {
			return nullptr;
		}
		if (CUSTOM_ZIP_STAT != (zs.valid & CUSTOM_ZIP_STAT)) {
			return nullptr;
		}
		if (ZIP_EM_NONE == zs.encryption_method) {
			return nullptr;
		}
		//打开压缩包内文件，密码不使用中文
		zip_file_t* zf = zip_fopen_index_encrypted(m_Impl->ZipFile, index, ZIP_FL_ENC_GUESS, password);
		if (zf == nullptr) {
			return nullptr;
		}
		//申请内存
		fcyRefPointer<fcyMemStream> stream;
		try {
			stream.DirectSet(new fcyMemStream(nullptr, zs.size, true, false));//引用计数1
		}
		catch (const bad_alloc&) {
			zip_fclose(zf);//关闭过程中可能会出错，但是关我pi事
			return nullptr;
		}
		//读取内容
		zip_int64_t read = zip_fread(zf, (void*)stream->GetInternalBuffer(), zs.size);
		zip_fclose(zf);//先关掉再说
		if (read != zs.size) {
			return nullptr;
		}
		else {
			stream->AddRef();//引用计数2，这里加一次计数是因为一会返回的时候会自动释放一次……智能指针，智障指针
			return *stream;//返回指针
		}
	}
}

long long Archive::GetFileCount() {
	return zip_get_num_entries(m_Impl->ZipFile, ZIP_FL_UNCHANGED);
}

const char* Archive::GetFileName(long long index) {
	if ((index < 0) || (index >= GetFileCount())) {
		return nullptr;
	}
	return zip_get_name(m_Impl->ZipFile, index, ZIP_FL_ENC_RAW | ZIP_FL_ENC_GUESS);
}

void Archive::ListFile() {
	zip_int64_t count = zip_get_num_entries(m_Impl->ZipFile, ZIP_FL_UNCHANGED);
	for (zip_int64_t index = 0; index < count; index++) {
		const char* pname = zip_get_name(m_Impl->ZipFile, index, ZIP_FL_ENC_RAW | ZIP_FL_ENC_GUESS);
		if (pname != nullptr) {
			Eyes2D::EYESINFO(pname);
		}
		else {
			Eyes2D::EYESERROR("NULL");
		}
	}
}

//======================================

struct FileManager::Impl {
	set<Archive*, Archive::ArchiveSort> ArchiveSet;
};

FileManager::FileManager() {
	m_Impl = new FileManager::Impl;
	m_ArchiveUID = 0;
}

FileManager::~FileManager() {
	UnloadAllArchive();
	delete m_Impl;
}

bool FileManager::LoadArchive(const char* name, int priority, const char* password) {
	Archive* zip;
	try {
		if (password != nullptr) {
			zip = new Archive(name, priority, password, m_ArchiveUID);
		}
		else {
			zip = new Archive(name, priority, m_ArchiveUID);
		}
	}
	catch (E2DException& e) {
		Eyes2D::EYESERROR(e);
		return false;
	}
	m_Impl->ArchiveSet.insert(zip);
	m_ArchiveUID++;
	Eyes2D::EYESDEBUG(string(string("Archive : ") + string(name) + string(" was loaded.")).c_str());
	return true;
}

void FileManager::SetArchivePriority(const char* name, int priority) {
	Archive* zip = nullptr;
	bool find = false;
	string frompath = name;
	string topath;
	for (auto it = m_Impl->ArchiveSet.begin(); it != m_Impl->ArchiveSet.end();) {
		topath = (*it)->GetArchivePath();
		if (frompath == topath) {
			zip = *it;//储存
			find = true;//标记
			it = m_Impl->ArchiveSet.erase(it);//擦除
			break;
		}
		else {
			it++;//下一个
		}
	}
	if (find) {
		zip->SetPriority(priority);//更改优先级
		m_Impl->ArchiveSet.insert(zip);//重新插入
	}
}

void FileManager::SetArchivePriorityByUID(unsigned int uid, int priority) {
	Archive* zip = nullptr;
	bool find = false;
	for (auto it = m_Impl->ArchiveSet.begin(); it != m_Impl->ArchiveSet.end();) {
		if (uid == (*it)->GetUID()) {
			zip = *it;//储存
			find = true;//标记
			it = m_Impl->ArchiveSet.erase(it);//擦除
			break;
		}
		else {
			it++;//下一个
		}
	}
	if (find) {
		zip->SetPriority(priority);//更改优先级
		m_Impl->ArchiveSet.insert(zip);//重新插入
	}
}

Archive* FileManager::GetArchive(const char* name) {
	string frompath = name;
	string topath;
	for (auto i : m_Impl->ArchiveSet) {
		topath = i->GetArchivePath();
		if (frompath == topath) {
			return i;
		}
	}
	Eyes2D::EYESERROR(string(string("Can't find archive : ") + string(name)).c_str());
	return nullptr;
}

Archive* FileManager::GetArchive(unsigned int pos) {
	if ((pos < 0) || (pos > m_Impl->ArchiveSet.size())) {
		return nullptr;
	}
	unsigned int cur = 0;
	for (auto i : m_Impl->ArchiveSet) {
		if (cur == pos) {
			return i;
		}
		cur++;
	}
	return nullptr;
}

Archive* FileManager::GetArchiveByUID(unsigned int uid) {
	for (auto i : m_Impl->ArchiveSet) {
		if (i->GetUID() == uid) {
			return i;
		}
	}
	return nullptr;
}

bool FileManager::ArchiveExist(const char* name) {
	return GetArchive(name) != nullptr;
}

unsigned int FileManager::GetArchiveCount() {
	return m_Impl->ArchiveSet.size();
}

void FileManager::UnloadArchive(const char* name) {
	string frompath = name;
	string topath;
	for (auto it = m_Impl->ArchiveSet.begin(); it != m_Impl->ArchiveSet.end();) {
		topath = (*it)->GetArchivePath();
		if (frompath == topath) {
			Archive* p = *it;
			it = m_Impl->ArchiveSet.erase(it);
			delete p;//只会擦除元素，不会将指针指向的内存释放
			return;
		}
		++it;
	}
	Eyes2D::EYESWARN(string(string("Can't find archive : ") + string(name)).c_str());
}

void FileManager::UnloadAllArchive() {
	for (auto it = m_Impl->ArchiveSet.begin(); it != m_Impl->ArchiveSet.end();) {
		Archive* p = *it;
		it = m_Impl->ArchiveSet.erase(it);//这个操作会自动移动到下一个迭代器
		delete p;//只会擦除元素，不会将指针指向的内存释放
	}
}

bool FileManager::FileExist(const char* filepath) {
	filesystem::path p(Eyes2D::String::UTF8ToANSI(filepath));
	filesystem::directory_entry en(p);
	if (en.is_regular_file() && filesystem::exists(p)) {
		return true;
	}
	else {
		return false;
	}
}

bool FileManager::FileExistEx(const char* filepath) {
	for (auto& z : m_Impl->ArchiveSet) {
		if (z->FileExist(filepath)) {
			return true;
		}
	}
	return FileExist(filepath);
}

fcyStream* FileManager::LoadFile(const char* filepath) {
	//从压缩包内查找
	for (auto& zip : m_Impl->ArchiveSet) {
		if (zip->FileExist(filepath)) {
			fcyStream* stream = zip->LoadFile(filepath);
			if (stream != nullptr) {
				return stream;
			}
		}
	}

	/*//从文件系统查找
	fcyStream* ss = nullptr;
	try {
		fcyRefPointer<fcyFileStream> fs;
		fs.DirectSet(new fcyFileStream(fcyStringHelper::MultiByteToWideChar(filepath, CP_UTF8).c_str(), false));
		fcyRefPointer<fcyMemStream> ms;
		ms.DirectSet(new fcyMemStream(nullptr, fs->GetLength(), true, false));
		if (FCYERR_OK != fs->SetPosition(FCYSEEKORIGIN_BEG, 0)) { return nullptr; }
		if (FCYERR_OK != ms->SetPosition(FCYSEEKORIGIN_BEG, 0)) { return nullptr; }
		if (FCYERR_OK != fs->WriteBytes(ms->GetInternalBuffer(), fs->GetLength(), nullptr)) { return nullptr; }
		ms->AddRef();
		ss = (fcyStream*)ms;
	}
	catch (...) {
		return nullptr;
	}
	return ss;
	//*/
	//*
	fstream f;
	string ansipath = Eyes2D::String::UTF8ToANSI(string(filepath));
	f.open(ansipath, ios::in | ios::binary);
	if (!f.is_open()) {
		return nullptr;
	}
	else {
		//警告：潜在的溢出错误，寻址返回的为long long，但是size_t为unsigned int
		f.seekg(0, ios::beg);
		size_t pos1 = (size_t)f.tellg();
		f.seekg(0, ios::end);
		size_t pos2 = (size_t)f.tellg();
		size_t buffersize = pos2 - pos1;
		
		fcyMemStream* stream = new fcyMemStream(nullptr, 0, true, true);
		stream->SetLength((fLen)buffersize);
		stream->SetPosition(FCYSEEKORIGIN_BEG, 0);

		f.seekg(0, ios::beg);
		f.read((char*)(stream->GetInternalBuffer()), buffersize);

		pos2 = (size_t)f.tellg();
		size_t buffersize2 = pos2 - pos1;

		f.close();

		if (buffersize != buffersize2) {
			stream->Release();//先释放
			return nullptr;
		}
		else {
			return (fcyStream*)stream;
		}
	}
	//*/
}

fcyStream* FileManager::LoadFile(const char* filepath, const char* archive) {
	Archive* zip = GetArchive(archive);
	if (zip != nullptr) {
		return zip->LoadFile(filepath);
	}
	else {
		return nullptr;
	}
}
