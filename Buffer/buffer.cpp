#include"buffer.h"
#include <windows.h>
#include <algorithm>

/**
* Constructor
* @param id The ID of the table.
* @param attrSize The size of a slot.
*/
PageManager::PageManager(int id, int attrSize, int nullBitMapSize) {
	this->id = id;
	this->attrSize = attrSize;
	this->nullBitMapSize = nullBitMapSize;
	this->recordSize = attrSize + nullBitMapSize;
	this->isQueryListLoad = false;
	/* Calculate the maximum number of records stored on a page (head or body). */
	slotsHead = (BLOCK_SIZE - sizeof(int)) / recordSize;
	slotsBody = (BLOCK_SIZE) / recordSize;
	requestPage();
	initGapList();
}

void PageManager::requestPage() {
	int fileNum = 2;
	for (int i = 0; i < fileNum; i++) {

		string fileName = to_string(this->id) + "_" + to_string(i) + ".re";
		HANDLE handle = CreateFile(
			fileName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (handle == INVALID_HANDLE_VALUE) {
			//TODO ������
		}
		DWORD fileSize = GetFileSize(handle, NULL);

		auto * buffer = (char *)calloc(BLOCK_SIZE, 1);
		/* Init room if file has not been created. */
		DWORD writtenSize = 0;
		if (fileSize == 0) {
			if (i == 0) {
				GET_INT(buffer) = 1;    //First slot
				GET_INT(buffer + sizeof(int)) = EMPTY_FLAG;
			}
			this->pages.push_back(buffer);
		}
		else {
			DWORD readSize = 0;
			ReadFile(handle, buffer, BLOCK_SIZE, &readSize, NULL);
			this->pages.push_back(buffer);
		}
		CloseHandle(handle);
	}
}

//ָ��λ�ý��������ȡ
int PageManager::extract(int pageIndex, int offset) {
	//FIXME �����ҳ��δ�����أ��ᷢ���δ���
	return GET_INT(pages.at(pageIndex) + offset);
}

void PageManager::locateSlot(int slotIndex, int* pageIndex, int* offset) {
	if (slotIndex == 0) {
		*pageIndex = 0;
		*offset = 0;
		return;
	}
	if (slotIndex <= slotsHead) {
		slotIndex--;
		*pageIndex = 0;
		*offset = slotIndex * recordSize + sizeof(int);
	}
	else {
		int index = ((slotIndex - 1 - slotsHead) / slotsBody) + 1;
		*pageIndex = index;
		*offset = (slotIndex - 1 - slotsHead - slotsBody*(index - 1)) * recordSize;
	}
}

void PageManager::initGapList() {
	//�ӿ�������ͷ����ʼѰ��
	int pi = 0;    //��ʾҳ�����
	int oi = 0;    //��ʾƫ����
	int slotIndex = extract(pi, oi);
	//FIXME �������������ѭ�����ǳ�Σ��
	for (;;) {
		gapList.push_back(slotIndex);
		locateSlot(slotIndex, &pi, &oi);
		slotIndex = extract(pi, oi);
		if (slotIndex == EMPTY_FLAG)
			break;
	}
}
/**
* ͨ�������б��õ�ǰ���м�¼��slotλ�ã�����ʼ����ѯ�б�
*/
void PageManager::initQueryList() {
	if (!queryList.empty()) {
		queryList.clear();
	}
	int lastSlot = gapList.at(0);
	bool ifAdd = true;
	for (int i = 1; i < lastSlot; i++) {
		ifAdd = true;
		for (int j = 1; j < gapList.size(); j++) {
			if (i == gapList.at(j)) {
				ifAdd = false;
				break;
			}
		}
		if (ifAdd) {
			queryList.push_back(i);
		}
	}
	//�����й������ɾ��ʱ��Ҫ��isQueryListLoad = false;
	isQueryListLoad = true;
}


/**
* Write all buffer pages to disk.
*/
void PageManager::flush() {
	for (int i = 0; i < this->pages.size(); i++) {
		string fileName = to_string(this->id) + "_" + to_string(i) + ".re";
		HANDLE handle = CreateFile(
			fileName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (handle == INVALID_HANDLE_VALUE) {
			//TODO HANDLE EXCEPTION
		}
		DWORD writtenSize = 0;
		WriteFile(handle, pages.at(i), BLOCK_SIZE, &writtenSize, NULL);
		CloseHandle(handle);
	}
}

/**
* ����һ����¼��Ӧ�ñ�֤��ǰҳ�治��ʱ�Զ�������ҳ��
* @param buffer
*/
void PageManager::insert(DynamicMemory& buffer) {
	//�ӿ����б����ҵ����һ��Ԫ�ز�����
	int slotIndex = gapList.back();
	gapList.pop_back();
	//TODO ��������
	memcpy(getPosition(slotIndex), buffer.getPtr(), recordSize);
	//ά�������б�
	int pi = 0;
	int oi = 0;
	if (gapList.empty()) {
		slotIndex++;
		gapList.push_back(slotIndex);
		//setHead(slotIndex);
		GET_INT(pages.at(pi) + oi) = slotIndex;
	}
	else {
		slotIndex = gapList.back();
	}
	locateSlot(slotIndex, &pi, &oi);
	//setSlotEmpty(slotIndex);
	GET_INT(pages.at(pi) + oi) = EMPTY_FLAG;
}


/**
* ����ָ��λ�õ�Ԫ�أ������ÿ����б�Ͳ�ѯ�б�
* @param slotIndex
*/
void PageManager::erase(int slotIndex) {
	int preLastSlot = gapList.back();
	int pi = 0;
	int oi = 0;
	locateSlot(preLastSlot, &pi, &oi);
	GET_INT(pages.at(pi) + oi) = slotIndex;
	locateSlot(slotIndex, &pi, &oi);
	GET_INT(pages.at(pi) + oi) = EMPTY_FLAG;
	gapList.push_back(slotIndex);
	auto it = find(gapList.begin(), gapList.end(), slotIndex);
	if (it != gapList.end()) {
		gapList.erase(it);
	}
}

/**
* ͨ�����λ��ö��������ָ��
* @note û��Խ�籣����
* @param slotIndex
* @return
*/
char *PageManager::getPosition(int slotIndex) {
	int pi = 0;
	int oi = 0;
	locateSlot(slotIndex, &pi, &oi);
	return (pages.at(pi) + oi);
}

/**
* �õ����м�¼�Ĵ��λ��(slot)������������³�ʼ����ѯ�б�<br>
* Get the slot for all records. The query list may be re-initialized as appropriate.
* @return queryList
*/
vector<int> &PageManager::getQueryList() {
	if (!isQueryListLoad) {
		initQueryList();
	}
	return queryList;
}

/**
* �õ����м�¼���׵�ַ��<br>Get the first address of all records.
* @return һ����¼�׵�ַ���顣<br>The vector of record first addresses.
*/
vector<char *> PageManager::getRePosAll() {
	vector<char *> pos;
	auto list = getQueryList();
	for (auto &it : list) {
		pos.push_back(getPosition(it));
	}
	return pos;
}






