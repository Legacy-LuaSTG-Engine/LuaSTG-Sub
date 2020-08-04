#pragma once
#include "E2DGlobal.hpp"
#include "fcyIO/fcyStream.h"

namespace Eyes2D {
	namespace IO {
		//压缩包
		class EYESDLLAPI Archive {
		public:
			struct ArchiveSort {
				int priority;//优先级，越大越靠前
				unsigned int uid;//uid，加载一个压缩包就自增一次；
				bool operator()(const Archive* lp, const Archive* rp) const {
					const Archive& l = *lp;
					const Archive& r = *rp;
					if (l.sort.priority != r.sort.priority) {
						//优先级大的排在前面
						return l.sort.priority > r.sort.priority;
					}
					else {
						//优先级相同uid大的排在前面
						return l.sort.uid > r.sort.uid;
					}
				}
			};
		private:
			struct Impl;
			Impl* m_Impl;
			ArchiveSort sort;//优先级，越大越靠前
		private:
			//内部方法，加载压缩包
			void class_init(const char* path, const char* password = nullptr);
			//内部方法，卸载压缩包
			void class_del();
			//内部方法，检查文件是否存在并返回索引或者错误码
			long long file_precheck(const char* filepath);
		public:
			//uid
			Archive(unsigned int uid = 0);
			//路径+uid
			Archive(const char* path, unsigned int uid = 0);
			//路径+优先级+uid
			Archive(const char* path, int priority, unsigned int uid = 0);
			//路径+密码+uid
			Archive(const char* path, const char* password, unsigned int uid = 0);
			//路径+优先级+密码+uid
			Archive(const char* path, int priority, const char* password, unsigned int uid = 0);
			~Archive();
		public:
			//获取压缩包加载时的路径
			const char* GetArchivePath();
			//获取压缩包优先级
			int GetPriority();
			//设置压缩包优先级，！仅仅只是设置哦！
			void SetPriority(int priority);
			//获取压缩包UID
			unsigned int GetUID();
		public:
			//指定文件是否存在
			bool FileExist(const char* filepath);
			//加载文件到内存流，如果被加密了则使用默认密码，加载失败则返回nullptr
			fcyStream* LoadFile(const char* filepath);
			//加载加密文件到内存流，加载失败则使用默认密码，还加载失败则返回nullptr
			fcyStream* LoadEncryptedFile(const char* filepath, const char* password);
			//获取文件数量
			long long GetFileCount();
			//获取当前索引的文件名
			const char* GetFileName(long long index);
			//列出所有文件，输出到log
			void ListFile();
		};
		
		//文件读取、文件系统
		class EYESDLLAPI FileManager {
		private:
			unsigned int m_ArchiveUID;
			struct Impl;
			Impl* m_Impl;
		public:
			FileManager();
			~FileManager();
		public:
			//加载压缩包，如果文件不存在、加载失败或者格式不支持则返回false
			bool LoadArchive(const char* name, int priority = 0, const char* password = nullptr);
			//设置已有的压缩包的优先级
			void SetArchivePriority(const char* name, int priority);
			//设置已有的压缩包的优先级
			void SetArchivePriorityByUID(unsigned int uid, int priority);
			//获取已经加载的压缩包的指针，如果不存在则返回nullptr
			Archive* GetArchive(const char* name);
			Archive* GetArchive(unsigned int pos);
			//获取已经加载的压缩包的指针，如果不存在则返回nullptr
			Archive* GetArchiveByUID(unsigned int uid);
			//判断压缩包是否已加载
			bool ArchiveExist(const char* name);
			//获取资源包数量
			unsigned int GetArchiveCount();
			//卸载压缩包
			void UnloadArchive(const char* name);
			//卸载所有压缩包
			void UnloadAllArchive();
			//指定文件是否存在
			bool FileExist(const char* filepath);
			//指定文件是否存在，包括压缩包
			bool FileExistEx(const char* filepath);
			//加载文件,包括从压缩包内查找
			fcyStream* LoadFile(const char* filepath);
			fcyStream* LoadFile(const char* filepath, const char* archive);
		};
	}
}
