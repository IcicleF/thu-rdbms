# Record Management 文档

接口和功能大量地借鉴了 Stanford 课程网站上给出的接口示例。

## RID

`RID` 通过保存一条记录的页号，以及该记录相对于页头的偏移量来 `O(1)` 地定位一条记录。

`RID` 是不可变类型。

`RID` 是独立于 `RMFile` 存在的，不能完整地表示一条记录。从一个 `RMFile` 实例取得的 `RID` 用在另一个 `RMFile` 实例上（例如，作为 `getRec` 的参数传入）将得到不可预期的结果。

`RID` 提供原型分别为 `void getPage() const` 和 `void getSlot() const` 的接口分别表示记录所在的页号，和记录所在位置相对于页头的偏移量。

## RMRecord

`RMRecord` 保存一个 `RID` 实例，和一个 `std::string` 实例代表该 `RID` 对应位置的数据。

`RMRecord` 是不可变类型。

`RMRecord` 是独立于 `RMFile` 存在的，尽管它的成员确实取决于构造它的那个 `RMFile` 实例。从一个 `RMFile` 实例取得的 `RMRecord` 用在另一个 `RMFile` 实例上（例如，作为 `updateRec` 的参数传入）将得到不可预期的结果。

`RMRecord` 提供原型分别为 `RID getRID() const` 和 `void getData(char*) const` 的接口分别表示记录的 RID 和记录的内容，注意实例的内容不会随着数据库内容更新而更新。

## RMFile

`RMFile` 是一个几乎完全公有的类，主要原因是它需要暴露大量接口给 `RMManager` 和 `RMScanner`。

`RMFile` 以页为单位管理文件中的记录。它保留每页的前 16 个字节，并将剩余的 8176 个字节划分为等长的存储槽，页尾剩余的空间则被废弃。

目前，前 16 个字节的使用方式如下：

* `RecSize` (0~3)：这 4 个字节用于存储记录长度。仅在第 0 页中有数据，在后续的页中这 4 个字节固定为 0；
* `EmptyPtr` (4~5)：这 2 个字节存储一个相对于页头的偏移量，该偏移量处是本页第一个空槽的位置。所谓“第一个”并不是指最靠前的一个，而是指链表的第一个元素。另外，该存储槽有可能尚未被初始化。
* `OccPtr` (6~7)：这 2 个字节存储一个相对于页头的偏移量，该偏移量处是本页第一个记录的位置。所谓“第一个”并不是指最靠前的一个，而是指链表的第一个元素。
* `UsedSize` (8~9)：这 2 个字节存储一个相对于页头的偏移量，该偏移量及其后的空间尚未被初始化。
* `TotPages` (12~15)：在第 0 页存储一个总页数。

每条记录占用的空间等于 `createFile` 时指定的 `recordSize` 再加上 4 字节，这 4 个字节用于存储双向链表节点的 2 个指针。以 `EmptyPtr` 和 `OccPtr` 为链表头，所有空槽和所有记录分别串成两个链表。

### 插入记录

接口原型 `RID insertRec(const char* data)` 用于插入一条记录，记录的内容为 `data` 的至多前 `recordSize` 个字符，返回该记录的 RID。

插入记录时，程序从前到后遍历每页并检查页头的 `EmptyPtr` 域；如果值为 0，意味着本页已满；否则，将记录插入在当前页 `EmptyPtr` 所指示的存储槽位置。如果 `EmptyPtr` 等于 `UsedSize`，那么就无需更改；否则将该节点从 `EmptyPtr` 所指向的链表中删除。

之后，在槽中填入数据，并将记录插入 `OccPtr` 所指向的链表头。

### 检索/删除/更新记录

接口原型 `RMRecord getRec(const RID&)` 用于取得一条记录。
接口原型 `void deleteRec(const RID&)` 用于删除一条记录。
接口原型 `void updateRec(const RMRecord&)` 用于更新一条记录。

可以根据传入的 `RID` 直接检索到记录。

* 检索：构造 `RMRecord` 对象并直接返回；
* 更新：更新数据；
* 删除：将记录移出 `OccPtr` 所在的链表，并插入 `EmptyPtr` 所在的链表头。

## RecordManager
- 记录管理模块最顶层的类，用于创建、打开和关闭文件，有一个FileManager和一个BufPageManager成员变量用于文件管理。

### 具体接口

`void createFile(const char *fileName, int recordSize)`

- 创建并打开文件，并在首页(Page0)写入recordSize

`RMFile openFile(const char *fileName)`

- 打开文件名为fileName的文件，并返回一个RMFile实例用于处理该文件

`void closeFile(RMFile &RM)`

- 关闭RMFile实例处理的文件

## RMScanner
- 一个用于扫描单个文件中所有满足某些条件的记录的类，用currentPageID和currentSlotId记录当前扫描到的位置，每次调用nextRec后从当前位置开始往后扫描
- 在调用openScan后会初始化扫描的条件

### 具体接口

#### 外部

`void openScan(RMFile &rmf, AttrType type, int AttrLength, int AttrOffset, ScanType comp, void *value)`

- 初始化扫描条件，被扫描的文件，扫描的信息位于单条记录中的位置和长度，扫描条件，被扫描信息的标准值，并将从文件头开始扫描。

`bool nextRec(RMRecord &rc)`

- 不断调用GetNextRec和vcompare，直到找到下一条满足条件的记录或扫描完所有记录为止

- 如果存在下一条，在rc返回下一个符合条件的记录，并在本地记录下当前扫描到的位置以便下一次调用时从此处开始扫描，如果没有下一个返回条件的记录，直接返回false

`void closeScan()`

- 清空扫描信息(closeScan后若直接调用nextScan会返回false)

#### 内部

`void GetNextRec()`

- 返回当前位置的下一条记录(无论是否满足条件)

`bool vcompare(CharBufType b)`

- 返回bool表示当前记录b是否满足扫描条件