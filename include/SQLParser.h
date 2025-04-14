#ifndef SQLPARSER_H
#define SQLPARSER_H

#include <string>
#include <vector>
#include "forward_declarations.h"

namespace SQLParser {

enum class JoinType {
    NONE,
    INNER,
    LEFT,
    RIGHT
};

enum class AggregateFunction {
    NONE,
    COUNT,
    AVG,
    SUM,
    MIN,
    MAX
};

struct Column {
    std::string name;        // 列名
    std::string alias;       // 列别名
    std::string tableAlias;  // 表别名
    std::string type;
    bool nullable = true;
    bool primaryKey = false;
    bool isForeignKey = false;
    std::string referenceTable;
    std::string referenceColumn;
    AggregateFunction aggregateFunc = AggregateFunction::NONE;
    std::string expression;  // 用于存储原始表达式
};

struct ParsedQuery {
    std::string type;
    std::string tableName;
    std::string tableAlias;      // 主表别名
    std::vector<Column> columns;
    std::string whereClause;
    std::vector<std::string> values;
    std::vector<std::string> updateColumns;
    std::vector<std::string> updateValues;
    
    // 分组和排序
    std::vector<std::string> groupByColumns;
    std::string havingClause;
    std::string orderByColumn;
    bool orderDesc = false;
    int limit = -1;
    
    // 连接查询
    std::vector<std::string> joinTables;
    std::vector<std::string> joinConditions;
};

class SQLParser {
public:
    SQLParser() = default;
    ParsedQuery parse(const std::string& sql);
    
private:
    ParsedQuery parseSelect(const std::string& sql);
    ParsedQuery parseCreate(const std::string& sql);
    ParsedQuery parseInsert(const std::string& sql);
    ParsedQuery parseUpdate(const std::string& sql);
    ParsedQuery parseDelete(const std::string& sql);
    ParsedQuery parseDrop(const std::string& sql);
    std::vector<Column> parseColumns(const std::string& columnsStr);
    
    // 辅助方法
    static size_t findKeyword(const std::string& sql, const std::string& keyword, size_t startPos = 0) {
        std::string upperSql = sql;
        std::transform(upperSql.begin(), upperSql.end(), upperSql.begin(), ::toupper);
        std::string upperKeyword = keyword;
        std::transform(upperKeyword.begin(), upperKeyword.end(), upperKeyword.begin(), ::toupper);
        return upperSql.find(upperKeyword, startPos);
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }
    
    // 添加辅助函数来处理分号
    static std::string removeSemicolon(const std::string& sql) {
        std::string cleanSql = sql;
        while (!cleanSql.empty() && (cleanSql.back() == ';' || std::isspace(cleanSql.back()))) {
            cleanSql.pop_back();
        }
        return cleanSql;
    }
};

} // namespace SQLParser

#endif 