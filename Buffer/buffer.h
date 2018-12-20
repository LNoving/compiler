#pragma once
#ifndef LLVMSQL_BUFFER_H
#define LLVMSQL_BUFFER_H
#include"..\catalog\catalog.h"
// queryable-record <=> buffer <=> DB files
template <typename T1 = unsigned int, typename T2 = bool>
class block
{
	std::pair<T1, T2> zata;
	unsigned int freq = 0u;
public:
	int series;  //series ָ��һ���ǻ������ĵڼ�ҳ
	bool isdirty = false;	// ����record�ķ�������
	// ��record�ļ����µ�db�ļ�
	string tbname;
	string filename;
	// ��ʾ��һҳ���ж���record
	int recordnum = -1;
	// ��ʾ��һҳ��ÿ��record�Ĵ�С
	int bytes = 0;
	// ��ʾ�����ļ�ͷ�ľ���
	int offset = 0;
	bool ispin = false;
	block(int t1, bool t2) :series(t1),isdirty(t2){}
	void writeback()
	{
		if (filename.length() == 0)
			throw runtime_error("filename.length()==0\n");
		char* tempuc = buff[series];
		FILE* wr = fopen(filename.c_str(), "rb+");
		fseek(wr, offset*BLOCK_8k, SEEK_SET);
		fwrite(tempuc, sizeof(unsigned char), BLOCK_8k, wr);
		fclose(wr);
		isdirty = false;
	}
	void updatefreq() { freq++; }
	const unsigned int getfreq()const { return this->freq; }
	
};

extern vector<block<>> BufferManager;
#endif // !LLVMSQL_BUFFER_H
