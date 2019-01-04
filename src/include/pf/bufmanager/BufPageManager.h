#ifndef BUF_PAGE_MANAGER
#define BUF_PAGE_MANAGER

#include "../utils/pagedef.h"
#include "../fileio/FileManager.h"

#include <vector>

/*
 * BufPageManager
 * 实现了一个缓存的管理器
 */
struct BufPageManager {
public:
	std::map<int, int> replaceMarks;
    std::map<std::pair<int, int>, int> indexes;
	std::map<int, std::pair<int, int>> backward;
    std::map<int, BufType> addrs;
    std::map<int, bool> dirty;
    int usedIndex;

	FileManager* fileManager;

	BufType allocMem() {
		return new unsigned int[(PAGE_SIZE >> 2)];
	}
	BufType fetchPage(int fileID, int pageID, int& index) {
		auto p = std::make_pair(fileID, pageID);
		if (indexes.find(p) == indexes.end()) {
            index = indexes[p] = usedIndex++;
			backward[index] = p;
            addrs[index] = allocMem();
            dirty[index] = false;
        }
        else
            index = indexes[p];
        return addrs[index];
	}
public:
	/*
	 * @函数名allocPage
	 * @参数fileID:文件id，数据库程序在运行时，用文件id来区分正在打开的不同的文件
	 * @参数pageID:文件页号，表示在fileID指定的文件中，第几个文件页
	 * @参数index:函数返回时，用来记录缓存页面数组中的下标
	 * @参数ifRead:是否要将文件页中的内容读到缓存中
	 * 返回:缓存页面的首地址
	 * 功能:为文件中的某一个页面获取一个缓存中的页面
	 *           缓存中的页面在缓存页面数组中的下标记录在index中
	 *           并根据ifRead是否为true决定是否将文件中的内容写到获取的缓存页面中
	 * 注意:在调用函数allocPage之前，调用者必须确信(fileID,pageID)指定的文件页面不存在缓存中
	 *           如果确信指定的文件页面不在缓存中，那么就不用在hash表中进行查找，直接调用替换算法，节省时间
	 */
	BufType allocPage(int fileID, int pageID, int& index, bool ifRead = false) {
		BufType b = fetchPage(fileID, pageID, index);
		if (ifRead) {
			fileManager->readPage(fileID, pageID, b, 0);
		}
		return b;
	}
	/*
	 * @函数名getPage
	 * @参数fileID:文件id
	 * @参数pageID:文件页号
	 * @参数index:函数返回时，用来记录缓存页面数组中的下标
	 * 返回:缓存页面的首地址
	 * 功能:为文件中的某一个页面在缓存中找到对应的缓存页面
	 *           文件页面由(fileID,pageID)指定
	 *           缓存中的页面在缓存页面数组中的下标记录在index中
	 *           首先，在hash表中查找(fileID,pageID)对应的缓存页面，
	 *           如果能找到，那么表示文件页面在缓存中
	 *           如果没有找到，那么就利用替换算法获取一个页面
	 */
	BufType getPage(int fileID, int pageID, int& index) {
		auto p = std::make_pair(fileID, pageID);
		index = -1;
		if (indexes.find(p) != indexes.end())
			index = indexes[p];
		int repMark = fileManager->getReplaceMark(fileID);
		if (index != -1 && replaceMarks.find(fileID) != replaceMarks.end() && repMark == replaceMarks[fileID]) {
			access(index);
			return addrs[index];
		}
		else {
			replaceMarks[fileID] = repMark;
			BufType b = fetchPage(fileID, pageID, index);
			fileManager->readPage(fileID, pageID, b, 0);
			return b;
		}
	}
	/*
	 * @函数名access
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:标记index代表的缓存页面被访问过，为替换算法提供信息
	 */
	void access(int index) { }

	/*
	 * @函数名markDirty
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:标记index代表的缓存页面被写过，保证替换算法在执行时能进行必要的写回操作，
	 *           保证数据的正确性
	 */
	void markDirty(int index) {
		dirty[index] = true;
	}

	/*
	 * @函数名release
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 */
	void release(int index) {
		writeBack(index);
	}

	/*
	 * @函数名writeBack
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据需要根据脏页标记决定是否写到对应的文件页面中
	 */
	void writeBack(int index) {
		if (backward.find(index) == backward.end())
			return;
		auto fp = backward[index];
		if (dirty[index])
			fileManager->writePage(fp.first, fp.second, addrs[index], 0);
		dirty.erase(index);
		delete[] addrs[index];
		addrs.erase(index);
		backward.erase(index);
		indexes.erase(fp);
	}
	/*
	 * @函数名close
	 * 功能:将所有缓存页面归还给缓存管理器，归还前需要根据脏页标记决定是否写到对应的文件页面中
	 */
	void close() {
		std::vector<int> remains;
		for (auto i : backward)
			remains.push_back(i.first);
		for (auto i : remains)
			writeBack(i);
		replaceMarks.clear();
		indexes.clear();
		backward.clear();
		addrs.clear();
		dirty.clear();
		usedIndex = 1;
	}

	BufPageManager(FileManager* fm) {
		fileManager = fm;
	}
};
#endif
