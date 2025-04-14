# 简易数据库管理系统

这是一个基于 C++ 和 Qt 开发的简易数据库管理系统，支持基本的 SQL 操作和用户管理功能。

## 功能特性

### 数据库操作

- 创建和删除数据库
- 创建和删除表
- 基本的 CRUD 操作（增删改查）
- 支持多表联合查询
- 支持聚合函数和分组查询
- 支持条件过滤（WHERE 子句）
- 支持排序（ORDER BY）

### SQL 支持

- CREATE TABLE
- DROP TABLE
- INSERT INTO
- SELECT（支持 *、列选择、表别名）
- UPDATE
- DELETE
- 支持 JOIN 操作
- 支持 GROUP BY 和 HAVING
- 支持聚合函数（COUNT、AVG、SUM 等）

### 用户管理

- 用户注册
- 用户登录
- 权限管理

## 使用示例

### 创建表

*CREATE TABLE Students (ID INTEGER PRIMARY KEY,Name TEXT NOT NULL,Age INTEGER,Department TEXT);*

### 插入数据

*INSERT INTO Students (ID, Name, Age, Department)VALUES (1, '张三', 20, '计算机系');

### 查询数据

-- 简单查询

*SELECT FROM Students;*

-- 条件查询

*SELECT Name, Age FROM Students WHERE Age > 20;*

-- 聚合查询

*SELECT Department, COUNT() as StudentCountFROM StudentsGROUP BY Department;*

-- 多表联合查询

*SELECT s.Name, c.CourseName, sc.Grade FROM Students s*

*JOIN StudentCourses sc ON s.ID = sc.StudentID*

***JOIN Courses c ON sc.CourseID = c.CourseID;***


## 注意事项

1. 所有 SQL 语句都需要以分号结尾
2. 字符串值需要用单引号括起来
3. 表名和列名不能包含特殊字符
4. 主键列不能为空或重复

## 错误处理

系统会对常见错误进行提示：

- 语法错误
- 表不存在
- 列不存在
- 数据类型不匹配
- 违反主键约束
- 违反外键约束

## 技术栈

- C++17
- Qt 5
- 文件系统存储
