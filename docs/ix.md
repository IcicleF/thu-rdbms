# Indexing 文档

接口和功能大量地借鉴了 Stanford 课程网站上给出的接口示例。

## IXManager

`IXManager` 负责管理索引文件，它的实现原理和记录管理部分的 `RMManager` 是完全类似的。

Stanford 课程官网中，`createIndex` 和 `openIndex` 支持指定一个索引编号。我们在实现时照搬了这一设定，但之后的流程中，如果存在多个索引，我们怀疑可能会出现某些索引失效的问题。

* 对此，我们现阶段不打算考虑此事。如果问题的确存在，我们会规定一个表只支持一个簇集索引。

我们提供 `createIndex`、`openIndex`、`closeIndex` 三个接口来创建、打开和关闭一个索引。

#