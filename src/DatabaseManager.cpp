#include "DatabaseManager.h"
#include "Table.h"
#include "SQLParser.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <stdexcept>

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

DatabaseManager::DatabaseManager(const std::string& path) : dbPath(path) {
    // 确保数据目录存在
    try {
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }
        // 初始化时不需要loadFromFile，因为还没有选择数据库
    } catch (const std::exception& e) {
        throw std::runtime_error("无法创建数据库目录: " + std::string(e.what()));
    }
}

bool DatabaseManager::createDatabase(const std::string& dbName) {
    try {
        std::filesystem::path dbDir = dbPath + "/" + dbName;
        if (std::filesystem::exists(dbDir)) {
            return false;
        }
        
        bool success = std::filesystem::create_directories(dbDir);
        if (success) {
            // 创建成功后自动使用该数据库
            currentDatabase = dbName;
            tables.clear();  // 清除旧表
        }
        return success;
    } catch (const std::exception& e) {
        throw std::runtime_error("创建数据库失败: " + std::string(e.what()));
    }
}

bool DatabaseManager::dropDatabase(const std::string& dbName) {
    try {
        std::filesystem::path dbDir = dbPath + "/" + dbName;
        return std::filesystem::remove_all(dbDir) > 0;
    } catch (...) {
        return false;
    }
}

bool DatabaseManager::useDatabase(const std::string& dbName) {
    try {
        std::filesystem::path dbPath = this->dbPath + "/" + dbName;
        if (!std::filesystem::exists(dbPath)) {
            return false;
        }
        
        currentDatabase = dbName;
        return loadFromFile();
    } catch (const std::exception& e) {
        throw std::runtime_error("切换数据库失败: " + std::string(e.what()));
    }
}

bool DatabaseManager::createTable(const std::string& tableName, 
                                const std::vector<ColumnDef>& columns) {
    try {
        // 检查数据库是否已选择
        if (currentDatabase.empty()) {
            throw std::runtime_error("未选择数据库");
        }

        // 检查表名是否已存在
        if (tables.find(tableName) != tables.end()) {
            throw std::runtime_error("表已存在: " + tableName);
        }

        // 验证列定义
        if (columns.empty()) {
            throw std::runtime_error("表必须至少包含一列");
        }

        // 创建表
        tables.emplace(tableName, Table(tableName, columns));

        // 保存到文件
        return saveToFile();
    } catch (const std::exception& e) {
        throw std::runtime_error("创建表失败: " + std::string(e.what()));
    }
}

bool DatabaseManager::dropTable(const std::string& tableName) {
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        return false;
    }
    
    tables.erase(it);
    return saveToFile();
}

bool DatabaseManager::insertInto(const std::string& tableName, 
                               const std::vector<std::string>& values) {
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        return false;
    }
    
    bool success = it->second.insertRow(values);
    if (success) {
        return saveToFile();
    }
    return false;
}

std::vector<std::vector<std::string>> DatabaseManager::select(
    const std::string& tableName,
    const std::vector<std::string>& columns,
    const std::string& whereClause) {
    
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        return std::vector<std::vector<std::string>>();
    }
    
    return it->second.select(columns, whereClause);
}

bool DatabaseManager::saveToFile() {
    try {
        if (currentDatabase.empty()) {
            return false;
        }

        std::filesystem::path dbDir = dbPath + "/" + currentDatabase;
        
        // 遍历所有表并保存
        for (const auto& [tableName, table] : tables) {
            std::filesystem::path tablePath = dbDir / (tableName + ".txt");
            std::ofstream file(tablePath);
            if (!file) {
                throw std::runtime_error("无法创建表文件: " + tablePath.string());
            }
            
            // 保存列定义
            const auto& columns = table.getColumns();
            bool first = true;
            for (const auto& col : columns) {
                if (!first) file << ",";
                file << col.name << ":" 
                     << col.type << ":" 
                     << (col.nullable ? "1" : "0") << ":" 
                     << (col.primaryKey ? "1" : "0");
                first = false;
            }
            file << "\n";
            
            // 保存数据
            const auto& data = table.getData();
            for (const auto& row : data) {
                first = true;
                for (const auto& value : row) {
                    if (!first) file << ",";
                    // 处理特殊字符
                    std::string escapedValue = value;
                    size_t pos = 0;
                    while ((pos = escapedValue.find(',', pos)) != std::string::npos) {
                        escapedValue.replace(pos, 1, "\\,");
                        pos += 2;
                    }
                    file << escapedValue;
                    first = false;
                }
                file << "\n";
            }
        }
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("保存数据库失败: " + std::string(e.what()));
    }
}

bool DatabaseManager::loadFromFile() {
    if (currentDatabase.empty()) {
        return false;
    }
    
    try {
        tables.clear();
        std::filesystem::path dbDir = dbPath + "/" + currentDatabase;
        
        // 检查数据库目录是否存在
        if (!std::filesystem::exists(dbDir)) {
            return false;
        }
        
        // 遍历数据库目录下的所有.txt文件
        for (const auto& entry : std::filesystem::directory_iterator(dbDir)) {
            if (entry.path().extension() == ".txt") {
                std::string tableName = entry.path().stem().string();
                std::ifstream file(entry.path());
                if (!file) {
                    continue;
                }
                
                // 读取列定义
                std::string line;
                if (!std::getline(file, line)) {
                    continue;
                }
                
                std::vector<ColumnDef> columns;
                std::istringstream iss(line);
                std::string colDef;
                while (std::getline(iss, colDef, ',')) {
                    std::istringstream colIss(colDef);
                    std::string name, type, nullable, primaryKey;
                    std::getline(colIss, name, ':');
                    std::getline(colIss, type, ':');
                    std::getline(colIss, nullable, ':');
                    std::getline(colIss, primaryKey);
                    
                    columns.push_back({
                        name,
                        type,
                        nullable == "1",
                        primaryKey == "1"
                    });
                }
                
                // 创建表并插入到map中
                Table table(tableName, columns);
                
                // 读取数据
                while (std::getline(file, line)) {
                    std::vector<std::string> values;
                    std::istringstream rowIss(line);
                    std::string value;
                    while (std::getline(rowIss, value, ',')) {
                        values.push_back(value);
                    }
                    table.insertRow(values);
                }
                
                tables.emplace(tableName, std::move(table));
            }
        }
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("加载数据库失败: " + std::string(e.what()));
    }
}

void DatabaseManager::setDbPath(const std::string& path) {
    dbPath = path;
    
    try {
        // 确保数据目录存在
        if (!std::filesystem::exists(dbPath)) {
            std::filesystem::create_directories(dbPath);
        }
        
        // 如果当前有选中的数据库，加载它
        if (!currentDatabase.empty()) {
            loadFromFile();
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("设置数据库路径失败: " + std::string(e.what()));
    }
}

std::vector<std::string> DatabaseManager::getTableList() {
    std::vector<std::string> tableNames;
    for (const auto& [name, _] : tables) {
        tableNames.push_back(name);
    }
    return tableNames;
}

std::vector<std::vector<std::string>> DatabaseManager::executeSelect(
    const SQLParser::ParsedQuery& query) {
    
    try {
        // 检查是否是多表查询
        if (!query.joinTables.empty() || query.tableName.find(',') != std::string::npos) {
            return executeMultiTableSelect(query);
        }
        
        // 单表查询的原有逻辑
        const Table& table = getTable(query.tableName);
        
        // 检查是否有聚合函数或分组
        bool hasAggregates = false;
        for (const auto& col : query.columns) {
            if (col.aggregateFunc != SQLParser::AggregateFunction::NONE) {
                hasAggregates = true;
                break;
            }
        }
        
        if (hasAggregates || !query.groupByColumns.empty()) {
            return table.selectWithAggregates(
                query.columns,
                query.whereClause,
                query.groupByColumns,
                query.havingClause
            );
        }
        
        // 普通查询
        std::vector<std::string> columnNames;
        for (const auto& col : query.columns) {
            columnNames.push_back(col.name);
        }
        
        return table.select(
            columnNames,
            query.whereClause,
            query.orderByColumn,
            query.orderDesc
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("查询执行失败: " + std::string(e.what()));
    }
}

// 添加新方法处理多表查询
std::vector<std::vector<std::string>> DatabaseManager::executeMultiTableSelect(
    const SQLParser::ParsedQuery& query) {
    
    try {
        // 解析所有表名
        std::vector<std::string> tableNames;
        std::vector<std::string> tableAliases;
        
        // 处理主表
        size_t pos = 0;
        std::string tables = query.tableName;
        while (pos < tables.length()) {
            size_t commaPos = tables.find(',', pos);
            if (commaPos == std::string::npos) commaPos = tables.length();
            
            std::string tableDef = trim(tables.substr(pos, commaPos - pos));
            std::istringstream iss(tableDef);
            std::string tableName, alias;
            iss >> tableName >> alias;
            
            tableNames.push_back(tableName);
            tableAliases.push_back(alias.empty() ? tableName : alias);
            
            pos = commaPos + 1;
        }
        
        // 添加 JOIN 的表
        for (const auto& joinTable : query.joinTables) {
            tableNames.push_back(joinTable);
            tableAliases.push_back(joinTable); // 可以根据需要处理别名
        }
        
        // 获取所有表的数据
        std::vector<const Table*> tables_ptrs;
        for (const auto& name : tableNames) {
            tables_ptrs.push_back(&getTable(name));
        }
        
        // 生成笛卡尔积
        std::vector<std::vector<std::string>> result = generateCartesianProduct(tables_ptrs);
        
        // 应用 WHERE 条件
        std::vector<std::vector<std::string>> filtered;
        for (const auto& row : result) {
            if (evaluateJoinCondition(row, query.whereClause, tableNames, tableAliases)) {
                filtered.push_back(row);
            }
        }
        
        // 选择需要的列
        std::vector<std::vector<std::string>> finalResult;
        for (const auto& row : filtered) {
            std::vector<std::string> selectedRow;
            for (const auto& col : query.columns) {
                // 处理表别名
                std::string tableName;
                std::string colName = col.name;
                
                if (!col.tableAlias.empty()) {
                    // 找到对应的实际表名
                    auto it = std::find(tableAliases.begin(), tableAliases.end(), col.tableAlias);
                    if (it != tableAliases.end()) {
                        tableName = tableNames[it - tableAliases.begin()];
                    }
                }
                
                // 在对应表中找到列的位置
                size_t tableOffset = 0;
                size_t colIndex = 0;
                bool found = false;
                
                for (size_t i = 0; i < tables_ptrs.size(); i++) {
                    if (tableName.empty() || tableNames[i] == tableName) {
                        try {
                            colIndex = tables_ptrs[i]->getColumnIndex(colName);
                            found = true;
                            colIndex += tableOffset;
                            break;
                        } catch (...) {
                            // 继续查找下一个表
                        }
                    }
                    tableOffset += tables_ptrs[i]->getColumns().size();
                }
                
                if (!found) {
                    throw std::runtime_error("列不存在: " + col.name);
                }
                
                selectedRow.push_back(row[colIndex]);
            }
            finalResult.push_back(selectedRow);
        }
        
        return finalResult;
    } catch (const std::exception& e) {
        throw std::runtime_error("多表查询执行失败: " + std::string(e.what()));
    }
}

// 辅助方法：生成笛卡尔积
std::vector<std::vector<std::string>> DatabaseManager::generateCartesianProduct(
    const std::vector<const Table*>& tables) {
    
    std::vector<std::vector<std::string>> result;
    result.push_back(std::vector<std::string>());
    
    for (const auto& table : tables) {
        std::vector<std::vector<std::string>> newResult;
        for (const auto& existingRow : result) {
            for (const auto& tableRow : table->getData()) {
                std::vector<std::string> combinedRow = existingRow;
                combinedRow.insert(combinedRow.end(), tableRow.begin(), tableRow.end());
                newResult.push_back(combinedRow);
            }
        }
        result = std::move(newResult);
    }
    
    return result;
}

bool DatabaseManager::executeNonQuery(const SQLParser::ParsedQuery& query) {
    try {
        // 检查数据库是否已选择
        if (currentDatabase.empty()) {
            throw std::runtime_error("未选择数据库");
        }

        bool success = false;
        if (query.type == "CREATE") {
            std::vector<ColumnDef> columns;
            for (const auto& col : query.columns) {
                ColumnDef colDef;
                colDef.name = col.name;
                colDef.type = col.type;
                colDef.nullable = col.nullable;
                colDef.primaryKey = col.primaryKey;
                columns.push_back(colDef);
            }
            success = createTable(query.tableName, columns);
        } else if (query.type == "INSERT") {
            success = insertIntoTable(query.tableName, 
                                    std::vector<std::string>(),
                                    query.values);
        } else if (query.type == "UPDATE") {
            auto it = tables.find(query.tableName);
            if (it == tables.end()) {
                throw std::runtime_error("表不存在: " + query.tableName);
            }
            
            // 如果有更新列和值，执行更新操作
            if (!query.updateColumns.empty()) {
                success = it->second.updateRows(
                    query.updateColumns,
                    query.updateValues,
                    query.whereClause
                );
            } else {
                // 如果没有更新列和值，只触发保存
                success = true;
            }
        } else if (query.type == "DELETE") {
            auto it = tables.find(query.tableName);
            if (it == tables.end()) {
                throw std::runtime_error("表不存在: " + query.tableName);
            }
            
            // 执行删除操作
            success = it->second.deleteRows(query.whereClause);
        } else if (query.type == "DROP") {
            // 执行DROP TABLE
            success = dropTable(query.tableName);
            if (!success) {
                throw std::runtime_error("删除表失败: " + query.tableName);
            }
        }

        // 如果操作成功，保存到文件
        if (success) {
            return saveToFile();
        }
        return false;
    } catch (const std::exception& e) {
        throw std::runtime_error("执行SQL失败: " + std::string(e.what()));
    }
}

bool DatabaseManager::insertIntoTable(
    const std::string& tableName,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& values) {
    
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        return false;
    }
    
    bool success = it->second.insertRow(values);
    if (success) {
        return saveToFile();
    }
    return false;
}

const Table& DatabaseManager::getTable(const std::string& tableName) const {
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("表不存在: " + tableName);
    }
    return it->second;
}

std::vector<std::string> DatabaseManager::getDatabaseList() {
    std::vector<std::string> databases;
    try {
        // 确保数据目录存在
        if (!std::filesystem::exists(dbPath)) {
            std::filesystem::create_directories(dbPath);
            return databases;
        }
        
        // 遍历数据目录
        for (const auto& entry : std::filesystem::directory_iterator(dbPath)) {
            if (entry.is_directory()) {
                databases.push_back(entry.path().filename().string());
            }
        }
        
        // 按字母顺序排序
        std::sort(databases.begin(), databases.end());
        
    } catch (const std::exception& e) {
        throw std::runtime_error("获取数据库列表失败: " + std::string(e.what()));
    }
    return databases;
}

std::string DatabaseManager::getCurrentDatabase() const {
    return currentDatabase;
}

bool DatabaseManager::evaluateJoinCondition(
    const std::vector<std::string>& row,
    const std::string& condition,
    const std::vector<std::string>& tableNames,
    const std::vector<std::string>& tableAliases) {
    
    if (condition.empty()) {
        return true;
    }
    
    try {
        // 分割多个条件（用 AND 连接的条件）
        std::vector<std::string> conditions;
        size_t pos = 0;
        while (pos < condition.length()) {
            size_t andPos = condition.find("AND", pos);
            if (andPos == std::string::npos) {
                conditions.push_back(trim(condition.substr(pos)));
                break;
            }
            conditions.push_back(trim(condition.substr(pos, andPos - pos)));
            pos = andPos + 3;  // Skip "AND"
        }
        
        // 评估每个条件
        for (const auto& cond : conditions) {
            // 查找操作符
            std::vector<std::string> operators = {">=", "<=", "!=", "=", ">", "<"};
            std::string op;
            size_t opPos = std::string::npos;
            
            for (const auto& testOp : operators) {
                if ((opPos = cond.find(testOp)) != std::string::npos) {
                    op = testOp;
                    break;
                }
            }
            
            if (opPos == std::string::npos) {
                throw std::runtime_error("无效的条件: " + cond);
            }
            
            // 获取左右值
            std::string leftExpr = trim(cond.substr(0, opPos));
            std::string rightExpr = trim(cond.substr(opPos + op.length()));
            
            // 处理表达式中的表别名
            auto getColumnValue = [&](const std::string& expr) -> std::string {
                // 检查是否是字面值（用引号括起来的）
                if (expr[0] == '\'' || expr[0] == '"') {
                    return expr.substr(1, expr.length() - 2);
                }
                
                // 处理列引用
                std::string tableAlias;
                std::string colName = expr;
                size_t dotPos = expr.find('.');
                if (dotPos != std::string::npos) {
                    tableAlias = expr.substr(0, dotPos);
                    colName = expr.substr(dotPos + 1);
                }
                
                // 找到对应的表
                size_t tableOffset = 0;
                for (size_t i = 0; i < tableAliases.size(); i++) {
                    if (tableAlias.empty() || tableAliases[i] == tableAlias) {
                        // 获取该表的列数
                        const Table& table = getTable(tableNames[i]);
                        try {
                            size_t colIndex = table.getColumnIndex(colName);
                            return row[tableOffset + colIndex];
                        } catch (...) {
                            if (!tableAlias.empty()) {
                                throw;  // 如果指定了表别名但列不存在，则报错
                            }
                        }
                    }
                    const Table& table = getTable(tableNames[i]);
                    tableOffset += table.getColumns().size();
                }
                throw std::runtime_error("列不存在: " + expr);
            };
            
            // 获取实际值
            std::string leftVal = getColumnValue(leftExpr);
            std::string rightVal = getColumnValue(rightExpr);
            
            // 比较值
            bool condResult;
            if (op == "=") condResult = leftVal == rightVal;
            else if (op == "!=") condResult = leftVal != rightVal;
            else if (op == ">") condResult = leftVal > rightVal;
            else if (op == "<") condResult = leftVal < rightVal;
            else if (op == ">=") condResult = leftVal >= rightVal;
            else if (op == "<=") condResult = leftVal <= rightVal;
            else throw std::runtime_error("不支持的操作符: " + op);
            
            if (!condResult) return false;  // 如果任何条件不满足，返回false
        }
        
        return true;  // 所有条件都满足
    } catch (const std::exception& e) {
        throw std::runtime_error("条件评估失败: " + std::string(e.what()));
    }
} 