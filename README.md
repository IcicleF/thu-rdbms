# THU-RDBMS

一个单用户的、基于 C++ 的数据库系统，是清华大学计算机系大三上课程**数据库系统概论**的大作业。

## 应该实现的功能

### DDL

* CREATE DATABASE
* CREATE TABLE

#### CREATE TABLE 数据类型

* INT
* FLOAT
* CHAR
* DECIMAL (可选)
* VARCHAR (可选)
* DATE (可选)

#### CREATE TABLE 约束

* PRIMARY KEY
  * 实体完整性约束
* FOREIGN KEY (部分)
  * 参照完整性约束
* NOT NULL
* CHECK (部分)
  * 域完整性约束

### DML

* INSERT
* SELECT
* DELETE
* UPDATE

注意这部分实际上有诸多功能需要实现，例如：

* SELECT 嵌套
* DISTINCT
* ORDER BY
* GROUP BY
* 聚集函数

(待扩充)

### 其他

应当至少支持下列来自 MySQL 的语句：

* USE
* SHOW (DATABASES 或 TABLES)
* DESCRIBE
