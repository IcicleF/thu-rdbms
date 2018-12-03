# THU-RDBMS

一个单用户的、基于 C++ 的数据库系统，是清华大学计算机系大三上课程**数据库系统概论**的大作业。

## 应该实现的功能

### DDL

* CREATE DATABASE
* CREATE TABLE
* USE
* SHOW DATABASES/SHOW TABLES
* DESC

FOREIGN KEY REFERENCE 到复合的主键上的情况不考虑。

#### CREATE TABLE 数据类型

* INT
* FLOAT
* CHAR
* DATE
* VARCHAR

#### CREATE TABLE 约束

* PRIMARY KEY
  * 实体完整性约束
* FOREIGN KEY (部分)
  * 参照完整性约束
* NOT NULL

### DML

* INSERT
* SELECT
* DELETE
* UPDATE

注意这部分有诸多功能都**不**需要实现，例如：

* SELECT 嵌套：不用实现
* DISTINCT：不用实现
* ORDER BY：不用实现
* GROUP BY：不用实现
* 聚集函数：不用实现

