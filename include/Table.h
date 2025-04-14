#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include "forward_declarations.h"
#include "SQLParser.h"

// 前向声明
enum class JoinType {
    NONE,
    INNER,
    LEFT,
    RIGHT,
    FULL
};

// 条件结构
struct Condition {
    std::string column;
    std::string operation;  // =, >, <, >=, <=, !=, LIKE
    std::string value;
};

class Table {
public:
    Table() = default;
    Table(const std::string& tableName, const std::vector<ColumnDef>& cols);
    
    // 基本操作
    bool insertRow(const std::vector<std::string>& values);
    std::vector<std::vector<std::string>> select(
        const std::vector<std::string>& columns,
        const std::string& whereClause = "",
        const std::string& orderByColumn = "",
        bool orderDesc = false) const;
    bool updateRows(
        const std::vector<std::string>& columns,
        const std::vector<std::string>& values,
        const std::string& whereClause = "");
    bool deleteRows(const std::string& whereClause);
    
    // JOIN操作
    std::vector<std::vector<std::string>> join(
        const Table& otherTable,
        const std::string& leftCol,
        const std::string& rightCol,
        JoinType joinType = JoinType::INNER);
    
    // 索引操作
    bool createIndex(const std::string& columnName);
    bool dropIndex(const std::string& columnName);
    
    // Getter方法
    const std::vector<ColumnDef>& getColumns() const { return columns; }
    const std::vector<std::vector<std::string>>& getData() const { return data; }
    const std::string& getName() const { return name; }
    
    // 添加新的查询方法
    std::vector<std::vector<std::string>> selectWithGroupBy(
        const std::vector<std::string>& columns,
        const std::string& whereClause,
        const std::string& groupBy,
        const std::string& having,
        const std::string& orderBy,
        bool orderDesc);
    
    size_t getColumnIndex(const std::string& columnName) const;
    
    // 添加新的查询方法
    std::vector<std::vector<std::string>> selectWithAggregates(
        const std::vector<SQLParser::Column>& columns,
        const std::string& whereClause,
        const std::vector<std::string>& groupByColumns,
        const std::string& havingClause) const;
        
private:
    std::string name;
    std::vector<ColumnDef> columns;
    std::vector<std::vector<std::string>> data;
    std::map<std::string, std::map<std::string, std::set<size_t>>> indices;

    // 辅助方法
    bool validateDataType(const std::string& value, const std::string& type);
    bool evaluateCondition(const std::vector<std::string>& row, const std::string& whereClause) const;
    std::vector<Condition> parseWhereClause(const std::string& whereClause) const;
    bool evaluateSingleCondition(const std::string& value, const Condition& cond) const;
    void updateIndices(size_t rowIndex, const std::vector<std::string>& values);
    void removeFromIndices(size_t rowIndex);
    
    // 添加新的辅助方法
    std::vector<std::vector<std::string>> groupData(
        const std::vector<std::vector<std::string>>& data,
        const std::string& groupBy,
        const std::vector<std::string>& columns);
        
    bool evaluateHavingCondition(
        const std::vector<std::string>& groupValues,
        const std::string& havingClause);
        
    void sortData(std::vector<std::vector<std::string>>& data,
                 const std::string& orderByColumn,
                 bool desc) const;
    
    // 添加比较辅助方法
    static bool compareValues(const std::string& a, const std::string& b,
                            const std::string& type);
    
    // 添加辅助方法
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
    
    // 添加辅助方法
    std::string calculateAggregate(
        const std::vector<std::string>& values,
        SQLParser::AggregateFunction func) const;
        
    bool evaluateHavingClause(
        const std::vector<std::string>& groupValues,
        const std::string& havingClause) const;
    
    // 添加 split 辅助方法
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

#endif 