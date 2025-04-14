#ifndef FORWARD_DECLARATIONS_H
#define FORWARD_DECLARATIONS_H

#include <string>
#include <vector>

struct ColumnDef {
    std::string name;
    std::string type;
    bool nullable = true;
    bool primaryKey = false;
    // 外键相关
    bool isForeignKey = false;
    std::string referenceTable;
    std::string referenceColumn;
};

class Table;

#endif 