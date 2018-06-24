#pragma once
#ifndef llvmsql_catalog
#define llvmsql_catalog
#include"../Interpreter/llvmsql.h"

void loadcata();
void make_cata(shared_ptr<CreateTableSimpleAST> T);
int getbyte(int n);

namespace catalog 
{
	extern string cata_path;
	extern std::map<string, unsigned int> catamap;
	

class SQLcol
{
public:
	string colname;
	int coltype;
	unsigned int N = 0;
	bool isprim;
	bool isunic;
	bool isnull;
	SQLcol(const string& colname,const unsigned int& coltype, const unsigned char& N, bool isprim, bool isunic, bool isnull) :
		colname(colname), coltype(coltype), N(N), isprim(isprim), isunic(isunic), isnull(isnull) {}
};

class SQLtable
{
public:
	// ��̬��Ϣ
	string tbname;
	unsigned int col_num;
	unsigned int record_size;
	vector<SQLcol> cols;
	vector<string> uniccols;
	vector<string> primcols;

	// ����ʱ��Ϣ
	bool isinmemory = false;
	int record_num = 0;
	vector<int> pages;
	int beginpos;	//blk �±�
	int endpos;	//blk �±�
	//���캯��
	SQLtable(const string& tbname,const unsigned int& col_num,const unsigned int& record_size):
		tbname(tbname),col_num(col_num),record_size(record_size){}
};
extern vector<SQLtable> tablebase;
}


#endif
