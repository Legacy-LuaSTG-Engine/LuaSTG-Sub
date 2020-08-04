#include "Network.h"
#include "Global.h"

int EXTCPDEBUG = 0;


EXTCPIPSERVERCLIENTINFO::EXTCPIPSERVERCLIENTINFO()
{
	buffer = NULL;
	status = EXCI_FREE;
	clientsocket = NULL;
}

EXTCPIPSERVERCLIENTINFO::~EXTCPIPSERVERCLIENTINFO()
{
	Stop();
}

void EXTCPIPSERVERCLIENTINFO::Create(SOCKET s, SOCKADDR_IN s1)
{
	WriteStart();
	clean_up();
	buffer_head = 0;
	buffer_tail = 0;
	buffer = new char[LOCAL_BUFFER_LENGTH + 1];
	memset(buffer, 0, LOCAL_BUFFER_LENGTH + 1);
	status = EXCI_CONNECTED;
	clientsocket = s;
	clientaddr = s1;
	if (EXTCPDEBUG)printf("SV_CL_Start\n");
	_beginthread(_server_listen_thread, 0, this);
	WriteEnd();
}

bool EXTCPIPSERVERCLIENTINFO::Connect(const char *host, int port)
{
	if (host[0] == 0)return true;//disconnect
	WriteStart();
	clean_up();
	buffer_head = 0;
	buffer_tail = 0;
	buffer = new char[LOCAL_BUFFER_LENGTH + 1];
	memset(buffer, 0, LOCAL_BUFFER_LENGTH + 1);
	if ((clientsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)	//create a tcp socket
	{
		printf("Create socket fail!\n");
		//return -1;
	}
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(port);
	clientaddr.sin_addr.S_un.S_addr = inet_addr(host);

	//connect to server
	printf("Try to connect...\n");
	if (connect(clientsocket, (SOCKADDR *)&clientaddr, sizeof(clientaddr)) != 0)
	{
		printf("Connect fail!\n");
		WriteEnd();
		return false;
	}
	if (EXTCPDEBUG)printf("SV_CL_Start\n");
	status = EXCI_CONNECTED;
	_beginthread(_server_listen_thread, 0, this);
	WriteEnd();
	return true;
}

void EXTCPIPSERVERCLIENTINFO::clean_up()
{
	if (buffer){
		delete[] buffer;
		buffer = NULL;
	}
	if (clientsocket){
		closesocket(clientsocket);
		clientsocket = NULL;
	}
}

void EXTCPIPSERVERCLIENTINFO::Stop()
{
	bool waitflag = false;
	WriteStart();
	clean_up();
	if (status == EXCI_CONNECTED || status == EXCI_FULL){
		status = EXCI_CLOSE;
		waitflag = true;

	}
	WriteEnd();
	if (waitflag){
		while (status != EXCI_DISCONNECTED){
			Sleep(0);
		}
	}
}

bool EXTCPIPSERVERCLIENTINFO::Send(const char *data, int count)
{
	bool rt = false;
	WriteStart();
	if (status == EXCI_CONNECTED || status == EXCI_FULL){
		if (EXTCPDEBUG)printf("SV_CL_Send:%s\n", data);
		int t = send(clientsocket, data, count + 1, 0);
		rt = (t == count + 1);
	}
	WriteEnd();
	return rt;
}

int EXTCPIPSERVERCLIENTINFO::Receive(char *out_data, int max_count)
{
	int i = 0;
	WriteStart();
	if (buffer && (status == EXCI_CONNECTED || status == EXCI_FULL)){
		while (buffer[buffer_tail] && i < max_count && buffer_tail != buffer_head)
		{
			out_data[i] = buffer[buffer_tail];
			buffer_tail = (buffer_tail + 1) % LOCAL_BUFFER_LENGTH;
			i++;
		}
		if (i && !buffer[buffer_tail]){
			buffer_tail = (buffer_tail + 1) % LOCAL_BUFFER_LENGTH;
		}
		if (status == EXCI_FULL)status = EXCI_CONNECTED;
	}
	out_data[i] = 0;
	if (EXTCPDEBUG)if (i)printf("SV_CL_Get:%s\n", out_data);
	if(out_data[0]!='K' && i)LINFO("获得网络输入'%m'", out_data);
	WriteEnd();
	return i;
}

void EXTCPIPSERVERCLIENTINFO::_server_listen_thread(void *p)
{
	EXTCPIPSERVERCLIENTINFO *self = (EXTCPIPSERVERCLIENTINFO *)p;
	self->server_listen_thread();
	
	_endthread();
}

void EXTCPIPSERVERCLIENTINFO::server_listen_thread()
{
	char *temp_buffer = new char[LOCAL_BUFFER_LENGTH + 1];
	while (1)
	{
		//waiting for data receive
		int count = recv(clientsocket, temp_buffer, LOCAL_BUFFER_LENGTH, 0);
		WriteStart();
		if (count <= 0)
		{
			if (EXTCPDEBUG)printf("SV_CL_Failed\n");
			clean_up();
			delete[] temp_buffer;
			status = EXCI_DISCONNECTED;
			WriteEnd();
			return;
		}
		else{
			if (EXTCPDEBUG)printf("SV_CL_Receive:%s\n", temp_buffer);
			//insert content into buffer
			int i = 0;
			while (count > 0){
				buffer[buffer_head] = temp_buffer[i++];
				buffer_head = (buffer_head + 1) % LOCAL_BUFFER_LENGTH;
				if (buffer_head == buffer_tail){
					//buffer is full,wait for possible get_buffer
					buffer_head--;
					status = EXCI_FULL;
					if (EXTCPDEBUG)printf("SV_CL_Full\n");
					WriteEnd();
					while (status == EXCI_FULL)
					{
						Sleep(0);
					}
					WriteStart();
					if (status != EXCI_CONNECTED){
						if (EXTCPDEBUG)printf("SV_CL_Failed\n");
						clean_up();
						delete[] temp_buffer;
						status = EXCI_DISCONNECTED;
						WriteEnd();
						return;
					}
					buffer_head++;
				}
				count--;
			}
		}
		WriteEnd();
		//printf("%s\n", buf);
	}
}


EXTCPIPSERVER::EXTCPIPSERVER()
{
	status = EXCI_FREE;
	stop = false;
	stop_boardcast = false;
	serversocket = NULL;
}

EXTCPIPSERVER::~EXTCPIPSERVER()
{
	Stop();
}

bool EXTCPIPSERVER::Start(int port)
{
	Stop();
	if ((serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)	//create a tcp socket
	{
		printf("Create socket fail!\n");
		//return -1;
	}
	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serversocket, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Bind fail!\n");
		closesocket(serversocket);
		serversocket = NULL;
		return false;
	}
	status = EXCI_CONNECTED;
	_beginthread(static_server_thread, 0, this);
	_beginthread(static_server_boardcast_thread, 0, this);
	return true;
}

void EXTCPIPSERVER::Stop()
{
	if (serversocket){
		stop_boardcast = true;
		while (stop_boardcast){
			Sleep(0);
		}
		stop = true;
		closesocket(serversocket);
		while (stop){
			Sleep(0);
		}
		for (int i = 0; i < MAX_TCPIP_CLIENT; i++){
			m_clients[i].Stop();
		}
		if (EXTCPDEBUG)printf("SV_Stop\n");
		serversocket = NULL;
	}
}

void EXTCPIPSERVER::static_server_thread(void *p)
{
	EXTCPIPSERVER *self = (EXTCPIPSERVER *)p;
	self->server_thread();
	_endthread();
}

void EXTCPIPSERVER::server_thread()
{
	if (listen(serversocket, 5) != 0)
	{
		printf("Listen fail!\n");
		//return -1;
	}
	while (!stop){
		SOCKET temp_client = NULL;
		SOCKADDR_IN temp_addr;
		int len = sizeof(SOCKADDR_IN);
		if ((int)(temp_client = accept(serversocket, (SOCKADDR *)&temp_addr, &len)) <= 0)
		{
			if (EXTCPDEBUG)printf("SV_Failed\n");
		}
		else{
			if (EXTCPDEBUG)printf("SV_Connected\n");
			//alloc a new client position
			for (int i = 0; i < MAX_TCPIP_CLIENT; i++){
				if (m_clients[i].status == EXCI_FREE){
					m_clients[i].Create(temp_client, temp_addr);
					char hellos[10];
					hellos[0] = 'U';
					hellos[1] = 'S';
					hellos[2] = '1' + i;
					hellos[3] = 0;
					bool rt = m_clients[i].Send(hellos, 3);
					break;
				}
			}
		}
		Sleep(0);
	}
	stop = false;
}

void EXTCPIPSERVER::static_server_boardcast_thread(void *p)
{
	EXTCPIPSERVER *self = (EXTCPIPSERVER *)p;
	self->server_boardcast_thread();
	_endthread();
}

void EXTCPIPSERVER::server_boardcast_thread()
{
	char *temp_buffer = new char[LOCAL_BUFFER_LENGTH];
	while (!stop_boardcast){
		for (int i = 0; i < MAX_TCPIP_CLIENT; i++){
			int t;
			if (m_clients[i].status == EXCI_DISCONNECTED){
				m_clients[i].status = EXCI_FREE;
			}
			if ((t = m_clients[i].Receive(temp_buffer, LOCAL_BUFFER_LENGTH - 1))){
				for (int j = 0; j < MAX_TCPIP_CLIENT; j++){
					if (j != i){
						m_clients[j].Send(temp_buffer, t);
					}
				}
			}
		}
		Sleep(1);
	}
	stop_boardcast = false;
}

void EXTCPIPSERVER::Send(const char *data, int count)
{
	if (serversocket){
		for (int i = 0; i < MAX_TCPIP_CLIENT; i++){
			m_clients[i].Send(data, count);
		}
	}
}
