# 记录管理模块

记录管理模块在文件层面管理记录的

* 检索
* 增加
* 删除
* 更新

## 记录存储格式

每个表存储为一个目录，记录存储在目录下的文件中。

### metadata

`metadata` 中包含表的所有列的信息。

每个列的信息表示为如下的结构体：

```cpp
struct Column {
    enum ColumnType {
        Int,
        Char,
        Varchar,
        Float,
        Date
    };

    string name;
    ColumnType type;
    int width;
    bool notnull;
    string constraint;
};
```

表的所有列表示为如下的结构体：

```cpp
struct Table {
    struct ForeignKey {
        vector<int> key;
        int table;
    };

    string name;
    vector<Column> col;
    vector<int> primary;
    vector<ForeignKey> foreign;
};
```

这些内容以纯文本形式存储。

### table

`table` 中存储记录。

`table` 的访问以页为单位。每页的大小是 8192 字节，因此**页内指针**的大小为 2 字节。后续将会提到指向文件内（而不是页内）位置的指针，这样的**文件内指针**大小为 4 字节。

在这个文件中，每个记录都是定长的。一条记录的长度等于每列所需的长度之和加上一个**页内指针**。每列所需的长度定义如下：

|   类型      |    长度    |
|------------|------------|
| INT        |   4 Byte   |
| CHAR(x)    |   x Byte   |
| VARCHAR(x) |   不定      |
| DATE       |   4 Byte   |
| Float      |   4 Byte   |

VARCHAR(x) 所需的长度定义如下：

* 如果 x <= 32，那么 VARCHAR(x) 固定占用 x 个字节；
* 如果 x > 32，那么 VARCHAR(x) 占用 32 个字节和一个**文件内指针**；这个指针的用途后续说明。

每一页的前 16 字节作为保留区域使用，余下的空间划分为等长的存储槽，最后的无法使用的空间则直接废弃不用。

每个存储槽中的页内指针指向下一个同类型的存储槽，即，被填入的存储槽指向下一个被填入的存储槽，空的存储槽指向下一个空的存储槽。这构成了两个链表，链表头存在页头的保留区域中。

### varcharX

`varcharX` 这样的文件用以存储 VARCHAR 类型。

存储 VARCHAR 的基本思路是倍增法。对于实际长度大于等于 16 的 VARCHAR 类型数据，我们保证它占据的存储空间近似不大于它实际长度的 2 倍。

具体实现是这样的：

* `table` 存储第 1～32 个字符；
* `varchar32` 存储第 33～64 个字符；
* `varchar64` 存储第 65～128 个字符；
* `varchar128` 存储第 129～256 个字符；
* ……

每个 `varcharX` 文件中的存储槽长度等于 X 字节，加上一个**文件内指针**和一个**页内指针**。其中，文件内指针用以指向下一个 `varcharX` 文件中的存储槽，页内指针的用途与 `table` 相同。
