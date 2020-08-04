// Added by ESC 20171023
#include "ESC.h"
#include <WinSock2.h>
#include "ResourceMgr.h"
#pragma comment(lib, "ws2_32.lib")

#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"
#include "Network.h"

using namespace std;
using namespace LuaSTGPlus;

//======================================
//ex+网络传输

//--------------------------------------
//vkey按键状态链表

//vkey状态链表，veky到VKEYcode的映射
//链表节点，单向链表
struct EX_ALIAS_MAP_LIST
{
	int vkey;//veky虚拟键
	int keycode;//VKEYcode
	int slot;//！废弃的变量
	EX_ALIAS_MAP_LIST *next;
	EX_ALIAS_MAP_LIST(int vk, int k){
		vkey = vk;
		keycode = k;
		next = 0;
		slot = 0;//！废弃的变量
	}
};

//销毁链表
void DestroyInputAlias(EX_ALIAS_MAP_LIST *_list){
	EX_ALIAS_MAP_LIST *ptr = _list;
	while (_list){
		_list = _list->next;
		delete ptr;
		ptr = _list;
	}
}

//将整个vkey按键状态链表编码成EX_KEY(ulong)按键状态码（位操作）
EX_KEY ParseInput(const bool *buffer, EX_ALIAS_MAP_LIST *keyalias){
	EX_KEY a = 0;
	while (keyalias){
		if (LAPP.GetKeyState(keyalias->keycode)) {
			a |= 1 << keyalias->vkey;
		}
		//if (buffer[keyalias->keycode]){
			//a |= 1 << keyalias->vkey;
		//}
		keyalias = keyalias->next;
	}
	return a;
}

//将EX_KEY(ulong)按键状态码解码回vkey状态
bool _GetVKeyState(int vkcode, EX_KEY key){
	return (key&(1 << vkcode))!=0;
}

//--------------------------------------
//网络传输设备，虚拟设备

//最主要的逻辑类，负责与服务器通信，传输数据以及总线的更新
//类的预定义
class ExInputControl;

//input端接口
class ExInputInterface: public IExCommonInput{
protected:
	int m_type;
	bool m_writer;
	unsigned long m_mask;
	int m_framestamp;
	int m_slot;
	ExInputControl *m_main;
public:
	virtual void SetSlot(int slot,ExInputControl *main){
		m_slot = slot;
		m_main = main;
	}
	virtual void SetMask(unsigned long mask){
		m_mask = mask;
	}
	virtual void AddMask(unsigned long mask){
		m_mask |= mask;
	}
	virtual void RemoveMask(unsigned long mask){
		m_mask &= ~mask;
	}
	virtual unsigned long GetMask(){
		return m_mask;
	}
	virtual void SetRead(bool isread){
		m_writer = !isread;
	}

	int GetType(){ return m_type; }
	virtual void NextFrame(int frameid){ m_framestamp=frameid; };
	virtual bool IsReady(){ return true; }
	virtual bool IsReader(){ return !m_writer; }
	virtual bool IsWriter(){ return m_writer; }

	virtual EX_KEY GetInput(){ return 0; }
	virtual void SetInput(EX_KEY key){ return; }
	virtual void ProceedLocalInput(bool rkeys[]){ return; }
	virtual void ProceedRemoteInput(int frame,EX_KEY key){ return; }
};

//vkey按键状态码链表，用于储存延迟的操作，和vkey状态链表不一样的是，这个储存的是编码好的EX_KEY(ulong)按键状态码
//链表节点，单向链表
struct EX_INPUT_BUFFER_LIST
{
	int framestamp;
	EX_KEY keys;
	EX_INPUT_BUFFER_LIST *next;
	EX_INPUT_BUFFER_LIST(int a, EX_KEY b){
		framestamp = a;
		keys = b;
		next = 0;
	}
};

//vkey按键状态码链表链表，用于管理所有槽位的输入
//！弃用的代码
class ExInputAlias{
public:
	EX_ALIAS_MAP_LIST *m_pHead;
	ExInputAlias(){
		m_pHead = 0;
	}
	~ExInputAlias(){

	}
};

//input客户端，传输与管理按键状态
class ExInputClient :public ExInputInterface, public IExLocalInput{
public:
	EX_INPUT_BUFFER_LIST *m_head;
	int m_delay;
	int m_laststamp;
	int m_isremote;
	EXSTRINGBUFFER *m_buffer;
	EX_ALIAS_MAP_LIST *m_alias;
	EXTCPIPSERVERCLIENTINFO *m_network;
	ExInputClient(){
		m_head = 0;
		m_delay = 0;
		m_alias = 0;
		m_laststamp = -1;
		m_isremote = 0;
		m_network = NULL;
	}
	virtual void SetNetwork(EXTCPIPSERVERCLIENTINFO *p){
		m_network = p;
	}
	//获取本地输入状态并广播给其他玩家
	virtual void ProceedLocalInput(bool rkeys[]) {
		char buf[1000];
		EX_KEY key = 0;
		if (m_isremote == 0) {//是本地输入
			key = ParseInput(rkeys, m_alias);
			int cur = m_framestamp + m_delay;
			if (cur <= m_laststamp) {
				//delay调整导致已经读取了当前的输入，这里放弃
				return;
			}
			m_laststamp++;
			while (m_laststamp <= cur) {
				InsertInput(m_laststamp, key);
				//todo 发送消息
				sprintf_s<1000>(buf, "K%d,%d,%d", m_slot, m_laststamp, key);
				//m_network = m_main->m_network;
				if (m_network)m_network->Send(buf, strlen(buf));
				m_laststamp++;
			}
			m_laststamp = cur;
		}
		else {//不是本地输入
			/*
			while (m_network.Receive(buf, 1000)){//把所有网络输入都拿出来
			if (buf[0] == 'K'){//得到的输入插入
			int stamp0;
			sscanf_s(buf, "K%d,%d", &stamp0, &key);
			InsertInput(stamp0, key);
			if (stamp0 > m_laststamp){
			m_laststamp = stamp0;
			}
			}
			else if (buf[0] == 'U'){//其他的放在一边
			m_buffer->Push(buf + 1);
			}
			}*/
		}
	};
	//插入远程输入状态
	virtual void ProceedRemoteInput(int frame, EX_KEY key){
		if (m_isremote){
			InsertInput(frame, key);
			if (frame > m_laststamp){
				m_laststamp = frame;
			}
		}
	}
	//根据framestamp获取应该获取的输入，获取到输入后，会销毁对应的vkey按键状态码节点
	virtual EX_KEY GetInput(){
		if (!m_head)return 0;
		EX_INPUT_BUFFER_LIST *ptr;
		while (m_head->framestamp < m_framestamp){
			ptr = m_head;
			m_head = m_head->next;
			delete ptr;
			if (!m_head)return 0;
		}
		if (m_head->framestamp > m_framestamp){
			return 0;
		}
		ptr = m_head;
		m_head = m_head->next;
		EX_KEY k = ptr->keys;
		delete ptr;
		return k;
	}
	//vkey按键状态码链表中插入一个新的EX_KEY状态码，根据framestamp进行排序
	bool InsertInput(int framestamp, EX_KEY key){
		EX_INPUT_BUFFER_LIST *p = new EX_INPUT_BUFFER_LIST(framestamp, key);
		if (!m_head){
			m_head = p;
			return true;
		}
		else{
			EX_INPUT_BUFFER_LIST *ptr = m_head;
			EX_INPUT_BUFFER_LIST *ptrl = m_head;
			if (ptr->framestamp > framestamp){
				m_head = p;
				p->next = ptr;
				return true;
			}
			while (ptr->framestamp < p->framestamp){
				ptrl = ptr;
				ptr = ptr->next;
				if (!ptr)break;
			}
			ptrl->next = p;
			p->next = ptr;
		}
		return true;
	}
	virtual void NextFrame(int frameid){
		m_framestamp = frameid; 
	};
	virtual int GetDelay(){ return m_delay; }
	virtual void SetDelay(int delay){ 
		m_delay = delay;
	}
	//向vkey状态链表中插入一个新的vkey到VKEYcode的映射
	//！有坑警告，没有检查已有的映射是否存在
	virtual int AddKeyAlias(int vkey, int key){
		EX_ALIAS_MAP_LIST *p = new EX_ALIAS_MAP_LIST(vkey, key);
		p->next = m_alias;
		m_alias = p;
		return 1;
	}
	virtual int RemoveKeyAlias(int vkey, int key){
		if (!m_alias)return 0;
		EX_ALIAS_MAP_LIST *p1 = 0;
		EX_ALIAS_MAP_LIST *p2 = m_alias;
		EX_ALIAS_MAP_LIST *p = 0;
		int i = 0;
		while (p2){
			if (p2->vkey == vkey && p2->keycode == key){
				if (!p1){
					m_alias = p2->next;
					p = p2;
					p2 = p2->next;
					delete p;
					i++;
				}
				else{
					p = p2;
					p2 = p2->next;
					p1->next = p2;
					delete p;
					i++;
				}
			}
			else {
				p1 = p2;
				p2 = p2->next;
			}
		}
		return i;
	}
	virtual int ResetKeyAlias(int key){ return 0; }
	virtual int ResetVKeyAlias(int vkey){ return 0; }
	virtual int ResetAllAlias(){
		DestroyInputAlias(m_alias);
		m_alias = 0;
		return 0;
	}
	//返回基类类型的this指针，多继承的好处（个屁
	virtual IExCommonInput *ToCommonInput(){
		return this;
	};
	//通过检查vkey按键状态码链表是否加载完成、vkey按键状态码链表头的framestamp是否已经和本地同步返回就绪状态
	virtual bool IsReady(){
		if (!m_head)return false;
		return m_head->framestamp==m_framestamp; 
	}
	virtual EXTCPIPSERVERCLIENTINFO *SetRemote(bool remote){
		m_isremote = remote;
		if (!m_buffer)m_buffer = new EXSTRINGBUFFER(2000);
		return m_network;
	
	}
	virtual void Release(){
		//m_network.Stop();
		if (m_buffer)delete m_buffer;
		DestroyInputAlias(m_alias);
		m_alias = NULL;
		EX_INPUT_BUFFER_LIST *ptr;
		while (m_head){
			ptr = m_head;
			m_head = m_head->next;
			delete ptr;
		}
		delete this;
	}
};

//玩家组
//！弃用的代码
struct EX_CLIENT_DECLARE{
	unsigned long local_player_mask;//掩码形式的本地玩家组，0则为观察者

};

//最主要的逻辑类，负责与服务器通信，传输数据以及总线的更新
class ExInputControl : public IExInputControl{
public:
	EX_CLIENT_DECLARE m_current_state;//！弃用的代码
	ExInputInterface *m_clients[MAX_INPUT_CLIENTS];
	EX_KEY m_keys[MAX_INPUT];
	EXTCPIPSERVERCLIENTINFO *m_network;
	EXSTRINGBUFFER *m_buffer;
	virtual EXSTRINGBUFFER *GetNetworkBuffer(){
		return m_buffer;
	}
	virtual bool ConnectTo(const char *dest, int port){
		if (!m_network) {
			m_network = new EXTCPIPSERVERCLIENTINFO;
		}
		m_network->Stop();
		if (m_buffer)delete m_buffer;
		m_buffer = new EXSTRINGBUFFER(30000);
		return m_network->Connect(dest, port);
	}
	
	virtual EXTCPIPSERVERCLIENTINFO *GetNetwork(){
		//if (!m_network)m_network = new EXTCPIPSERVERCLIENTINFO;
		return m_network;
	}
	int m_nextframe;
	int m_currentframe;
	int m_delay;
	int m_ignoreinput;

	//重置计数器
	virtual void Reset(){
		m_currentframe = 0;
		m_nextframe = 1;
	}

	ExInputControl(){
		memset(m_clients, 0, sizeof(void *) * MAX_INPUT_CLIENTS);
		m_currentframe = 0;
		m_nextframe = 1;
		m_delay = 0;
		m_ignoreinput = 0;
		m_network = NULL;
		m_buffer = NULL;
	}

	//每帧的更新逻辑
	bool ProceedInput(bool *localInput){
		//清空总线
		memset(m_keys, 0, sizeof(EX_KEY)*MAX_INPUT);
		if (m_ignoreinput > 0){
			m_ignoreinput--;
			return true;
		}
		int i = 0;
		int j = 0;
		ExInputInterface *input;
		//捕获本地输入
		for (i = 0; i < MAX_INPUT_CLIENTS; i++){
			if (input = m_clients[i]){
				if (input->IsWriter()){
					input->ProceedLocalInput(localInput);
				}
			}
		}
		//捕获网络输入
		if (m_network){
			char buf[1000];
			while (m_network->Receive(buf, 1000)){//把所有网络输入都拿出来
				if (buf[0] == 'K'){//得到的输入插入
					int stamp0,slot,key;
					sscanf_s(buf, "K%d,%d,%d",&slot,&stamp0, &key);
					if (m_clients[slot]){
						m_clients[slot]->ProceedRemoteInput(stamp0, key);
					}
				} else if (buf[0] == 'U'){//其他的放在一边，这些都是用户的消息，保存到缓冲区中，以便读取
					m_buffer->Push(buf + 1);
				}
			}
		}


		//检查所有的输入输出设备是否就绪
		for (i = 0; i < MAX_INPUT_CLIENTS; i++){
			if (input = m_clients[i]){
				if (!input->IsReady()){
					return false;
				}
			}
		}

		//如果检查成功，则开始传递输入输出
		//首先将输入传递给总线
		for (i = 0; i < MAX_INPUT_CLIENTS; i++){
			if (input = m_clients[i]){
				if (input->IsWriter()){
					unsigned long mask = input->GetMask();
					EX_KEY key = input->GetInput();
					for (j = 0; j < MAX_INPUT; j++){
						if (mask & (1 << j)){
							m_keys[j] |= key;
						}
					}
				}
			}
		}

		//再将总线传递给输出
		//！废弃的代码？
		for (i = 0; i < MAX_INPUT_CLIENTS; i++){
			if (input = m_clients[i]){
				if (input->IsReader()){
					unsigned long mask = input->GetMask();
					EX_KEY key = 0;
					for (j = 0; j < MAX_INPUT; j++){
						if (mask & (1 << j)){
							key |= m_keys[j];
						}
					}
					input->SetInput(key);//这个方法是空方法，别找了，估计其实现转到lua逻辑里面了
				}
			}
		}

		//告知所有输入输出可以进行下一帧处理
		for (i = 0; i < MAX_INPUT_CLIENTS; i++){
			if (input = m_clients[i]){
				input->NextFrame(m_nextframe);
			}
		}
		m_currentframe = m_nextframe;
		m_nextframe++;
		return true;

	}

	virtual bool GetVKeyState(int slot, int vkcode){
		return _GetVKeyState(vkcode, m_keys[slot]);
	}
	virtual IExLocalInput *CreateLocalInput(){
		return new ExInputClient;
	}
	virtual IExReplayInput *CreateReplayInput(){
		return NULL;
	}
	virtual bool AssignInput(int deviceslot, IExCommonInput *input){
		if (!input){
			m_clients[deviceslot] = NULL;
			return true;
		}
		ExInputInterface *p = (ExInputInterface *)input;
		p->SetSlot(deviceslot, this);
		m_clients[deviceslot] = p;
		return true;
	}
	virtual void SetIgnoreInput(int frames){
		m_ignoreinput = frames;
	}
	virtual int GetIgnoreInput(){
		return m_ignoreinput;
	}
	
	//未用到的部分，可能是遗留代码
	EX_ALIAS_MAP_LIST *m_alias;
	virtual int SPAddKeyAlias(int vkey, int key, int slot){
		EX_ALIAS_MAP_LIST *p = new EX_ALIAS_MAP_LIST(vkey, key);
		p->slot = slot;
		p->next = m_alias;
		m_alias = p;
		return 1;
	}
	virtual int SPRemoveKeyAlias(int vkey, int key,int slot){
		if (!m_alias)return 0;
		EX_ALIAS_MAP_LIST *p1 = 0;
		EX_ALIAS_MAP_LIST *p2 = m_alias;
		EX_ALIAS_MAP_LIST *p = 0;
		int i = 0;
		while (p2){
			if (p2->vkey == vkey && p2->keycode == key && p2->slot == slot){
				if (!p1){
					m_alias = p2->next;
					p = p2;
					p2 = p2->next;
					delete p;
					i++;
				}
				else{
					p = p2;
					p2 = p2->next;
					p1->next = p2;
					delete p;
					i++;
				}
			}
			else {
				p1 = p2;
				p2 = p2->next;
			}
		}
		return i;
	}
	virtual int SPResetKeyAlias(int key){ return 0; }
	virtual int SPResetVKeyAlias(int vkey){ return 0; }
	virtual int SPResetAllAlias(){
		DestroyInputAlias(m_alias);
		m_alias = 0;
		return 0;
	}
	virtual bool SPGetKeyState(int keycode){
		EX_ALIAS_MAP_LIST *p = m_alias;
		while (p){
			if (p->keycode == keycode){
				if (m_keys[p->slot] & (1 << p->vkey)){
					return true;
				}
			}
		}
		return false;
	}
};

//创建一个本地输入
IExLocalInput *CreateLocalInput(){
	return new ExInputClient;
}

//创建一个ex+InputEx类
IExInputControl * CreateInputEx()
{
	return new ExInputControl;
}

//======================================
//ex+模型读取

#include <ios>
#include <fstream>
#include <sstream>
#include <string.h>
#include <map>
using namespace std;

class MtlObj{
public:
	string path;
	MtlObj(const string &pathi){
		//path = pathi;
		mtlNum = 0;
		readfile(pathi);
	}
	string *names;
	string *textures;
	int mtlNum;
	int getIndexByName(string name);
	void getLineNum(const string &pathi);
	void readfile(const string &pathi);
	void getLength(int &a){
		a = mtlNum;
	}
};
void MtlObj::getLineNum(const string &pathi) {
	istringstream infile(pathi.c_str()); //打开指定文件  
	string sline;//每一行  
	while (getline(infile, sline)) {//从指定文件逐行读取  
		if (sline[0] == 'n'&&sline[1] == 'e')//newmtl  
			mtlNum++;
	}
	//infile.close();
}

void MtlObj::readfile(const string &pathi) {
	getLineNum(pathi);
	names = new string[mtlNum];
	textures = new string[mtlNum];
	int n = 0;
	int t = 0;

	istringstream infile(pathi.c_str()); //打开指定文件  
	string sline;//每一行  

	string value, name, texture;
	while (getline(infile, sline)) {//从指定文件逐行读取  
		if (sline != "") {
			istringstream ins(sline);
			ins >> value;
			if (value == "newmtl") {
				ins >> name;
				names[n] = name;
				n++;
			}
			else if (value == "map_Kd") {
				ins >> texture;
				textures[t] = texture;
				t++;
			}
		}
	}
	//infile.close();
}

int MtlObj::getIndexByName(string name) {
	int index = -1;
	for (int i = 0; i < mtlNum; i++) {
		if (names[i] == name) {
			index = i;
			break;
		}
	}
	return index;
}
struct NormalTexVertex
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float u;
	float v;
};


class ModelObj{
public:
	void getLineNum(const string &pathi);
	void readfile(const string &pathi);
	void initTriangles();
	void clearTriangles();
	string path;
	ModelObj(const string &pathi,const string &patht) :vnNum(0), vtNum(0), vNum(0), fNum(0){
		
		mtl = new MtlObj(patht);
		readfile(pathi);
		initTriangles();
	}
	int vnNum;
	int vtNum;
	int vNum;
	int fNum;
	string *mtArr;
	int *groupArr;
	int groupNum;
	NormalTexVertex *vertices;
	int *indices;
	MtlObj *mtl;
	std::map<int, int> groupMtlMap;
	float **vArr;
	float **vnArr;
	float **vtArr;
	int **fvArr;
	int **ftArr;
	int **fnArr;

};

void ModelObj::getLineNum(const string &pathi) {
	istringstream infile(pathi.c_str());; //打开指定文件  
	string sline;//每一行  
	while (getline(infile, sline)) {//从指定文件逐行读取  
		if (sline[0] == 'v') {
			if (sline[1] == 'n')
				vnNum++;
			else if (sline[1] == 't')
				vtNum++;
			else
				vNum++;
		}
		if (sline[0] == 'f'){
			fNum++;
		}
	}
	//infile.close();

	istringstream ifile(pathi.c_str());
	string value, um, group, face;
	mtArr = new string[fNum];
	groupArr = new int[fNum];
	groupNum = 0;
	int fi = 0;
	while (getline(ifile, sline)) {
		if (sline != ""){
			istringstream ins(sline);
			ins >> value;
			if (value == "usemtl") {
				ins >> um;
				int mtlId = mtl->getIndexByName(um);
				groupMtlMap.insert(pair<int, int>(groupNum, mtlId));
			}
			else if (value == "g") {
				ins >> group;
				groupNum++;
			}
			else if (value == "f") {
				ins >> face;
				mtArr[fi] = um;
				groupArr[fi] = groupNum;
				fi++;
			}
		}
	}
	//ifile.close();
}


void ModelObj::readfile(const string &pathi) {
	getLineNum(pathi);
	vertices = new NormalTexVertex[fNum * 3];
	indices = new int[fNum * 3];

	//new二维数组  
	vArr = new float*[vNum];
	for (int i = 0; i < vNum; i++)
		vArr[i] = new float[3];

	vnArr = new float*[vnNum];
	for (int i = 0; i < vnNum; i++)
		vnArr[i] = new float[3];

	vtArr = new float*[vtNum];
	for (int i = 0; i < vtNum; i++)
		vtArr[i] = new float[3];

	fvArr = new int*[fNum];
	ftArr = new int*[fNum];
	fnArr = new int*[fNum];
	for (int i = 0; i < fNum; i++) {
		fvArr[i] = new int[3];
		ftArr[i] = new int[3];
		fnArr[i] = new int[3];
	}
	istringstream infile(pathi.c_str());
	string sline;//每一行  
	int ii = 0, tt = 0, jj = 0, kk = 0;

	std::string s1;
	float f2, f3, f4;

	while (getline(infile, sline)) {
		if (sline[0] == 'v') {
			if (sline[1] == 'n') {//vn  
				istringstream ins(sline);
				ins >> s1 >> f2 >> f3 >> f4;
				vnArr[ii][0] = f2;
				vnArr[ii][1] = f3;
				vnArr[ii][2] = f4;
				ii++;
			}
			else if (sline[1] == 't') {//vt  
				istringstream ins(sline);
				ins >> s1 >> f2 >> f3 >> f4;
				vtArr[tt][0] = f2;
				vtArr[tt][1] = - f3;
				vtArr[tt][2] = f4;
				tt++;
			}
			else {//v  
				istringstream ins(sline);
				ins >> s1 >> f2 >> f3 >> f4;
				vArr[jj][0] = f2;
				vArr[jj][1] = f3;
				vArr[jj][2] = f4;
				jj++;
			}
		}
		if (sline[0] == 'f') { //存储面  
			istringstream in(sline);
			float a;
			in >> s1;//去掉f  
			int i, k, flag;
			for (i = 0; i < 3; i++) {
				in >> s1;
				//取出第一个顶点和法线索引  
				a = 0;
				flag = 1;
				for (k = 0; s1[k] != '/'; k++)
					if (s1[k] != '-')
						a = a * 10 + (s1[k] - 48);
					else
						flag = -1;
				if (flag < 0){
					fvArr[kk][i] = vNum - a + 1;
				}
				else{
					fvArr[kk][i] = a ;
				}

				a = 0;
				flag = 1;
				for (k = 0; s1[k] != '/'; k++)
					if (s1[k] != '-')
						a = a * 10 + (s1[k] - 48);
					else
						flag = -1;
				if (flag < 0){
					ftArr[kk][i] = vNum - a + 1;
				}
				else{
					ftArr[kk][i] = a ;
				}

				if (vnNum){
					a = 0;
					flag = 1;
					for (k = 0; s1[k] != '/'; k++)
						if (s1[k] != '-')
							a = a * 10 + (s1[k] - 48);
						else
							flag = -1;
					if (flag < 0){
						fnArr[kk][i] = vNum - a + 1;
					}
					else{
						fnArr[kk][i] = a ;
					}
				}
			}

			kk++;
		}
	}
	//infile.close();
}

void ModelObj::initTriangles() {
	for (int i = 0; i < fNum; i++) {
		int v1Index = i * 3;
		int v2Index = i * 3 + 1;
		int v3Index = i * 3 + 2;

		vertices[v1Index].x = vArr[fvArr[i][0] - 1][0];
		vertices[v1Index].y = vArr[fvArr[i][0] - 1][1];
		vertices[v1Index].z = vArr[fvArr[i][0] - 1][2];
// 		vertices[v1Index].nx = vnArr[fnArr[i][0] - 1][0];
// 		vertices[v1Index].ny = vnArr[fnArr[i][0] - 1][1];
// 		vertices[v1Index].nz = vnArr[fnArr[i][0] - 1][2];
		vertices[v1Index].u = vtArr[ftArr[i][0] - 1][0];
		vertices[v1Index].v = vtArr[ftArr[i][0] - 1][1];

		vertices[v2Index].x = vArr[fvArr[i][1] - 1][0];
		vertices[v2Index].y = vArr[fvArr[i][1] - 1][1];
		vertices[v2Index].z = vArr[fvArr[i][1] - 1][2];
// 		vertices[v2Index].nx = vnArr[fnArr[i][1] - 1][0];
// 		vertices[v2Index].ny = vnArr[fnArr[i][1] - 1][1];
// 		vertices[v2Index].nz = vnArr[fnArr[i][1] - 1][2];
		vertices[v2Index].u = vtArr[ftArr[i][1] - 1][0];
		vertices[v2Index].v = vtArr[ftArr[i][1] - 1][1];

		vertices[v3Index].x = vArr[fvArr[i][2] - 1][0];
		vertices[v3Index].y = vArr[fvArr[i][2] - 1][1];
		vertices[v3Index].z = vArr[fvArr[i][2] - 1][2];
// 		vertices[v3Index].nx = vnArr[fnArr[i][2] - 1][0];
// 		vertices[v3Index].ny = vnArr[fnArr[i][2] - 1][1];
// 		vertices[v3Index].nz = vnArr[fnArr[i][2] - 1][2];
		vertices[v3Index].u = vtArr[ftArr[i][2] - 1][0];
		vertices[v3Index].v = vtArr[ftArr[i][2] - 1][1];

		indices[i * 3] = v1Index;
		indices[i * 3 + 1] = v2Index;
		indices[i * 3 + 2] = v3Index;
	}

	clearTriangles();
}

void ModelObj::clearTriangles() {
	for (int i = 0; i < vNum; i++)
		delete[] * (vArr + i);
	for (int i = 0; i < vnNum; i++)
		delete[] * (vnArr + i);
	for (int i = 0; i < vtNum; i++)
		delete[] * (vtArr + i);
	for (int i = 0; i < fNum; i++) {
		delete[] * (fvArr + i);
		delete[] * (ftArr + i);
		delete[] * (fnArr + i);
	}

	delete[] vArr;
	delete[] vnArr;
	delete[] vtArr;
	delete[] fvArr;
	delete[] ftArr;
	delete[] fnArr;

	delete[] mtArr;
}

class ObjModel{
public:
	ObjModel(ModelObj* obj, string path);
	~ObjModel(){
		delete objLoader;
		delete[] vertices;
		delete[] indices;
	}
	void initVertices();
	void initTextures();
	void render();
	ModelObj *objLoader;

	f2dGraphics2DVertex * vertices;
	int vcount;
	unsigned short * indices;
	int icount;

	string TEX_PATH;
	int mtlNum;
	string tex;
};

ObjModel::ObjModel(ModelObj* obj, string path) {
	objLoader = obj;
	TEX_PATH = path;
	
	initVertices();
	initTextures();

}

void ObjModel::initVertices() {

	vertices = new f2dGraphics2DVertex[objLoader->fNum * 3];

	for (int i = 0; i < objLoader->fNum * 3; i++){
		vertices[i].x = objLoader->vertices[i].x;
		vertices[i].y = objLoader->vertices[i].y;
		vertices[i].z = objLoader->vertices[i].z;
		vertices[i].u = objLoader->vertices[i].u;
		vertices[i].v = objLoader->vertices[i].v;
		vertices[i].color = 0xFFFFFFFF;
	}
	vcount = objLoader->fNum * 3;

	indices = new unsigned short[objLoader->fNum * 3];
	for (int i = 0; i < objLoader->fNum * 3; i++)
		indices[i] = objLoader->indices[i];
	icount = objLoader->fNum * 3;
}

void ObjModel::initTextures() {
	tex = objLoader->mtl->textures[0];
}

void ObjModel::render() {
	LAPP.RenderTexture(tex.c_str(), LuaSTGPlus::BlendMode::MulAlpha, vcount, vertices, icount, indices);
}



std::map<std::string, void *> g_ObjPool;

void * LoadObj(const string &id, const string &path, const string &path2){
	auto p = g_ObjPool.find(id);
	if (p != g_ObjPool.end()){
		return p->second;
	}
	ModelObj *objLoader = new ModelObj(path, path2);
	ObjModel *objModel = new ObjModel(objLoader, "");
	g_ObjPool[id]= objModel;
	return objModel;
}

ObjModel * GetObj(string id){
	auto p = g_ObjPool.find(id);
	if (p != g_ObjPool.end()){		
		return (ObjModel *)p->second;
	}
	return NULL;
}

void RenderObj(string id){
	ObjModel *p = GetObj(id);
	if (p){
		p->render();
	}
}



ObjModel *objModel=NULL;
/*
void Test(){
	if (!objModel){
		ModelObj *objLoader = new ModelObj("c:\\reimu\\1.obj");
		objModel = new ObjModel(objLoader, "c:\\reimu\\");
	}
	objModel->render();
}*/

enum SampleStyle
{
	LINE=0,
	Bezier=1
};

struct ExBezierNode
{
	fcyVec2 pos;
	fcyVec2 pin;
	fcyVec2 pout;
};

ExBezierNode g_NodeCache[256];

int ExSampleBezierA1(lua_State* L, int count, int sampleBy, float length,float rate) // ... t(list)
{

	if (count < 2){
		lua_pop(L, 1); //... 
		lua_newtable(L); //... t(list)
		return 0;
	}


	//申请空间
	ExBezierNode *cache = count > 256 ? new ExBezierNode[count] : g_NodeCache;
	//填充空间
	ExBezierNode *last_cache = NULL;
	int i;
	for (i = 1; i <= count; i++){
		lua_rawgeti(L, -1, i);// ... t(list) t(object)

		lua_getfield(L, -1, "x");// ... t(list) t(object) <x>
		cache[i-1].pos.x = luaL_checknumber(L, -1);
		lua_pop(L, 1);// ... t(list) t(object)
		lua_getfield(L, -1, "y");// ... t(list) t(object) <y>
		cache[i-1].pos.y = luaL_checknumber(L, -1);
		lua_pop(L, 1);// ... t(list) t(object)

		lua_pop(L, 1);// ... t(list)
	}
	//计算Node连接方向
	for (i = 1; i < count; i++){
		cache[i].pin = cache[i].pos - cache[i - 1].pos;
		cache[i - 1].pout = cache[i].pin;
	}
	cache[0].pin = cache[0].pout;
	cache[count - 1].pout = cache[count - 1].pin;
	//计算Node本地方向
	fcyVec2 dir;
	for (i = 0; i < count ; i++){
		dir = cache[i].pin + cache[i].pout;
		dir = dir.GetNormalize()*rate;
		cache[i].pin = dir*(-cache[i].pin.Length()) + cache[i].pos;
		cache[i].pout = dir*(cache[i].pout.Length()) + cache[i].pos;
	}
	//BezierSample
	/* A-B-C-D   t
		P(t) = (1-t)^3 A + 3t(1-t)^2 B + 3t^2(1-t) C + t^3 D
		=  ( -A + 3B - 3C + D)t^3 + ( 3A -6B +3C )t^2 + ( -3A + 3B)t + A 

		P'(t) = 3( -A + 3B - 3C + D)t^2 + 2( 3A -6B +3C )t + ( -3A + 3B)
		      = R2 t^2 + R1 t + R0
	*/
	fcyVec2 A, B, C, D, R2, R1, R0, U3, U2, U1, U0;
	float t = 0;
	float left = 0;
	float speed = 0;
	float dt = 0;
	float simulated_length = 0;
	int flag = 0;

	int newcount = 0;

	lua_pop(L,1); //... 
	lua_newtable(L); //... t(list)

	for (i = 0; i < count - 1; i++){
		A = cache[i].pos;
		B = cache[i].pout;
		C = cache[i + 1].pin;
		D = cache[i + 1].pos;

		R2 = (D - A + B * 3 - C * 3) * 3;
		R1 = (A - B * 2 + C) * 6; 
		R0 = (B - A) * 3;

		t = 0;

		do 
		{
			flag = 0;
			dir = R2*t*t + R1*t + R0;
			speed = dir.Length();
			dt = speed ? left / speed : 0.2;
			if (dt > 0.2){
				dt = 0.2;
				flag = 1;
			}
			if (dt + t > 1){
				dt = 1 - t;
				flag = 2;
			}
			if (left <= 0){
				float l = (1 - t);
				//计算位置
				fcyVec2 P = A*(l*l*l) + B*(3 * l*l*t) + C*(3 * l* t *t) + D*(t*t*t);
				float angle = dir.CalcuAngle()*LRAD2DEGREE;
				lua_newtable(L); //... t(list) t(object)
				lua_pushnumber(L, P.x); //... t(list) t(object) <x>
				lua_setfield(L, -2, "x");//... t(list) t(object)
				lua_pushnumber(L, P.y); //... t(list) t(object) <y>
				lua_setfield(L, -2, "y");//... t(list) t(object)
				lua_pushnumber(L, angle); //... t(list) t(object) <angle>
				lua_setfield(L, -2, "rot");//... t(list) t(object)
				newcount++;
				lua_rawseti(L, -2, newcount);//... t(list)
				left = length;
				flag = 1;
			}
			//向前步进
			simulated_length = speed * dt;
			if (flag == 0 || left < simulated_length){
				left = 0;
			}
			else{
				left = left - simulated_length;
			}
			t = t + dt;
		} while (flag != 2);
	}
	//释放空间
	if (cache != g_NodeCache){
		delete [] cache;
	}
	return count;
}


sbuffer TransformStrSingle(sbuffer source, int sourcepage, int targetpage){
	if (sourcepage == targetpage)return source;
	const char *ptr = &source[0];
	sbuffer x;
	DWORD dBufSize = 0;
	if (sourcepage != CP_UNICODE){
		int cnt = source.size();
		dBufSize = MultiByteToWideChar(sourcepage, 0, ptr, cnt, NULL, 0);

		x.resize(dBufSize * 2);
		wchar_t *ptrw = (wchar_t *)&x[0];
		MultiByteToWideChar(sourcepage, 0, ptr, cnt, ptrw, dBufSize);
		if (targetpage == CP_UNICODE){
			return x;
		}
	}
	else{
		x = source;
		dBufSize = source.size() >> 1;
	}
	wchar_t *ptrw = (wchar_t *)&x[0];
	DWORD dBufSize2 = WideCharToMultiByte(targetpage, 0, ptrw, dBufSize, NULL, 0, NULL, FALSE);
	sbuffer x2;
	x2.resize(dBufSize2);
	char * ptr2 = &x2[0];
	WideCharToMultiByte(targetpage, 0, ptrw, dBufSize, ptr2, dBufSize2, NULL, FALSE);
	return x2;
}

/*
CODEPAGECOUNTER::CODEPAGECOUNTER()
{
	ansi = 0;
	unicode = 0;
	utf8 = 0;
	gbk = 0;
	shiftjs = 0;
	total = 0;
}

int CODEPAGECOUNTER::CheckAll(const char *buffer)
{
	const wchar_t *buffer2;
	int cnt = 0;
	while (buffer[0]){
		buffer2 = (const wchar_t *)buffer;
		int uct = 1;
		if (buffer[0] > 0)
		{
			ansi++;
		}
		else
		{
			uct = 2;
			unsigned short c = buffer2[0];
			c = (c >> 8) + ((c & 0xFF) << 8);
			if (c >= 0x8140 && c <= 0x9FFC)
			{
				shiftjs++;
			}
			if (c >= 0x3041 && c <= 0x30f6)
			{
				unicode++;
			}
			if (c >= 0xB0A1 && c <= 0xE03F)
			{
				gbk++;
			}
		}
		buffer += uct;
		total++;
		cnt++;
	}
	return cnt;
}

int CODEPAGECOUNTER::getLikelyCode(int def)
{
	if (shiftjs || unicode || gbk){
		if (shiftjs > unicode + gbk)
		{
			return CP_SHIFT_JS;
		}
		if (unicode > shiftjs + gbk){
			return CP_UNICODE;
		}
		if (gbk > shiftjs + unicode){
			return CP_GBK;
		}
	}
	return def;
}

CODEDSTR & CODEDSTR::operator=(int page)
{
	buffer = TransformStrSingle(buffer, codepage, page);
	codepage = page;
	return *this;
}

CODEDSTR & CODEDSTR::operator=(const CODEDSTR&t)
{
	if (codepage == 0)
	{
		codepage = t.codepage;
		buffer = t.buffer;
	}
	else
	{
		buffer = TransformStrSingle(t.buffer, t.codepage, codepage);
	}
	return *this;
}

CODEDSTR::CODEDSTR(int page)
{
	codepage = page;
}

CODEDSTR::CODEDSTR()
{
	codepage = 0;
}

int CODEDSTR::getstring(char *str, int maxcnt)
{
	int i = buffer.size();
	if (maxcnt < i)i = maxcnt;
	memcpy(str, &buffer[0], i);
	str[i] = 0;
	return i;
}

char * CODEDSTR::getstring()
{
	int i = buffer.size();
	char *p = new char[i + 1];
	memcpy(p, &buffer[0], i);
	p[i] = 0;
	return p;
}

wchar_t * CODEDSTR::getwstring()
{
	int i = buffer.size() >> 1;
	wchar_t *p = new wchar_t[i + 1];
	memcpy(p, &buffer[0], buffer.size());
	p[i] = 0;
	return p;
}

void CODEDSTR::getwstring(std::wstring &ws)
{
	wchar_t *buf = getwstring();
	ws = buf;
	delete[] buf;
}

bool CODEDSTR::LoadBuffer(const void *tbuffer, int tcodepage)
{
	if (tcodepage == 0)
	{
		CODEPAGECOUNTER counter;
		counter.CheckAll((const char *)tbuffer);
		tcodepage = counter.getLikelyCode(codepage);
	}
	if (tcodepage == CP_UNICODE)
	{
		const wchar_t *p = (const wchar_t *)tbuffer;
		int n = strlenT(p);
		buffer.resize(n * 2);
		memcpy(&buffer[0], p, n * 2);
		codepage = tcodepage;
	}
	else
	{
		const char *p = (const char *)tbuffer;
		int n = strlen(p);
		buffer.resize(n);
		memcpy(&buffer[0], p, n);
		codepage = tcodepage;
	}
	return true;
}
//*/