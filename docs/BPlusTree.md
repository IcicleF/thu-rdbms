# BPlusTree

## BPlusNode

* 每页一个节点，内部节点和叶节点的结构相同
* 页的划分
    * 2 Byte：填入的记录数量
    * 2 Byte：节点属性
    * 4 Byte：父结点
    * 2 Byte：父结点的第几个指针
    * 6 Byte：最左指针域
    * 之后每条记录 `attrLen + 6` Byte，存放 attr (`attrLen` Byte) 和一个指针域

attrLen、总的节点数、根节点编号 顺序放在 page #0，每个 4 Bytes

root node 放在 page #1

指针域 叶节点存一个 page 和一个 slot，内部节点存一个 page

### methods

* count / setCount：取得和设置节点中的记录数
* type / setType：取得和设置节点的类型（1 = 内部，2 = 叶子）
* parent / setParent：取得和设置父结点的页号
* parentPtr / setParentPtr：取得和设置父结点是第几个指针指向自己
* val / setVal：取得和设置某个记录的 attr （void*）
* child / setChild：取得和设置某个指向孩子节点的指针，应该只在内部节点和叶子的最后一个指针的地方用
* rec / setRec：取得和设置某个指向记录的 RID，应该只在叶子节点用