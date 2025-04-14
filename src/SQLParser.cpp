#include "SQLParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace SQLParser {

ParsedQuery SQLParser::parse(const std::string& sql) {
    try {
        std::string cleanSql = removeSemicolon(sql);
        
        // 转换为大写以便于查找关键字
        std::string upperSql = cleanSql;
        std::transform(upperSql.begin(), upperSql.end(), upperSql.begin(), ::toupper);
        
        // 确定SQL语句类型
        if (upperSql.find("SELECT") == 0) {
            return parseSelect(cleanSql);
        } else if (upperSql.find("INSERT") == 0) {
            return parseInsert(cleanSql);
        } else if (upperSql.find("UPDATE") == 0) {
            return parseUpdate(cleanSql);
        } else if (upperSql.find("DELETE") == 0) {
            return parseDelete(cleanSql);
        } else if (upperSql.find("CREATE") == 0) {
            return parseCreate(cleanSql);
        } else if (upperSql.find("DROP") == 0) {
            return parseDrop(cleanSql);
        } else {
            throw std::runtime_error("不支持的SQL语句类型");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("SQL解析失败: " + std::string(e.what()));
    }
}

ParsedQuery SQLParser::parseSelect(const std::string& sql) {
    std::string cleanSql = removeSemicolon(sql);
    
    ParsedQuery query;
    query.type = "SELECT";
    
    try {
        // 解析列名部分
        size_t fromPos = findKeyword(cleanSql, "FROM");
        if (fromPos == std::string::npos) {
            throw std::runtime_error("缺少FROM子句");
        }
        
        std::string columnsStr = cleanSql.substr(6, fromPos - 6);
        columnsStr = trim(columnsStr);
        
        // 特殊处理 SELECT *
        if (columnsStr == "*") {
            Column col;
            col.name = "*";
            query.columns.push_back(col);
        } else {
            // 原有的列解析逻辑
            std::vector<std::string> columnParts;
            
            // 分割列名
            size_t start = 0;
            size_t end = 0;
            int parenthesesCount = 0;
            
            for (size_t i = 0; i < columnsStr.length(); i++) {
                if (columnsStr[i] == '(') {
                    parenthesesCount++;
                } else if (columnsStr[i] == ')') {
                    parenthesesCount--;
                } else if (columnsStr[i] == ',' && parenthesesCount == 0) {
                    columnParts.push_back(trim(columnsStr.substr(start, i - start)));
                    start = i + 1;
                }
            }
            columnParts.push_back(trim(columnsStr.substr(start)));
            
            // 解析每个列定义
            for (const auto& colPart : columnParts) {
                Column col;
                
                // 检查是否有聚合函数
                size_t funcStart = colPart.find('(');
                if (funcStart != std::string::npos) {
                    std::string funcName = trim(colPart.substr(0, funcStart));
                    
                    // 设置聚合函数类型
                    if (funcName == "COUNT") col.aggregateFunc = AggregateFunction::COUNT;
                    else if (funcName == "AVG") col.aggregateFunc = AggregateFunction::AVG;
                    else if (funcName == "SUM") col.aggregateFunc = AggregateFunction::SUM;
                    else if (funcName == "MIN") col.aggregateFunc = AggregateFunction::MIN;
                    else if (funcName == "MAX") col.aggregateFunc = AggregateFunction::MAX;
                    
                    // 提取列名（包括可能的表别名）
                    size_t funcEnd = colPart.find(')', funcStart);
                    if (funcEnd == std::string::npos) {
                        throw std::runtime_error("聚合函数缺少右括号");
                    }
                    
                    std::string fullColName = trim(colPart.substr(funcStart + 1, funcEnd - funcStart - 1));
                    // 处理表别名
                    size_t dotPos = fullColName.find('.');
                    if (dotPos != std::string::npos) {
                        col.tableAlias = fullColName.substr(0, dotPos);
                        col.name = fullColName.substr(dotPos + 1);
                    } else {
                        col.name = fullColName;
                    }
                    
                    // 检查是否有别名
                    size_t asPos = colPart.find(" AS ", funcEnd);
                    if (asPos != std::string::npos) {
                        col.alias = trim(colPart.substr(asPos + 4));
                    } else {
                        // 如果没有显式的别名，使用整个表达式作为别名
                        col.alias = trim(colPart);
                    }
                } else {
                    // 处理普通列
                    size_t asPos = colPart.find(" AS ");
                    if (asPos != std::string::npos) {
                        std::string fullColName = trim(colPart.substr(0, asPos));
                        // 处理表别名
                        size_t dotPos = fullColName.find('.');
                        if (dotPos != std::string::npos) {
                            col.tableAlias = fullColName.substr(0, dotPos);
                            col.name = fullColName.substr(dotPos + 1);
                        } else {
                            col.name = fullColName;
                        }
                        col.alias = trim(colPart.substr(asPos + 4));
                    } else {
                        std::string fullColName = trim(colPart);
                        // 处理表别名
                        size_t dotPos = fullColName.find('.');
                        if (dotPos != std::string::npos) {
                            col.tableAlias = fullColName.substr(0, dotPos);
                            col.name = fullColName.substr(dotPos + 1);
                        } else {
                            col.name = fullColName;
                        }
                    }
                }
                
                query.columns.push_back(col);
            }
        }
        
        // 解析FROM子句
        size_t joinPos = findKeyword(cleanSql, "JOIN", fromPos);
        size_t wherePos = findKeyword(cleanSql, "WHERE", fromPos);
        
        // 解析主表和别名
        std::string fromStr = cleanSql.substr(fromPos + 4, 
            (joinPos != std::string::npos ? joinPos : wherePos) - fromPos - 4);
        std::istringstream fromIss(trim(fromStr));
        fromIss >> query.tableName;
        fromIss >> query.tableAlias;  // 可能为空
        
        // 处理 JOIN
        if (joinPos != std::string::npos) {
            std::string joinStr = cleanSql.substr(joinPos);
            std::istringstream joinIss(joinStr);
            std::string token;
            
            // 跳过 JOIN 关键字
            joinIss >> token;  // "JOIN"
            
            // 检查是否有 JOIN 类型
            size_t joinTypePos = joinPos - 6;  // 检查 JOIN 前面的单词
            std::string joinTypeStr = cleanSql.substr(joinTypePos, 5);
            
            // 初始化连接表和条件的向量
            query.joinTables.clear();
            query.joinConditions.clear();
            
            // 读取连接表名和别名
            std::string joinTable, joinTableAlias;
            joinIss >> joinTable;
            
            // 检查是否有别名
            joinIss >> token;
            if (token != "ON") {
                joinTableAlias = token;
                joinIss >> token;  // 读取 "ON"
            }
            
            // 读取连接条件
            std::string onStr;
            std::getline(joinIss, onStr);
            onStr = trim(onStr);
            
            // 添加到连接表和条件列表
            query.joinTables.push_back(joinTable);
            query.joinConditions.push_back(onStr);
        }
        
        // 解析WHERE条件
        if (wherePos != std::string::npos) {
            query.whereClause = trim(cleanSql.substr(wherePos + 5));
        }
        
        // 解析 ORDER BY
        size_t orderByPos = findKeyword(cleanSql, "ORDER BY", fromPos);
        if (orderByPos != std::string::npos) {
            std::string orderByClause = cleanSql.substr(orderByPos + 8);
            // 检查是否有 DESC 关键字
            size_t descPos = findKeyword(orderByClause, "DESC");
            if (descPos != std::string::npos) {
                query.orderDesc = true;
                orderByClause = orderByClause.substr(0, descPos);
            }
            query.orderByColumn = trim(orderByClause);
        }
        
        // 处理 GROUP BY
        size_t groupByPos = findKeyword(cleanSql, "GROUP BY", wherePos != std::string::npos ? wherePos : fromPos);
        if (groupByPos != std::string::npos) {
            size_t havingPos = findKeyword(cleanSql, "HAVING", groupByPos);
            size_t endPos = havingPos != std::string::npos ? havingPos : cleanSql.length();
            
            std::string groupByStr = cleanSql.substr(groupByPos + 8, endPos - (groupByPos + 8));
            std::istringstream groupByIss(trim(groupByStr));
            std::string groupCol;
            std::string fullGroupCol;
            
            while (std::getline(groupByIss, fullGroupCol, ',')) {
                fullGroupCol = trim(fullGroupCol);
                // 处理表别名
                size_t dotPos = fullGroupCol.find('.');
                if (dotPos != std::string::npos) {
                    // 只保存列名部分，去掉表别名
                    groupCol = trim(fullGroupCol.substr(dotPos + 1));
                } else {
                    groupCol = fullGroupCol;
                }
                query.groupByColumns.push_back(groupCol);
            }
            
            // 处理 HAVING
            if (havingPos != std::string::npos) {
                size_t orderByPos = findKeyword(cleanSql, "ORDER BY", havingPos);
                endPos = orderByPos != std::string::npos ? orderByPos : cleanSql.length();
                
                std::string havingStr = cleanSql.substr(havingPos + 6, endPos - (havingPos + 6));
                // 处理 HAVING 子句中的表别名
                size_t havingDotPos = havingStr.find('.');
                if (havingDotPos != std::string::npos) {
                    std::string beforeDot = havingStr.substr(0, havingDotPos);
                    std::string afterDot = havingStr.substr(havingDotPos + 1);
                    // 如果聚合函数在表别名之前
                    size_t funcPos = beforeDot.find("AVG(");
                    if (funcPos != std::string::npos) {
                        query.havingClause = beforeDot + afterDot;
                    } else {
                        // 去掉表别名
                        query.havingClause = trim(afterDot);
                    }
                } else {
                    query.havingClause = trim(havingStr);
                }
            }
        }
        
        return query;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析 SELECT 语句失败: " + std::string(e.what()));
    }
}

ParsedQuery SQLParser::parseCreate(const std::string& sql) {
    // 去除末尾的分号和空白字符
    std::string cleanSql = sql;
    while (!cleanSql.empty() && (cleanSql.back() == ';' || std::isspace(cleanSql.back()))) {
        cleanSql.pop_back();
    }
    
    ParsedQuery query;
    query.type = "CREATE";
    
    try {
        // 跳过"CREATE TABLE"
        size_t pos = cleanSql.find("(");
        if (pos == std::string::npos) {
            throw std::runtime_error("缺少列定义");
        }
        
        // 提取表名
        std::string tablePart = cleanSql.substr(12, pos - 12);
        query.tableName = trim(tablePart);
        
        // 解析列定义
        std::string colsPart = cleanSql.substr(pos + 1, cleanSql.find_last_of(")") - pos - 1);
        std::vector<std::string> definitions;
        std::string currentDef;
        int parenthesesCount = 0;
        
        // 分割列定义，考虑括号嵌套
        for (char c : colsPart) {
            if (c == '(') parenthesesCount++;
            else if (c == ')') parenthesesCount--;
            else if (c == ',' && parenthesesCount == 0) {
                definitions.push_back(trim(currentDef));
                currentDef.clear();
                continue;
            }
            currentDef += c;
        }
        if (!currentDef.empty()) {
            definitions.push_back(trim(currentDef));
        }
        
        // 处理每个定义
        for (const auto& def : definitions) {
            if (def.find("FOREIGN KEY") != std::string::npos) {
                // 解析外键约束
                size_t fkStart = def.find("(");
                size_t fkEnd = def.find(")");
                size_t refStart = def.find("REFERENCES");
                if (fkStart == std::string::npos || fkEnd == std::string::npos || 
                    refStart == std::string::npos) {
                    throw std::runtime_error("外键语法错误");
                }
                
                // 获取外键列名
                std::string fkColumn = trim(def.substr(fkStart + 1, fkEnd - fkStart - 1));
                
                // 获取引用表和列
                std::string refPart = def.substr(refStart + 10);
                size_t refColStart = refPart.find("(");
                size_t refColEnd = refPart.find(")");
                if (refColStart == std::string::npos || refColEnd == std::string::npos) {
                    throw std::runtime_error("外键引用语法错误");
                }
                
                std::string refTable = trim(refPart.substr(0, refColStart));
                std::string refColumn = trim(refPart.substr(refColStart + 1, 
                                          refColEnd - refColStart - 1));
                
                // 更新相应列的外键信息
                for (auto& col : query.columns) {
                    if (col.name == fkColumn) {
                        col.isForeignKey = true;
                        col.referenceTable = refTable;
                        col.referenceColumn = refColumn;
                        break;
                    }
                }
            } else {
                // 解析普通列定义
                std::istringstream iss(def);
                std::string name, type, constraint;
                iss >> name >> type;
                
                Column col;
                col.name = name;
                col.type = type;
                col.nullable = true;
                
                // 解析约束
                while (iss >> constraint) {
                    if (constraint == "PRIMARY") {
                        std::string key;
                        iss >> key;  // 读取"KEY"
                        col.primaryKey = true;
                    } else if (constraint == "NOT") {
                        std::string null;
                        iss >> null;  // 读取"NULL"
                        col.nullable = false;
                    }
                }
                
                query.columns.push_back(col);
            }
        }
        
        return query;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析CREATE TABLE语句失败: " + std::string(e.what()));
    }
}

ParsedQuery SQLParser::parseInsert(const std::string& sql) {
    // 去除末尾的分号和空白字符
    std::string cleanSql = sql;
    while (!cleanSql.empty() && (cleanSql.back() == ';' || std::isspace(cleanSql.back()))) {
        cleanSql.pop_back();
    }
    
    ParsedQuery query;
    query.type = "INSERT";
    
    try {
        // 跳过"INSERT INTO"
        size_t tableStart = cleanSql.find("INTO") + 4;
        size_t valuesStart = cleanSql.find("VALUES", tableStart);
        
        if (valuesStart == std::string::npos) {
            throw std::runtime_error("缺少VALUES子句");
        }
        
        // 提取表名
        std::string tablePart = cleanSql.substr(tableStart, valuesStart - tableStart);
        size_t leftParen = tablePart.find('(');
        if (leftParen != std::string::npos) {
            query.tableName = trim(tablePart.substr(0, leftParen));
        } else {
            query.tableName = trim(tablePart);
        }
        
        // 提取值
        size_t leftParenValues = cleanSql.find('(', valuesStart);
        size_t rightParenValues = cleanSql.find(')', leftParenValues);
        if (leftParenValues == std::string::npos || rightParenValues == std::string::npos) {
            throw std::runtime_error("VALUES语法错误");
        }
        
        std::string valuesStr = cleanSql.substr(leftParenValues + 1, rightParenValues - leftParenValues - 1);
        std::istringstream iss(valuesStr);
        std::string value;
        while (std::getline(iss, value, ',')) {
            query.values.push_back(trim(value));
        }
        
        return query;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析INSERT语句失败: " + std::string(e.what()));
    }
}

ParsedQuery SQLParser::parseUpdate(const std::string& sql) {
    ParsedQuery query;
    query.type = "UPDATE";
    
    try {
        std::string cleanSql = removeSemicolon(sql);
        
        // 解析表名
        size_t setPos = findKeyword(cleanSql, "SET");
        if (setPos == std::string::npos) {
            throw std::runtime_error("缺少SET子句");
        }
        
        // 获取表名
        std::string tablePart = cleanSql.substr(6, setPos - 6);  // 跳过"UPDATE"
        query.tableName = trim(tablePart);
        
        // 解析SET部分
        size_t wherePos = findKeyword(cleanSql, "WHERE", setPos);
        std::string setPart;
        if (wherePos != std::string::npos) {
            setPart = cleanSql.substr(setPos + 3, wherePos - (setPos + 3));
            query.whereClause = trim(cleanSql.substr(wherePos + 5));
        } else {
            setPart = cleanSql.substr(setPos + 3);
        }
        
        // 解析更新的列和值
        std::istringstream setStream(setPart);
        std::string assignment;
        while (std::getline(setStream, assignment, ',')) {
            assignment = trim(assignment);
            size_t equalPos = assignment.find('=');
            if (equalPos == std::string::npos) {
                throw std::runtime_error("无效的SET子句");
            }
            
            std::string colName = trim(assignment.substr(0, equalPos));
            std::string value = trim(assignment.substr(equalPos + 1));
            
            // 如果值是字符串，去掉引号
            if (value.front() == '\'' && value.back() == '\'') {
                value = value.substr(1, value.length() - 2);
            }
            
            query.updateColumns.push_back(colName);
            query.updateValues.push_back(value);
        }
        
        return query;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析UPDATE语句失败: " + std::string(e.what()));
    }
}

ParsedQuery SQLParser::parseDelete(const std::string& sql) {
    ParsedQuery query;
    query.type = "DELETE";
    
    try {
        std::string cleanSql = removeSemicolon(sql);
        
        // 解析表名
        size_t fromPos = findKeyword(cleanSql, "FROM");
        if (fromPos == std::string::npos) {
            throw std::runtime_error("缺少FROM子句");
        }
        
        size_t wherePos = findKeyword(cleanSql, "WHERE", fromPos);
        
        // 获取表名
        std::string tablePart;
        if (wherePos != std::string::npos) {
            tablePart = cleanSql.substr(fromPos + 4, wherePos - (fromPos + 4));
            query.whereClause = trim(cleanSql.substr(wherePos + 5));
        } else {
            tablePart = cleanSql.substr(fromPos + 4);
        }
        query.tableName = trim(tablePart);
        
        return query;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析DELETE语句失败: " + std::string(e.what()));
    }
}

ParsedQuery SQLParser::parseDrop(const std::string& sql) {
    // 去除末尾的分号和空白字符
    std::string cleanSql = sql;
    while (!cleanSql.empty() && (cleanSql.back() == ';' || std::isspace(cleanSql.back()))) {
        cleanSql.pop_back();
    }
    
    ParsedQuery query;
    query.type = "DROP";
    
    try {
        // 跳过"DROP TABLE"
        std::string tablePart = cleanSql.substr(10);
        query.tableName = trim(tablePart);
        
        if (query.tableName.empty()) {
            throw std::runtime_error("未指定表名");
        }
        
        return query;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析DROP TABLE语句失败: " + std::string(e.what()));
    }
}

std::vector<Column> SQLParser::parseColumns(const std::string& columnsStr) {
    std::vector<Column> columns;
    std::istringstream iss(columnsStr);
    std::string col;
    
    while (std::getline(iss, col, ',')) {
        Column column;
        column.name = trim(col);
        columns.push_back(column);
    }
    
    return columns;
}

} // namespace SQLParser 