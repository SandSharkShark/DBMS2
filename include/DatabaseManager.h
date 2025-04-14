#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <map>
#include "Table.h"
#include "SQLParser.h"

class DatabaseManager {
public:
    DatabaseManager(const std::string& path);
    
    // 数据库操作
    bool createDatabase(const std::string& dbName);
    bool dropDatabase(const std::string& dbName);
    bool useDatabase(const std::string& dbName);
    
    // 表操作
    bool createTable(const std::string& tableName, const std::vector<ColumnDef>& columns);
    bool dropTable(const std::string& tableName);
    bool insertInto(const std::string& tableName, const std::vector<std::string>& values);
    std::vector<std::vector<std::string>> select(const std::string& tableName, 
                                                const std::vector<std::string>& columns,
                                                const std::string& whereClause = "");
    
    const Table& getTable(const std::string& tableName) const;
    bool insertIntoTable(const std::string& tableName,
                        const std::vector<std::string>& columns,
                        const std::vector<std::string>& values);
    
    // 查询执行
    std::vector<std::vector<std::string>> executeSelect(const SQLParser::ParsedQuery& query);
    bool executeNonQuery(const SQLParser::ParsedQuery& query);
    
    // 工具方法
    void setDbPath(const std::string& path);
    std::vector<std::string> getTableList();
    std::vector<std::string> getDatabaseList();
    std::string getCurrentDatabase() const;
    
    std::string getDbPath() const { return dbPath; }
    
private:
    std::string dbPath;
    std::string currentDatabase;
    std::map<std::string, Table> tables;
    
    bool loadFromFile();
    bool saveToFile();
    
    std::vector<std::vector<std::string>> executeMultiTableSelect(
        const SQLParser::ParsedQuery& query);
    
    std::vector<std::vector<std::string>> generateCartesianProduct(
        const std::vector<const Table*>& tables);
        
    bool evaluateJoinCondition(
        const std::vector<std::string>& row,
        const std::string& condition,
        const std::vector<std::string>& tableNames,
        const std::vector<std::string>& tableAliases);
};

#endif 