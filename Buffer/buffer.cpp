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

// ��ȡ1�����ȫ�����ݵģ��ֽ���
unsigned int counttablebyte(const string& file)
{
	ifstream ifs(file, std::ifstream::ate | std::ifstream::binary);
	auto result = (unsigned int)ifs.tellg();
	ifs.close();
	return result;
}

// ����һ���������ļ�д��block
vector<int> blockgen(const string& tbname)
{
	string sss = catalog::cata_path + std::to_string(catalog::catamap[tbname]) + ".log";
	auto tbsize = counttablebyte(sss);
	auto blknum = tbsize / BLOCK_8k + (tbsize%BLOCK_8k ? 0 : 1);
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
	FILE* r = fopen(sss.c_str(), "rb");
	unsigned int dist = 0;
	for (auto i : result)
	{
		BufferManager[i].filename = sss;
		BufferManager[i].offset = dist;
		dist += BLOCK_8k;
		BufferManager[i].updatefreq();
		fread(buff[i], sizeof(unsigned char), BLOCK_8k, r);
	}
	fclose(r);
	return result;
}
