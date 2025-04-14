#include "Table.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include "SQLParser.h"

Table::Table(const std::string& tableName, const std::vector<ColumnDef>& cols)
    : name(tableName), columns(cols) {
}

bool Table::insertRow(const std::vector<std::string>& values) {
    try {
        if (values.size() != columns.size()) {
            throw std::runtime_error("列数不匹配");
        }
        
        // 验证数据类型
        for (size_t i = 0; i < values.size(); i++) {
            if (!validateDataType(values[i], columns[i].type)) {
                throw std::runtime_error("数据类型不匹配: " + columns[i].name);
            }
            
            // 检查非空约束
            if (!columns[i].nullable && values[i].empty()) {
                throw std::runtime_error("非空列不能为空: " + columns[i].name);
            }
        }
        
        // 更新索引
        size_t rowIndex = data.size();
        updateIndices(rowIndex, values);
        
        data.push_back(values);
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("插入数据失败: " + std::string(e.what()));
    }
}

std::vector<std::vector<std::string>> Table::select(
    const std::vector<std::string>& columns,
    const std::string& whereClause,
    const std::string& orderByColumn,
    bool orderDesc) const {
    
    try {
        std::vector<std::vector<std::string>> result;
        std::vector<size_t> columnIndices;
        
        // 处理 SELECT *
        if (columns.size() == 1 && columns[0] == "*") {
            // 如果是 SELECT *，返回所有列
            for (size_t i = 0; i < this->columns.size(); i++) {
                columnIndices.push_back(i);
            }
        } else {
            // 获取要查询的列的索引
            for (const auto& col : columns) {
                columnIndices.push_back(getColumnIndex(col));
            }
        }
        
        // 应用WHERE条件筛选数据
        for (const auto& row : data) {
            if (whereClause.empty() || evaluateCondition(row, whereClause)) {
                std::vector<std::string> selectedRow;
                for (size_t idx : columnIndices) {
                    selectedRow.push_back(row[idx]);
                }
                result.push_back(selectedRow);
            }
        }
        
        // 如果指定了排序列，进行排序
        if (!orderByColumn.empty()) {
            sortData(result, orderByColumn, orderDesc);
        }
        
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error("查询失败: " + std::string(e.what()));
    }
}

bool Table::updateRows(
    const std::vector<std::string>& updateColumns,
    const std::vector<std::string>& updateValues,
    const std::string& whereClause) {
    
    try {
        if (updateColumns.size() != updateValues.size()) {
            throw std::runtime_error("更新的列数和值的数量不匹配");
        }
        
        bool anyUpdated = false;
        // 遍历所有行
        for (size_t rowIndex = 0; rowIndex < data.size(); rowIndex++) {
            // 检查WHERE条件
            if (whereClause.empty() || evaluateCondition(data[rowIndex], whereClause)) {
                // 从索引中移除旧值
                removeFromIndices(rowIndex);
                
                // 更新值
                for (size_t i = 0; i < updateColumns.size(); i++) {
                    size_t colIndex;
                    try {
                        colIndex = getColumnIndex(updateColumns[i]);
                    } catch (const std::exception& e) {
                        throw std::runtime_error("更新列不存在: " + updateColumns[i]);
                    }
                    
                    // 验证数据类型
                    if (!validateDataType(updateValues[i], columns[colIndex].type)) {
                        throw std::runtime_error("数据类型不匹配: " + updateColumns[i]);
                    }
                    
                    // 更新值
                    data[rowIndex][colIndex] = updateValues[i];
                }
                
                // 更新索引
                updateIndices(rowIndex, data[rowIndex]);
                anyUpdated = true;
            }
        }
        
        return anyUpdated;
    } catch (const std::exception& e) {
        throw std::runtime_error("更新失败: " + std::string(e.what()));
    }
}

bool Table::deleteRows(const std::string& whereClause) {
    try {
        bool anyDeleted = false;
        
        // 从后向前遍历，这样删除时不会影响未处理的索引
        for (int rowIndex = data.size() - 1; rowIndex >= 0; rowIndex--) {
            if (whereClause.empty() || evaluateCondition(data[rowIndex], whereClause)) {
                // 从索引中移除
                removeFromIndices(rowIndex);
                
                // 从数据中移除
                data.erase(data.begin() + rowIndex);
                anyDeleted = true;
            }
        }
        
        return anyDeleted;
    } catch (const std::exception& e) {
        throw std::runtime_error("删除失败: " + std::string(e.what()));
    }
}

std::vector<std::vector<std::string>> Table::join(
    const Table& otherTable,
    const std::string& leftCol,
    const std::string& rightCol,
    JoinType joinType) {
    
    size_t leftIdx = getColumnIndex(leftCol);
    size_t rightIdx = otherTable.getColumnIndex(rightCol);
    
    std::vector<std::vector<std::string>> result;
    
    // 对于每一行
    for (const auto& leftRow : data) {
        bool matched = false;
        
        // 查找匹配的右表行
        for (const auto& rightRow : otherTable.getData()) {
            if (leftRow[leftIdx] == rightRow[rightIdx]) {
                // 合并行
                std::vector<std::string> joinedRow = leftRow;
                joinedRow.insert(joinedRow.end(), rightRow.begin(), rightRow.end());
                result.push_back(joinedRow);
                matched = true;
            }
        }
        
        // 处理外连接
        if (!matched && (joinType == JoinType::LEFT || joinType == JoinType::FULL)) {
            std::vector<std::string> joinedRow = leftRow;
            joinedRow.insert(joinedRow.end(), otherTable.getColumns().size(), "NULL");
            result.push_back(joinedRow);
        }
    }
    
    return result;
}

bool Table::createIndex(const std::string& columnName) {
    size_t colIndex = getColumnIndex(columnName);
    
    // 清除现有索引
    indices[columnName].clear();
    
    // 创建新索引
    for (size_t i = 0; i < data.size(); i++) {
        indices[columnName][data[i][colIndex]].insert(i);
    }
    
    return true;
}

bool Table::dropIndex(const std::string& columnName) {
    return indices.erase(columnName) > 0;
}

bool Table::validateDataType(const std::string& value, const std::string& type) {
    if (type == "INTEGER") {
        try {
            std::stoi(value);
            return true;
        } catch (...) {
            return false;
        }
    } else if (type == "FLOAT") {
        try {
            std::stof(value);
            return true;
        } catch (...) {
            return false;
        }
    }
    return true;  // STRING类型接受任何值
}

size_t Table::getColumnIndex(const std::string& columnName) const {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == columnName) {
            return i;
        }
    }
    throw std::runtime_error("列不存在: " + columnName);
}

bool Table::evaluateCondition(const std::vector<std::string>& row, const std::string& whereClause) const {
    if (whereClause.empty()) {
        return true;
    }
    
    try {
        // 分割多个条件（用 AND 连接的条件）
        std::vector<std::string> conditions;
        size_t pos = 0;
        while (pos < whereClause.length()) {
            size_t andPos = whereClause.find("AND", pos);
            if (andPos == std::string::npos) {
                conditions.push_back(trim(whereClause.substr(pos)));
                break;
            }
            conditions.push_back(trim(whereClause.substr(pos, andPos - pos)));
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
            
            // 获取列名和值
            std::string colName = trim(cond.substr(0, opPos));
            std::string value = trim(cond.substr(opPos + op.length()));
            
            // 如果值是字符串字面量，去掉引号
            if (value.front() == '\'' && value.back() == '\'') {
                value = value.substr(1, value.length() - 2);
            }
            
            // 获取列值
            size_t colIndex;
            try {
                colIndex = getColumnIndex(colName);
            } catch (const std::exception& e) {
                throw std::runtime_error("条件中的列不存在: " + colName);
            }
            
            std::string rowValue = row[colIndex];
            
            // 比较值
            bool condResult;
            if (op == "=") condResult = rowValue == value;
            else if (op == "!=") condResult = rowValue != value;
            else if (op == ">") condResult = rowValue > value;
            else if (op == "<") condResult = rowValue < value;
            else if (op == ">=") condResult = rowValue >= value;
            else if (op == "<=") condResult = rowValue <= value;
            else throw std::runtime_error("不支持的操作符: " + op);
            
            if (!condResult) return false;  // 如果任何条件不满足，返回false
        }
        
        return true;  // 所有条件都满足
    } catch (const std::exception& e) {
        throw std::runtime_error("条件评估失败: " + std::string(e.what()));
    }
}

bool Table::evaluateSingleCondition(const std::string& value, const Condition& cond) const {
    if (cond.operation == "=") {
        return value == cond.value;
    } else if (cond.operation == ">") {
        return std::stod(value) > std::stod(cond.value);
    } else if (cond.operation == "<") {
        return std::stod(value) < std::stod(cond.value);
    } else if (cond.operation == ">=") {
        return std::stod(value) >= std::stod(cond.value);
    } else if (cond.operation == "<=") {
        return std::stod(value) <= std::stod(cond.value);
    } else if (cond.operation == "!=") {
        return value != cond.value;
    }
    throw std::runtime_error("不支持的操作符: " + cond.operation);
}

void Table::updateIndices(size_t rowIndex, const std::vector<std::string>& values) {
    for (size_t i = 0; i < columns.size(); i++) {
        const std::string& columnName = columns[i].name;
        if (indices.find(columnName) != indices.end()) {
            indices[columnName][values[i]].insert(rowIndex);
        }
    }
}

void Table::removeFromIndices(size_t rowIndex) {
    for (auto& [columnName, columnIndex] : indices) {
        for (auto& [value, rowSet] : columnIndex) {
            rowSet.erase(rowIndex);
        }
    }
}

std::vector<Condition> Table::parseWhereClause(const std::string& whereClause) const {
    std::vector<Condition> conditions;
    if (whereClause.empty()) {
        return conditions;
    }
    
    std::istringstream iss(whereClause);
    std::string token;
    
    // 基本的WHERE子句解析
    // 格式: column operation value [AND column operation value]...
    while (iss >> token) {
        Condition cond;
        
        // 解析列名
        cond.column = token;
        
        // 解析操作符
        if (!(iss >> token)) break;
        cond.operation = token;
        
        // 解析值
        if (!(iss >> token)) break;
        // 处理带引号的字符串
        if (token[0] == '"' || token[0] == '\'') {
            token = token.substr(1, token.length() - 2);
        }
        cond.value = token;
        
        conditions.push_back(cond);
        
        // 检查是否还有AND
        if (!(iss >> token) || token != "AND") {
            break;
        }
    }
    
    return conditions;
}

std::vector<std::vector<std::string>> Table::selectWithGroupBy(
    const std::vector<std::string>& columns,
    const std::string& whereClause,
    const std::string& groupBy,
    const std::string& having,
    const std::string& orderBy,
    bool orderDesc) {
    
    try {
        // 首先执行WHERE过滤
        auto filteredData = select(columns, whereClause);
        
        // 如果有GROUP BY，进行分组
        if (!groupBy.empty()) {
            filteredData = groupData(filteredData, groupBy, columns);
            
            // 应用HAVING子句
            if (!having.empty()) {
                std::vector<std::vector<std::string>> havingFilteredData;
                for (const auto& row : filteredData) {
                    if (evaluateHavingCondition(row, having)) {
                        havingFilteredData.push_back(row);
                    }
                }
                filteredData = std::move(havingFilteredData);
            }
        }
        
        // 如果有ORDER BY，进行排序
        if (!orderBy.empty()) {
            sortData(filteredData, orderBy, orderDesc);
        }
        
        return filteredData;
    } catch (const std::exception& e) {
        throw std::runtime_error("高级查询执行失败: " + std::string(e.what()));
    }
}

std::vector<std::vector<std::string>> Table::groupData(
    const std::vector<std::vector<std::string>>& data,
    const std::string& groupBy,
    const std::vector<std::string>& columns) {
    
    // 获取分组列的索引
    size_t groupByColIndex = getColumnIndex(groupBy);
    
    // 创建分组映射
    std::map<std::string, std::vector<std::vector<std::string>>> groups;
    for (const auto& row : data) {
        groups[row[groupByColIndex]].push_back(row);
    }
    
    // 处理每个分组
    std::vector<std::vector<std::string>> result;
    for (const auto& [key, group] : groups) {
        std::vector<std::string> aggregatedRow;
        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].find("COUNT") != std::string::npos) {
                aggregatedRow.push_back(std::to_string(group.size()));
            } else if (columns[i].find("SUM") != std::string::npos) {
                // 提取列名
                size_t start = columns[i].find("(") + 1;
                size_t end = columns[i].find(")");
                std::string colName = columns[i].substr(start, end - start);
                
                // 计算总和
                double sum = 0;
                size_t colIndex = getColumnIndex(colName);
                for (const auto& row : group) {
                    sum += std::stod(row[colIndex]);
                }
                aggregatedRow.push_back(std::to_string(sum));
            } else if (columns[i].find("AVG") != std::string::npos) {
                // 类似SUM的实现，但需要除以组大小
                // ... 实现AVG聚合函数 ...
            } else {
                // 非聚合列，使用组的第一行的值
                aggregatedRow.push_back(group[0][getColumnIndex(columns[i])]);
            }
        }
        result.push_back(aggregatedRow);
    }
    
    return result;
}

void Table::sortData(std::vector<std::vector<std::string>>& data,
                    const std::string& orderByColumn,
                    bool desc) const {
    try {
        // 获取排序列的索引和类型
        size_t sortColIndex = getColumnIndex(orderByColumn);
        std::string colType = columns[sortColIndex].type;
        
        // 对数据进行排序
        std::sort(data.begin(), data.end(),
            [sortColIndex, colType, desc](const auto& a, const auto& b) {
                if (desc) {
                    return compareValues(b[sortColIndex], a[sortColIndex], colType);
                } else {
                    return compareValues(a[sortColIndex], b[sortColIndex], colType);
                }
            });
    } catch (const std::exception& e) {
        throw std::runtime_error("排序失败: " + std::string(e.what()));
    }
}

bool Table::compareValues(const std::string& a, const std::string& b,
                        const std::string& type) {
    if (a.empty() && b.empty()) return false;
    if (a.empty()) return true;   // 空值排在前面
    if (b.empty()) return false;
    
    try {
        if (type == "INTEGER") {
            return std::stoi(a) < std::stoi(b);
        } else if (type == "FLOAT") {
            return std::stof(a) < std::stof(b);
        } else {
            return a < b;  // 字符串比较
        }
    } catch (const std::exception&) {
        return a < b;  // 转换失败时按字符串比较
    }
}

bool Table::evaluateHavingCondition(
    const std::vector<std::string>& groupValues,
    const std::string& havingClause) {
    
    try {
        std::string clause = havingClause;
        
        size_t openParen = clause.find("(");
        size_t closeParen = clause.find(")");
        
        if (openParen == std::string::npos || closeParen == std::string::npos) {
            throw std::runtime_error("HAVING子句格式错误");
        }
        
        // 使用 Table 类的 trim 方法
        std::string aggFunc = Table::trim(clause.substr(0, openParen));
        std::string colName = Table::trim(
            clause.substr(openParen + 1, closeParen - openParen - 1));
        
        std::string remaining = Table::trim(clause.substr(closeParen + 1));
        
        // 查找操作符
        std::vector<std::string> operators = {">=", "<=", "!=", ">", "<", "="};
        std::string op;
        size_t opPos = std::string::npos;
        
        for (const auto& testOp : operators) {
            if ((opPos = remaining.find(testOp)) != std::string::npos) {
                op = testOp;
                break;
            }
        }
        
        if (opPos == std::string::npos) {
            throw std::runtime_error("HAVING子句中未找到有效的操作符");
        }
        
        // 获取比较值
        std::string compareValue = Table::trim(
            remaining.substr(opPos + op.length()));
        
        // 计算聚合值
        double aggregateValue = 0;
        if (aggFunc == "COUNT") {
            if (colName == "*") {
                aggregateValue = groupValues.size();
            } else {
                size_t colIndex = getColumnIndex(colName);
                size_t count = 0;
                for (const auto& value : groupValues) {
                    if (!value.empty()) count++;
                }
                aggregateValue = count;
            }
        } else if (aggFunc == "SUM") {
            size_t colIndex = getColumnIndex(colName);
            for (const auto& value : groupValues) {
                if (!value.empty()) {
                    aggregateValue += std::stod(value);
                }
            }
        } else if (aggFunc == "AVG") {
            size_t colIndex = getColumnIndex(colName);
            size_t count = 0;
            for (const auto& value : groupValues) {
                if (!value.empty()) {
                    aggregateValue += std::stod(value);
                    count++;
                }
            }
            if (count > 0) {
                aggregateValue /= count;
            }
        } else {
            throw std::runtime_error("不支持的聚合函数: " + aggFunc);
        }
        
        // 比较结果
        double compareVal = std::stod(compareValue);
        if (op == "=") return std::abs(aggregateValue - compareVal) < 1e-10;
        if (op == ">") return aggregateValue > compareVal;
        if (op == "<") return aggregateValue < compareVal;
        if (op == ">=") return aggregateValue >= compareVal;
        if (op == "<=") return aggregateValue <= compareVal;
        if (op == "!=") return std::abs(aggregateValue - compareVal) >= 1e-10;
        
        return false;
    } catch (const std::exception& e) {
        throw std::runtime_error("HAVING条件评估失败: " + std::string(e.what()));
    }
}

std::vector<std::vector<std::string>> Table::selectWithAggregates(
    const std::vector<SQLParser::Column>& columns,
    const std::string& whereClause,
    const std::vector<std::string>& groupByColumns,
    const std::string& havingClause) const {
    
    try {
        // 首先应用 WHERE 条件过滤数据
        std::vector<std::vector<std::string>> filteredData;
        for (const auto& row : data) {
            if (whereClause.empty() || evaluateCondition(row, whereClause)) {
                filteredData.push_back(row);
            }
        }
        
        // 如果没有分组，直接计算聚合
        if (groupByColumns.empty()) {
            std::vector<std::vector<std::string>> result;
            std::vector<std::string> row;
            
            for (const auto& col : columns) {
                if (col.aggregateFunc != SQLParser::AggregateFunction::NONE) {
                    // 特殊处理 COUNT(*)
                    if (col.aggregateFunc == SQLParser::AggregateFunction::COUNT && col.name == "*") {
                        row.push_back(std::to_string(filteredData.size()));
                        continue;
                    }
                    
                    // 获取列的所有值
                    std::vector<std::string> values;
                    try {
                        size_t colIndex = getColumnIndex(col.name);
                        for (const auto& dataRow : filteredData) {
                            values.push_back(dataRow[colIndex]);
                        }
                    } catch (const std::exception& e) {
                        throw std::runtime_error("聚合列不存在: " + col.name);
                    }
                    row.push_back(calculateAggregate(values, col.aggregateFunc));
                } else {
                    // 非聚合列，使用第一行的值
                    try {
                        size_t colIndex = getColumnIndex(col.name);
                        row.push_back(filteredData[0][colIndex]);
                    } catch (const std::exception& e) {
                        throw std::runtime_error("列不存在: " + col.name);
                    }
                }
            }
            result.push_back(row);
            return result;
        }
        
        // 按分组列进行分组
        std::map<std::string, std::vector<std::vector<std::string>>> groups;
        for (const auto& row : filteredData) {
            std::string groupKey;
            for (const auto& groupCol : groupByColumns) {
                if (!groupKey.empty()) groupKey += "|";
                try {
                    size_t colIndex = getColumnIndex(groupCol);
                    groupKey += row[colIndex];
                } catch (const std::exception& e) {
                    throw std::runtime_error("分组列不存在: " + groupCol);
                }
            }
            groups[groupKey].push_back(row);
        }
        
        // 对每个分组计算结果
        std::vector<std::vector<std::string>> result;
        for (const auto& [key, groupRows] : groups) {
            std::vector<std::string> resultRow;
            
            // 添加分组列的值
            for (const auto& groupCol : groupByColumns) {
                try {
                    size_t colIndex = getColumnIndex(groupCol);
                    resultRow.push_back(groupRows[0][colIndex]);
                } catch (const std::exception& e) {
                    throw std::runtime_error("分组列不存在: " + groupCol);
                }
            }
            
            // 计算聚合列的值
            for (const auto& col : columns) {
                if (col.aggregateFunc != SQLParser::AggregateFunction::NONE) {
                    // 特殊处理 COUNT(*)
                    if (col.aggregateFunc == SQLParser::AggregateFunction::COUNT && col.name == "*") {
                        resultRow.push_back(std::to_string(groupRows.size()));
                        continue;
                    }
                    
                    std::vector<std::string> values;
                    try {
                        size_t colIndex = getColumnIndex(col.name);
                        for (const auto& row : groupRows) {
                            values.push_back(row[colIndex]);
                        }
                    } catch (const std::exception& e) {
                        throw std::runtime_error("聚合列不存在: " + col.name);
                    }
                    resultRow.push_back(calculateAggregate(values, col.aggregateFunc));
                } else {
                    // 非聚合列必须出现在 GROUP BY 中
                    if (std::find(groupByColumns.begin(), groupByColumns.end(), col.name) == groupByColumns.end()) {
                        throw std::runtime_error("非聚合列必须出现在 GROUP BY 子句中: " + col.name);
                    }
                }
            }
            
            // 应用 HAVING 条件
            if (havingClause.empty() || evaluateHavingClause(resultRow, havingClause)) {
                result.push_back(resultRow);
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error("聚合查询失败: " + std::string(e.what()));
    }
}

std::string Table::calculateAggregate(
    const std::vector<std::string>& values,
    SQLParser::AggregateFunction func) const {
    
    if (values.empty()) return "0";
    
    switch (func) {
        case SQLParser::AggregateFunction::COUNT:
            return std::to_string(values.size());
            
        case SQLParser::AggregateFunction::AVG: {
            double sum = 0;
            int count = 0;
            for (const auto& val : values) {
                try {
                    sum += std::stod(val);
                    count++;
                } catch (...) {}
            }
            return std::to_string(sum / count);
        }
        
        case SQLParser::AggregateFunction::SUM: {
            double sum = 0;
            for (const auto& val : values) {
                try {
                    sum += std::stod(val);
                } catch (...) {}
            }
            return std::to_string(sum);
        }
        
        // ... 实现其他聚合函数 ...
        
        default:
            return values[0];
    }
}

bool Table::evaluateHavingClause(
    const std::vector<std::string>& groupValues,
    const std::string& havingClause) const {
    
    try {
        std::string clause = havingClause;
        
        size_t openParen = clause.find("(");
        size_t closeParen = clause.find(")");
        
        if (openParen == std::string::npos || closeParen == std::string::npos) {
            throw std::runtime_error("HAVING子句格式错误");
        }
        
        // 使用 Table 类的 trim 方法
        std::string aggFunc = Table::trim(clause.substr(0, openParen));
        std::string colName = Table::trim(
            clause.substr(openParen + 1, closeParen - openParen - 1));
        
        std::string remaining = Table::trim(clause.substr(closeParen + 1));
        
        // 查找操作符
        std::vector<std::string> operators = {">=", "<=", "!=", ">", "<", "="};
        std::string op;
        size_t opPos = std::string::npos;
        
        for (const auto& testOp : operators) {
            if ((opPos = remaining.find(testOp)) != std::string::npos) {
                op = testOp;
                break;
            }
        }
        
        if (opPos == std::string::npos) {
            throw std::runtime_error("HAVING子句中未找到有效的操作符");
        }
        
        // 获取比较值
        std::string compareValue = Table::trim(
            remaining.substr(opPos + op.length()));
        
        // 计算聚合值
        double aggregateValue = 0;
        if (aggFunc == "COUNT") {
            if (colName == "*") {
                aggregateValue = groupValues.size();
            } else {
                size_t colIndex = getColumnIndex(colName);
                size_t count = 0;
                for (const auto& value : groupValues) {
                    if (!value.empty()) count++;
                }
                aggregateValue = count;
            }
        } else if (aggFunc == "SUM") {
            size_t colIndex = getColumnIndex(colName);
            for (const auto& value : groupValues) {
                if (!value.empty()) {
                    aggregateValue += std::stod(value);
                }
            }
        } else if (aggFunc == "AVG") {
            size_t colIndex = getColumnIndex(colName);
            size_t count = 0;
            for (const auto& value : groupValues) {
                if (!value.empty()) {
                    aggregateValue += std::stod(value);
                    count++;
                }
            }
            if (count > 0) {
                aggregateValue /= count;
            }
        } else {
            throw std::runtime_error("不支持的聚合函数: " + aggFunc);
        }
        
        // 比较结果
        double compareVal = std::stod(compareValue);
        if (op == "=") return std::abs(aggregateValue - compareVal) < 1e-10;
        if (op == ">") return aggregateValue > compareVal;
        if (op == "<") return aggregateValue < compareVal;
        if (op == ">=") return aggregateValue >= compareVal;
        if (op == "<=") return aggregateValue <= compareVal;
        if (op == "!=") return std::abs(aggregateValue - compareVal) >= 1e-10;
        
        return false;
    } catch (const std::exception& e) {
        throw std::runtime_error("HAVING条件评估失败: " + std::string(e.what()));
    }
} 