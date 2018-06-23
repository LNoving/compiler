#pragma once
#ifndef LLVMSQL_BUFFER_H
#define LLVMSQL_BUFFER_H
#include"../Interpreter/llvmsql.h"





// examine whether no record is larger than a block
// read write conflict

// query <=> record <=> buffer <=> DB files
class block
{
public:
	const string filename;
	const unsigned int offset;
	bool isdirty = false;
	bool ispin = false;
	shared_ptr<unsigned char[BUFFER_BLOCK_SIZE]> data = nullptr;
	block(const string& filename,const unsigned int& offset) :filename(filename), offset(offset)
	{
		/*fstream fs(filename);
		fs.seekp(offset);
*/
		FILE* r=fopen(filename.c_str(),"rb");
		data = make_shared<unsigned char[BUFFER_BLOCK_SIZE]>();
		std::fseek(r, offset, SEEK_SET);
		fread(*data.get(), BUFFER_BLOCK_SIZE, sizeof(unsigned char), r);
	}
	void writeback()
	{
		;
	}
};


// lru
// mru




/*
class PageManager {
public:
	int id;    ///< ��ID
	int attrSize;    ///< �������ݵ��ܴ�С
	int nullBitMapSize;    ///< ��λͼ��С
	int recordSize;    ///< ����+��λͼ���ܴ�С
	int slotsHead;    ///< ͷҳ�ܴ��������¼��
	int slotsBody;    ///< ��ͨҳ�ܴ��������¼��
	vector<char *> pages;    ///< ���е�ҳ��Ĭ�ϳ�ʼ��ʱȫ��д�뻺����������flush��ȫ��д�����
	bool isQueryListLoad;    ///< ���ڱ���Ƿ���ڲ�ѯ�б���ʵ�ָ���
	vector<int> queryList;    ///< ��ѯ�б�������������Ч�ļ�¼�Ĳ��λ
	vector<int> gapList;    ///< �����б�
	void initGapList();    ///< ��ʼ�������б��ڹ��췽���е���
	void requestPage();    ///< ��buffer manager�����ڴ�ҳ
	int extract(int pageIndex, int offset);
	void locateSlot(int slotIndex, int* pageIndex, int* offset);
	PageManager(int id, int attrSize, int nullBitMapSize);
	void insert(DynamicMemory& buffer);
	void initQueryList();
	void erase(int slotIndex);
	void flush();
	vector<int> &getQueryList();
	char* getPosition(int slotIndex);
	vector<char *> getRePosAll();
};
*/


#endif // !LLVMSQL_BUFFER_H
