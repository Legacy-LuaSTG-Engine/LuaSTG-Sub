// Added by ESC 20171023
#include <ios>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <vector>

#include "AppFrame.h"
#include "ESC.h"

// model

using namespace std;
using namespace LuaSTGPlus;

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

// model manager

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
