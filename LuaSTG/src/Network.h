#include "AppFrame.h"
#include <stdio.h>
//#include <winsock2.h>//windows.h里面已经包含winsock老版本，与winsock2版本会有冲突
#include <process.h>
#include <assert.h>

#define MAX_TCPIP_CLIENT 16
#define LOCAL_BUFFER_LENGTH 2048
#define EXCI_FREE 0
#define EXCI_CONNECTING 1
#define EXCI_CONNECTED 2
#define EXCI_DISCONNECTED 3
#define EXCI_FULL 4
#define EXCI_CLOSE 5

//读写锁，通过原子操作和自旋锁定来实现读写互斥
/*
struct EX_RW_LOCK{
	volatile long mode;
	EX_RW_LOCK(){
		mode = 1;//0 write 1 none 2+ read
	}
	void ReadStart(){
		volatile long current_mode = mode;//Create A SnapShot of current mode
		while (current_mode == 0 || //In read mode
			InterlockedCompareExchange(&mode, current_mode + 1, current_mode) != current_mode){ //mode has changed by another thread
			Sleep(0);//Sleep a little time
			current_mode = mode;//Update snapshot
		}
	}
	void ReadEnd(){
#ifdef _DEBUG
		assert(InterlockedDecrement(&mode) != 0);
#else
		InterlockedDecrement(&mode);
#endif // _DEBUG
	}
	void WriteStart(){
		while (InterlockedCompareExchange(&mode, 0, 1) != 1)
		{
			Sleep(0);//Sleep a little time
		}
	}
	void WriteEnd(){
#ifdef _DEBUG
		assert(InterlockedIncrement(&mode) == 1);
#else
		InterlockedIncrement(&mode);
#endif // _DEBUG	
	}
};
*/
//即将根据C++17重写的读写锁
struct EX_RW_LOCK {
	volatile long mode;
	EX_RW_LOCK() {
		mode = 1;//0 write 1 none 2+ read
	}
	void ReadStart() {
		volatile long current_mode = mode;//Create A SnapShot of current mode
		while (current_mode == 0 || //In read mode
			InterlockedCompareExchange(&mode, current_mode + 1, current_mode) != current_mode) { //mode has changed by another thread
			Sleep(0);//Sleep a little time
			current_mode = mode;//Update snapshot
		}
	}
	void ReadEnd() {
#ifdef _DEBUG
		assert(InterlockedDecrement(&mode) != 0);
#else
		InterlockedDecrement(&mode);
#endif // _DEBUG
	}
	void WriteStart() {
		while (InterlockedCompareExchange(&mode, 0, 1) != 1)
		{
			Sleep(0);//Sleep a little time
		}
	}
	void WriteEnd() {
#ifdef _DEBUG
		assert(InterlockedIncrement(&mode) == 1);
#else
		InterlockedIncrement(&mode);
#endif // _DEBUG	
	}
};


//TCP连接类
class EXTCPIPSERVERCLIENTINFO :public EX_RW_LOCK{
public:
	EXTCPIPSERVERCLIENTINFO();
	~EXTCPIPSERVERCLIENTINFO();
	volatile int status;
	volatile int buffer_head;
	volatile int buffer_tail;
	char *buffer;
	SOCKET clientsocket;
	SOCKADDR_IN clientaddr;
	void Create(SOCKET s, SOCKADDR_IN s1);
	bool Connect(const char *host, int port);
	void Stop();
	bool Send(const char *data, int count);
	int Receive(char *out_data, int max_count);

	void clean_up();
	static void _server_listen_thread(void *p);
	void server_listen_thread();

};

//游戏联机服务器
//目前该功能在独立的控制台应用中使用
class EXTCPIPSERVER{
private:
	int status;
	volatile bool stop;
	volatile bool stop_boardcast;
	EXTCPIPSERVERCLIENTINFO m_clients[MAX_TCPIP_CLIENT];
	SOCKET serversocket;
public:
	EXTCPIPSERVER();
	~EXTCPIPSERVER();
	bool Start(int port);
	void Stop();

	static void static_server_thread(void *p);
	void server_thread();

	static void static_server_boardcast_thread(void *p);
	void server_boardcast_thread();

	void Send(const char *data, int count);

};

//定长字符串缓冲区
//可回环式储存数据，当到达缓冲区尾部，则自动跳转到缓冲区头
//！警告，如果获取数据时数据长度过长，会获得重复的数据
class EXSTRINGBUFFER :public EX_RW_LOCK{
	volatile int buffer_head;
	volatile int buffer_tail;
	char *buffer;
	int size;
public:
	EXSTRINGBUFFER(int count){
		size = count;
		buffer = new char[size + 1];
		buffer_head = 0;
		buffer_tail = 0;
	}
	~EXSTRINGBUFFER(){
		delete buffer;
	}
	void Push(const char *p){
		WriteStart();
		while (*p){
			buffer[buffer_head++] = *(p++);
			buffer_head = buffer_head%size;
			if (buffer_head == buffer_tail){
				break;
			}
		}
		buffer[buffer_head++] = 0;
		buffer_head = buffer_head%size;
		WriteEnd();
	}
	int Get(char *out_data, int max_count)
	{
		int i = 0;
		WriteStart();
		if (buffer){
			while (buffer[buffer_tail] && i < max_count && buffer_tail != buffer_head)
			{
				out_data[i] = buffer[buffer_tail];
				buffer_tail = (buffer_tail + 1) % size;
				i++;
			}
			if (i && !buffer[buffer_tail]){
				buffer_tail = (buffer_tail + 1) % size;
			}
		}
		out_data[i] = 0;
		WriteEnd();
		return i;
	}
};
