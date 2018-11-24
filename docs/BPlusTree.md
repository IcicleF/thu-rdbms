# BPlusTree

## BPlusNode

* 每页一个节点，内部节点和叶节点的结构相同
* 页的划分
    * 2 Byte：填入的记录数量
    * 2 Byte：节点属性
    * 4 Byte：父结点
    * 2 Byte：父结点的第几个指针
    * 4 Byte：最左指针
    * 之后每条记录 `attrLen + 6 + 4` Byte，存放 attr (`attrLen` Byte)、page + slot (`6` Byte)、孩子节点指针 (`4` Byte)

总的节点数、根节点编号、attrLen 顺序放在 page #0，每个 4 Bytes
root node 放在 page #1