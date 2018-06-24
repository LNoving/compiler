#include"buffer.h"
#include<numeric>
#include<functional>
unsigned char** buff;
// 0��ʾ���block���ڿ���
vector<block<>> BufferManager;

// ��ʼ������������
void initbuff(unsigned char** ptr)
{
	ptr = new unsigned char*[page_num];
	for (int i = 0; i < page_num; ++i)
	{
		ptr[i] = new unsigned char[BLOCK_8k];
		BufferManager.push_back(block<>(i, false));
	}
}

void ReplacePage(int needy, bool(*f)(const block<>&,const block<>&))
{
	std::sort(BufferManager.begin(), BufferManager.end(), f);
	int cnt = 0;
	for (auto i = BufferManager.begin(); i != BufferManager.end(); ++i)
	{
		if (cnt != needy && !i->ispin)
		{
			i->writeback();
			++cnt;
		}
	}
}

// ��ȡ1�����ȫ�������ֽ���, �͵���record���ֽ���
std::pair<unsigned int,unsigned int> counttablebyte(const string& tbname)
{
	string tb_db = minisql::record_path + std::to_string(catalog::catamap[tbname]) + ".db"; 
	string tb_log= catalog::cata_path + std::to_string(catalog::catamap[tbname]) + ".log";
	ifstream ifs(tb_db, std::ifstream::ate | std::ifstream::binary);
	auto tbsize = (unsigned int)ifs.tellg();
	ifs.close(); string temp; unsigned int record_size;
	ifstream ifs2(tb_log);
	ifs2 >> temp >> record_size >> record_size;
	return std::make_pair(tbsize, record_size);
}

// ����һ���������ļ�д��block
vector<int> blockgen(const string& tbname)
{
	auto tbsizeinfo = counttablebyte(tbname);
	auto recordnum = tbsizeinfo.first / tbsizeinfo.second;
	auto records_pre_blk = BLOCK_8k / tbsizeinfo.second;
	auto blknum = recordnum / records_pre_blk + (recordnum > recordnum / records_pre_blk*records_pre_blk) ? 1u : 0u;
	if (blknum > page_num)
	{
		throw runtime_error("Error: <del>the table `"+tbname+"` is too large</del>\n");
	}
	vector<unsigned int>tempi(page_num);
	std::transform(BufferManager.cbegin(), BufferManager.cend(), tempi.begin(),
		[](const block<>& blk)->unsigned int {return (blk.isdirty||blk.ispin) ? 0 : 1; });
	auto sum = std::accumulate(tempi.cbegin(), tempi.cend(), 0);
	if (sum < blknum)
	{
		ReplacePage(blknum - sum, [](const block<>& a, const block<>& b) {return a.getfreq() < b.getfreq(); });
	}
	// �ָ�ԭ˳��
	std::sort(BufferManager.begin(), BufferManager.end(),
		[](const block<>& a, const block<>& b) {return a.series < b.series; });
	vector<int> result;
	for (auto& i : BufferManager)
	{
		if ((!(i.isdirty||i.ispin)) && result.size() != blknum)
			result.push_back(i.series);
	}
	string tb_db = minisql::record_path + std::to_string(catalog::catamap[tbname]) + ".db";
	FILE* r = fopen(tb_db.c_str(), "rb");
	unsigned int dist = 0;
	for (auto i : result)
	{
		BufferManager[i].filename = tb_db;
		BufferManager[i].offset = dist;
		dist += BLOCK_8k;
		BufferManager[i].updatefreq();
		fread(buff[i], sizeof(unsigned char), BLOCK_8k, r);
	}
	fclose(r);
	return result;
}
